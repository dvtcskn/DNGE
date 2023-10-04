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

#include <functional>
#include <thread>
#include <list>
#include <future>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <vector>
#include <type_traits>
#include "Engine/ClassBody.h"

enum class EThreadStatus {
	EThread_Idle,
	EThread_Waiting,
	EThread_Running,
	EThread_Complete,
};

enum class EThreadState {
	EThread_Attached,
	EThread_Detached,
};

enum class EThreadType {
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

	sFORCEINLINE bool GetThreadIsFinished() { return mThread->joinable(); }
	sFORCEINLINE bool GetThreadIsDetached() { return ThreadState == EThreadState::EThread_Detached; }
	sFORCEINLINE EThreadStatus GetThreadStatus() { return ThreadStatus; }
	sFORCEINLINE EThreadType GetThreadType() { return ThreadType; }
	sFORCEINLINE bool GetThreadIsIdle() { return ThreadStatus == EThreadStatus::EThread_Idle; }
	sFORCEINLINE bool GetThreadIsWaiting() { return ThreadStatus == EThreadStatus::EThread_Waiting; }
	sFORCEINLINE bool GetThreadIsComplete() { return ThreadStatus == EThreadStatus::EThread_Complete; }
	sFORCEINLINE bool GetThreadIsRunning() { return ThreadStatus == EThreadStatus::EThread_Running; }
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

#if 0
class ThreadPool
{
public:
	ThreadPool(size_t numThreads) : stop(false)
	{
		for (size_t i = 0; i < numThreads; ++i) 
		{
			workers.emplace_back([this] {
				while (true) 
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(queueMutex);
						condition.wait(lock, [this] { return stop || !tasks.empty(); });
						if (stop && tasks.empty())
							return;
						task = std::move(tasks.front());
						tasks.pop();
					}
					task();
				}
			});
		}
	}

	template <class F, class... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
	{
		using return_type = std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<return_type> result = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			if (stop)
			{
				throw std::runtime_error("enqueue on stopped ThreadPool");
			}

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return result;
	}

	~ThreadPool() 
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();

		// Wait for all worker threads to finish before destroying the pool
		for (std::thread& worker : workers)
		{
			worker.join();
		}
	}

	// Additional function to wait for ongoing tasks to finish gracefully
	void wait()
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();

		for (std::thread& worker : workers) {
			worker.join();
		}
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop;
};

#endif

class ThreadPool
{
	sBaseClassBody(sClassConstructor, ThreadPool)
public:
	ThreadPool()
	{}

	~ThreadPool()
	{
		Stop();
	}

	void Start();
	void QueueJob(const std::function<void()>& job);
	void Stop();
	bool busy();

	/*
	* to do
	* return stand-by only Thread Count 
	*/
	inline std::size_t AvailableThreadCount() { return threads.size(); }

private:
	void ThreadLoop();

	std::atomic<bool> should_terminate = false;           // Tells threads to stop looking for jobs
	std::mutex queue_mutex;                  // Prevents data races to the job queue
	std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> jobs;
};
