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


#include "pch.h"
#include "Audio.h"
#include "Utilities/Exception.h"
#include "Utilities/FileManager.h"
#include "Engine/AbstractEngine.h"

using namespace Microsoft::WRL;

struct XAudio::AudioData
{
private:
    std::unique_ptr<uint8_t[]> waveFile;
    DirectX::WAVData waveData;

    std::string Name;

    IXAudio2SourceVoice* pSourceVoice;
    bool bIsVoiceDestroyed = false;
    bool bIsRunning = false;
    bool bIsSubmitted = false;
    bool bIsLoop = false;
    bool bRunOnce = false;
    bool bRunnedOnce = false;

public:
    AudioData(IXAudio2* XAudio2, std::string InName, std::string path, bool loop, bool RunOnce)
        : bIsLoop(loop)
        , bIsVoiceDestroyed(false)
        , bIsSubmitted(false)
        , bIsRunning(false)
        , bRunnedOnce(false)
        , Name(InName)
        , bRunOnce(RunOnce)
    {
        auto Path = FileManager::StringToWstring(path);
        auto Pathcst = Path.c_str();

        WCHAR strFilePath[MAX_PATH];
        HRESULT hr = FindMediaFileCch(strFilePath, MAX_PATH, Path.c_str());

        hr = DirectX::LoadWAVAudioFromFileEx(strFilePath, waveFile, waveData);

        XAudio2->CreateSourceVoice(&pSourceVoice, waveData.wfx);
    }

    ~AudioData()
    {
        if (!bIsVoiceDestroyed)
            DestroyVoice();

        pSourceVoice = nullptr;
        waveFile = nullptr;
    }

    void SubmitVoice()
    {
        if (GetBuffersQueued() == 0 && bRunOnce && bRunnedOnce)
        {
            return;
        }

        XAUDIO2_BUFFER buffer = { 0 };
        buffer.pAudioData = waveData.startAudio;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = waveData.audioBytes;

        if (bIsLoop)
        {
            buffer.LoopBegin = waveData.loopStart;
            buffer.LoopLength = waveData.loopLength;
            buffer.LoopCount = bIsLoop ? XAUDIO2_LOOP_INFINITE : 0;
        }

        HRESULT hr = pSourceVoice->SubmitSourceBuffer(&buffer);
        if (FAILED(hr))
        {
            // Error submitting source buffer
            DestroyVoice();
            return;
        }

        bIsRunning = false;
        bIsSubmitted = true;
        bIsVoiceDestroyed = false;
    }

    std::string GetName() const { return Name; }

    bool RunOnce() const { return bRunOnce; }
    bool IsRunnedOnce() const { return bRunnedOnce; }
    bool IsLooped() const { return bIsLoop; }
    bool IsVoiceDestroyed() const { return bIsVoiceDestroyed; }
    bool IsRunning() const { return bIsRunning; }
    bool IsSubmitted() const { return bIsSubmitted; }

    void Start()
    {
        pSourceVoice->Start();
        bIsRunning = true;
    }

    void Stop(UINT32 Flags)
    {
        if (bIsLoop)
            pSourceVoice->ExitLoop();

        pSourceVoice->Stop(Flags);
        if (Flags != 0)
            pSourceVoice->FlushSourceBuffers();

        bIsRunning = false;

        if (GetBuffersQueued() == 0 && bRunOnce)
        {
            bRunnedOnce = true;
        }
    }

    UINT32 GetBuffersQueued() const 
    {
        XAUDIO2_VOICE_STATE state;
        pSourceVoice->GetState(&state); 
        return state.BuffersQueued;
    }

    UINT64 GetSamplesPlayed() const 
    {
        XAUDIO2_VOICE_STATE state;
        pSourceVoice->GetState(&state); 
        return state.SamplesPlayed;
    }

    void DestroyVoice()
    {
        if (!bIsVoiceDestroyed)
        {
            pSourceVoice->DestroyVoice();
            bIsVoiceDestroyed = true;
            bIsSubmitted = false;
            bIsRunning = false;
        }
    }

