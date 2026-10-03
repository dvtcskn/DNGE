/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <thread>
#include <list>
#include <future>
#include <utility>
#include <mutex>
#include <memory>
#include <queue>
#include <iostream>
#include <vector>
#include <type_traits>
#include "Engine/ClassBody.h"

enum class EThreadStatus 
{
	EThread_Idle,
	EThread_Waiting,
	EThread_Running,
	EThread_Complete,
};

enum class EThreadState
{
	EThread_Attached,
	EThread_Detached,
};

enum class EThreadType 
{
	EThread_Deferred,
	EThread_immediate,
};

class WorkerThread
{
	sBaseClassBody(sClassConstructor, WorkerThread)
public:
	WorkerThread(EThreadType InType = EThreadType::EThread_immediate)
		: mThread(NULL)
		, mFunction(NULL)
		, ThreadStatus(EThreadStatus::EThread_Idle)
		, ThreadState(EThreadState::EThread_Attached)
		, ThreadType(InType)
		, bLoop(true)
	{};

	~WorkerThread()
	{
		mCV.notify_one();
		bLoop = false;
		if (mThread && !GetThreadIsDetached()) {
			if (mThread->joinable()) {
				mThread->join();
			}
			if (mThread) {
				delete mThread;
				mThread = nullptr;
			}
		}
		mThread = nullptr;
		mFunction = nullptr;
	};

	template<typename... Args>
	void BindOnStart(Args&& ... args)
	{
		std::lock_guard<std::mutex> lk(mMutex);
		mFunction = std::bind(std::forward<Args>(args)...);

		StartThread();
	}

	void BindOnStart(std::function<void()> pFunc)
	{
		std::lock_guard<std::mutex> lk(mMutex);
		mFunction = pFunc;

		StartThread();
	}

	sFORCEINLINE void StartThread()
	{
		mThread = new std::thread(&WorkerThread::RunThread, this);
	}

	sFORCEINLINE void Detach()
	{
		mThread->detach();
		ThreadState = EThreadState::EThread_Detached;
	}

	sFORCEINLINE void SetThreadType(EThreadType InType)
	{
		ThreadType = InType;
	}

	sFORCEINLINE void ForceFinishThread()
	{
		if (mThread && !GetThreadIsDetached()) {
			bLoop = false;
			if (mThread->joinable()) {
				mThread->join();
			}
		}
	}

	sFORCEINLINE bool Awake()
	{
		if (ThreadStatus == EThreadStatus::EThread_Waiting || ThreadStatus == EThreadStatus::EThread_Idle || ThreadStatus == EThreadStatus::EThread_Complete)
		{
			mCV.notify_one();
			return true;
		}
		return false;
	}

	sFORCEINLINE bool GetThreadIsFinished() const { return ThreadStatus == EThreadStatus::EThread_Complete && mThread->joinable(); }
	sFORCEINLINE bool GetThreadIsDetached() const { return ThreadState == EThreadState::EThread_Detached; }
	sFORCEINLINE EThreadStatus GetThreadStatus() const { return ThreadStatus; }
	sFORCEINLINE EThreadType GetThreadType() const { return ThreadType; }
	sFORCEINLINE bool GetThreadIsIdle() const { return ThreadStatus == EThreadStatus::EThread_Idle; }
	sFORCEINLINE bool GetThreadIsWaiting() const { return ThreadStatus == EThreadStatus::EThread_Waiting; }
	sFORCEINLINE bool GetThreadIsComplete() const { return ThreadStatus == EThreadStatus::EThread_Complete; }
	sFORCEINLINE bool GetThreadIsRunning() const { return ThreadStatus == EThreadStatus::EThread_Running; }
	sFORCEINLINE void SetThreadStatus(EThreadStatus InStatus) { ThreadStatus = InStatus; }

private:
	sFORCEINLINE void RunThread()
	{
		while (bLoop)
		{
			std::unique_lock<std::mutex> locker(mMutex);
			if (ThreadType == EThreadType::EThread_Deferred)
			{
				mCV.wait(locker);
			}

			if (!bLoop)
			{
				return;
			}

			ThreadStatus = EThreadStatus::EThread_Running;
			mFunction();
			ThreadStatus = EThreadStatus::EThread_Complete;
		}
	}

	std::thread* mThread;

	std::function<void()> mFunction;

	std::mutex mMutex;
	std::condition_variable mCV;
	std::atomic<EThreadStatus> ThreadStatus;

	std::atomic<EThreadType> ThreadType;
	std::atomic<EThreadState> ThreadState;
	std::atomic<bool> bLoop;
};

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t ThreadCount = std::thread::hardware_concurrency())
    {
        if (ThreadCount == 0)
            ThreadCount = 1;

        WorkerThreads.reserve(ThreadCount);

        try
        {
            for (std::size_t i = 0; i < ThreadCount; ++i)
                WorkerThreads.emplace_back([this] { WorkerLoop(); });
        }
        catch (...)
        {
            Stop();
            throw;
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool()
    {
        Stop();
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> Lock(Mutex);
            bStop.store(true);
        }

        Condition.notify_all();

        for (auto& Worker : WorkerThreads)
        {
            if (Worker.joinable())
                Worker.join();
        }

        WorkerThreads.clear();
    }

    void QueueJob(std::function<void()> Job)
    {
        if (!Job)
            throw std::invalid_argument("Cannot queue an empty job.");

        {
            std::lock_guard<std::mutex> Lock(Mutex);
            if (bStop.load())
                throw std::logic_error("Cannot queue a job after ThreadPool::Stop().");

            Jobs.push(std::move(Job));
        }

        Condition.notify_one();
    }

    template<class F, class... Args>
    auto Submit(F&& function, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using Result = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<Result()>>(std::bind(std::forward<F>(function), std::forward<Args>(args)...));
        auto result = task->get_future();
        QueueJob([task] { (*task)(); });
        return result;
    }

    bool HasActiveJobs() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return !Jobs.empty() || ActiveJobs != 0;
    }

    std::size_t AvailableThreadCount() const
    {
        return WorkerThreads.size();
    }

private:
    void WorkerLoop()
    {
        for (;;)
        {
            std::function<void()> Job;

            {
                std::unique_lock<std::mutex> Lock(Mutex);
                Condition.wait(Lock, [this] 
				{
                    return bStop.load() || !Jobs.empty();
				});

                if (bStop.load() && Jobs.empty())
                    return;

				Job = std::move(Jobs.front());
                Jobs.pop();
                ++ActiveJobs;
            }

            try
            {
				Job();
            }
            catch (...)
            {
                // log
            }

            {
                std::lock_guard<std::mutex> Lock(Mutex);
                --ActiveJobs;
            }
        }
    }

    mutable std::mutex Mutex;
    std::condition_variable Condition;
    std::queue<std::function<void()>> Jobs;
    std::vector<std::thread> WorkerThreads;
    std::size_t ActiveJobs = 0;
	std::atomic<bool> bStop = false;
};