    HRESULT FindMediaFileCch(WCHAR* strDestPath, int cchDest, LPCWSTR strFilename)
    {
        bool bFound = false;

        if (!strFilename || strFilename[0] == 0 || !strDestPath || cchDest < 10)
            return E_INVALIDARG;

        // Get the exe name, and exe path
        WCHAR strExePath[MAX_PATH] = { 0 };
        WCHAR strExeName[MAX_PATH] = { 0 };
        WCHAR* strLastSlash = nullptr;
        GetModuleFileName(nullptr, strExePath, MAX_PATH);
        strExePath[MAX_PATH - 1] = 0;
        strLastSlash = wcsrchr(strExePath, TEXT('\\'));
        if (strLastSlash)
        {
            wcscpy_s(strExeName, MAX_PATH, &strLastSlash[1]);

            // Chop the exe name from the exe path
            *strLastSlash = 0;

            // Chop the .exe from the exe name
            strLastSlash = wcsrchr(strExeName, TEXT('.'));
            if (strLastSlash)
                *strLastSlash = 0;
        }

        wcscpy_s(strDestPath, cchDest, strFilename);
        if (GetFileAttributes(strDestPath) != 0xFFFFFFFF)
            return S_OK;

        // Search all parent directories starting at .\ and using strFilename as the leaf name
        WCHAR strLeafName[MAX_PATH] = { 0 };
        wcscpy_s(strLeafName, MAX_PATH, strFilename);

        WCHAR strFullPath[MAX_PATH] = { 0 };
        WCHAR strFullFileName[MAX_PATH] = { 0 };
        WCHAR strSearch[MAX_PATH] = { 0 };
        WCHAR* strFilePart = nullptr;

        GetFullPathName(L".", MAX_PATH, strFullPath, &strFilePart);
        if (!strFilePart)
            return E_FAIL;

        while (strFilePart && *strFilePart != '\0')
        {
            swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName);
            if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
            {
                wcscpy_s(strDestPath, cchDest, strFullFileName);
                bFound = true;
                break;
            }

            swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s\\%s", strFullPath, strExeName, strLeafName);
            if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
            {
                wcscpy_s(strDestPath, cchDest, strFullFileName);
                bFound = true;
                break;
            }

            swprintf_s(strSearch, MAX_PATH, L"%s\\..", strFullPath);
            GetFullPathName(strSearch, MAX_PATH, strFullPath, &strFilePart);
        }
        if (bFound)
            return S_OK;

        // On failure, return the file as the path but also return an error code
        wcscpy_s(strDestPath, cchDest, strFilename);

        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
};

struct XAudio::XAudioEngineCallback : public IXAudio2EngineCallback
{
    XAudioEngineCallback() noexcept(false)
    {
        mCriticalError = (CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
        if (!mCriticalError)
        {
            throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "CreateEventEx");
        }
    }

    XAudioEngineCallback(XAudioEngineCallback&&) = default;
    XAudioEngineCallback& operator= (XAudioEngineCallback&&) = default;

    XAudioEngineCallback(XAudioEngineCallback const&) = delete;
    XAudioEngineCallback& operator= (XAudioEngineCallback const&) = delete;

    virtual ~XAudioEngineCallback() = default;

    STDMETHOD_(void, OnProcessingPassStart) () override {}
    STDMETHOD_(void, OnProcessingPassEnd)() override {}

    STDMETHOD_(void, OnCriticalError) (THIS_ HRESULT error)
    {
#ifndef _DEBUG
        UNREFERENCED_PARAMETER(error);
#endif
        //DebugTrace("ERROR: AudioEngine encountered critical error (%08X)\n", static_cast<unsigned int>(error));
        SetEvent(mCriticalError);
    }

    HANDLE mCriticalError;
};

XAudio::XAudio()
    : Super()
    , CurrentVoice(nullptr)
    , bIsPlayListEnabled(true)
    , fOnVoiceStart(nullptr)
    , fOnVoiceStop(nullptr)
{
    UINT32 flags = 0;
    HRESULT hr = XAudio2Create(XAudio2.ReleaseAndGetAddressOf(), flags);

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    // To see the trace output, you need to view ETW logs for this application:
    //    Go to Control Panel, Administrative Tools, Event Viewer.
    //    View->Show Analytic and Debug Logs.
    //    Applications and Services Logs / Microsoft / Windows / XAudio2. 
    //    Right click on Microsoft Windows XAudio2 debug logging, Properties, then Enable Logging, and hit OK 
    XAUDIO2_DEBUG_CONFIGURATION debug = { 0 };
    debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    debug.BreakMask = XAUDIO2_LOG_ERRORS;
    XAudio2->SetDebugConfiguration(&debug, nullptr);
#endif

    pEngineCallback = new XAudioEngineCallback;
    XAudio2->RegisterForCallbacks(pEngineCallback);

    XAudio2->CreateMasteringVoice(&pMasteringVoice);

    XAUDIO2_VOICE_DETAILS details;
    pMasteringVoice->GetVoiceDetails(&details);
    pMasteringVoice->GetChannelMask(&dwChannelMask);

    nSampleRate = details.InputSampleRate;
    nChannels = details.InputChannels;

    XAudio2->StartEngine();
}

XAudio::~XAudio()
{
    fOnVoiceStart = nullptr;
    fOnVoiceStop = nullptr;

    Stop();

    if (CurrentVoice)
        CurrentVoice->DestroyVoice();
    CurrentVoice = nullptr;

    for (auto& PlayList : PlayLists)
    {
        PlayList->DestroyVoice();
        PlayList = nullptr;
    }
    PlayLists.clear();

    for (auto& PlayList : OverlapPlayLists)
    {
        PlayList->DestroyVoice();
        PlayList = nullptr;
    }
    OverlapPlayLists.clear();

    XAudio2->StopEngine();

    XAudio2->UnregisterForCallbacks(pEngineCallback);
    delete pEngineCallback;
    pEngineCallback = nullptr;

    pMasteringVoice->DestroyVoice();
    pMasteringVoice = nullptr;

    XAudio2 = nullptr;
}

void XAudio::BeginPlay()
{
}

void XAudio::Tick(const double InDeltaTime)
{
    if (CurrentVoice)
    {
        if (CurrentVoice->GetBuffersQueued() == 0)
        {
            Next();
        }
    }
    else
    {
        Next();
    }
    
    {
        std::vector<std::shared_ptr<AudioData>>::iterator it = OverlapPlayLists.begin();
        while (it != OverlapPlayLists.end())
        {
            if ((*it))
            {
                if ((*it)->GetBuffersQueued() == 0)
                {
                    (*it)->Stop(0);
                    if (fOnVoiceStop)
                        fOnVoiceStop((*it)->GetName());
                    (*it)->DestroyVoice();
                    it = OverlapPlayLists.erase(it);
                    break;
                }
                else
                {
                    it++;
                }
            }
            else
            {
                it++;
            }
        }
    }
}

void XAudio::AddToPlayList(std::string Name, std::string path, bool loop, bool RunOnce)
{
    PlayLists.push_back(std::make_shared<AudioData>(XAudio2.Get(), Name, path, loop, RunOnce));
    if (!CurrentVoice && bIsPlayListEnabled)
        Next();
}

void XAudio::Play(std::string Name, std::string path, bool loop, bool PlayAsOverlap)
{
    if (PlayAsOverlap)
    {
        std::shared_ptr<AudioData> OverlapSound = std::make_shared<AudioData>(XAudio2.Get(), Name, path, loop, false);
        OverlapSound->SubmitVoice();
        OverlapSound->Start();
        if (fOnVoiceStart)
            fOnVoiceStart(OverlapSound->GetName());
        OverlapPlayLists.push_back(OverlapSound);
    }
    else
    {
        Stop();
        CurrentVoice = std::make_shared<AudioData>(XAudio2.Get(), Name, path, loop, false);
        CurrentVoice->SubmitVoice();
        CurrentVoice->Start();
        if (fOnVoiceStart)
            fOnVoiceStart(CurrentVoice->GetName());
    }
}

void XAudio::Next()
{
    if (!bIsPlayListEnabled)
        return;

    Stop();

    if (PlayLists.size() > 0)
    {
        if (!CurrentVoice)
            CurrentVoice = PlayLists.at(GetCurrentAudioIndex());
        else
            CurrentVoice = PlayLists.at(GetNextAudioIndex());

        if (CurrentVoice)
        {
            if (CurrentVoice->RunOnce() && CurrentVoice->IsRunnedOnce())
            {
            }
            else
            {
                CurrentVoice->SubmitVoice();
                CurrentVoice->Start();
                if (fOnVoiceStart)
                    fOnVoiceStart(CurrentVoice->GetName());
            }
        }
    }
}

void XAudio::Resume()
{
    if (CurrentVoice)
    {
        CurrentVoice->Start();
        if (fOnVoiceStart)
            fOnVoiceStart(CurrentVoice->GetName());
    }
}

void XAudio::Pause()
{
    if (CurrentVoice)
    {
        CurrentVoice->Stop(0);
        if (fOnVoiceStop)
            fOnVoiceStop(CurrentVoice->GetName());
    }
}

void XAudio::Remove(std::size_t index, bool IsOverlapSound)
{
    if (IsOverlapSound)
    {
        if (index >= OverlapPlayLists.size())
            return;

        OverlapPlayLists.at(index)->Stop(0);
        if (fOnVoiceStop)
            fOnVoiceStop(OverlapPlayLists.at(index)->GetName());
        OverlapPlayLists.at(index)->DestroyVoice();
        OverlapPlayLists.erase(OverlapPlayLists.begin() + index);
    }
    else
    {
        if (index >= PlayLists.size())
            return;

        if (GetCurrentAudioIndex() == index)
        {
            if (PlayLists.size() > 1)
                Next();
            else
                Stop();

            PlayLists.at(index)->DestroyVoice();

            PlayLists.erase(PlayLists.begin() + index);
        }
        else
        {
            PlayLists.at(index)->DestroyVoice();
            PlayLists.erase(PlayLists.begin() + index);
        }
    }
}

void XAudio::Remove(std::string Name, bool IsOverlapSound)
{
    if (IsOverlapSound)
    {
        std::shared_ptr<AudioData> PlayList = nullptr;
        for (std::size_t i = 0; i < OverlapPlayLists.size(); i++)
        {
            if (OverlapPlayLists[i]->GetName() == Name)
                PlayList = OverlapPlayLists[i];
        }

        if (!PlayList)
            return;

        PlayList->Stop(0);
        if (fOnVoiceStop)
            fOnVoiceStop(PlayList->GetName());
        PlayList->DestroyVoice();
        std::erase(OverlapPlayLists, PlayList);
        PlayList = nullptr;
    }
    else
    {
        std::shared_ptr<AudioData> PlayList = nullptr;
        for (std::size_t i = 0; i < PlayLists.size(); i++)
        {
            if (PlayLists[i]->GetName() == Name)
                PlayList = PlayLists[i];
        }

        if (!PlayList)
            return;

        if (PlayLists.at(GetCurrentAudioIndex())->GetName() == Name)
        {
            if (PlayLists.size() > 1)
                Next();
            else
                Stop();

            PlayList->DestroyVoice();
            std::erase(PlayLists, PlayList);
        }
        else
        {
            PlayList->DestroyVoice();
            std::erase(PlayLists, PlayList);
        }
        PlayList = nullptr;
    }
}

void XAudio::DestroyAllVoice(bool IsOverlapSoundOnly)
{
    for (auto& PlayList : OverlapPlayLists)
    {
        PlayList->Stop(0);
        PlayList->DestroyVoice();
        PlayList = nullptr;
    }
    OverlapPlayLists.clear();

    if (IsOverlapSoundOnly)
        return;

    for (auto& PlayList : PlayLists)
    {
        PlayList->Stop(0);
        PlayList->DestroyVoice();
        PlayList = nullptr;
    }
    PlayLists.clear();
}

void XAudio::SetPlayListState(bool State)
{
    bIsPlayListEnabled = State;
}

bool XAudio::IsLooped() const
{
    if (CurrentVoice)
        return CurrentVoice->IsLooped();
    return false;
}

void XAudio::Stop(bool immediate)
{
    if (!CurrentVoice)
    {
        return;
    }

    if (immediate)
    {
        CurrentVoice->Stop(0);
    }
    else
    {
        CurrentVoice->Stop(XAUDIO2_PLAY_TAILS);
    }

    if (fOnVoiceStop)
        fOnVoiceStop(CurrentVoice->GetName());

    if (std::find(PlayLists.begin(), PlayLists.end(), CurrentVoice) == PlayLists.end())
    {
        CurrentVoice->DestroyVoice();
        CurrentVoice = nullptr;
    }
}

float XAudio::GetVolume() const
{
    float Vol = 0.0f;
    pMasteringVoice->GetVolume(&Vol);
    return Vol;
}

void XAudio::SetVolume(float volume)
{
    pMasteringVoice->SetVolume(volume);
}

std::size_t XAudio::GetPlayListCount(bool IsOverlapSoundOnly) const
{
    return IsOverlapSoundOnly ? OverlapPlayLists.size() : PlayLists.size();
}

std::size_t XAudio::GetCurrentAudioIndex() const
{
    if (!CurrentVoice)
        return 0;

    auto it = std::find(PlayLists.begin(), PlayLists.end(), CurrentVoice);

    if (it != PlayLists.end())
    {
        return it - PlayLists.begin();;
    }
    return 0;
}

std::size_t XAudio::GetNextAudioIndex() const
{
    std::size_t Current = GetCurrentAudioIndex();
    if (Current + 1 >= PlayLists.size())
        return 0;
    return Current + 1;
}

void XAudio::BindFunctionOnVoiceStart(std::function<void(std::string)> InOnVoiceStart)
{
    fOnVoiceStart = InOnVoiceStart;
}

void XAudio::BindFunctionOnVoiceStop(std::function<void(std::string)> InOnVoiceStop)
{
    fOnVoiceStop = InOnVoiceStop;
}
