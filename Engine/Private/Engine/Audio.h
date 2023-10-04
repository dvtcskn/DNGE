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

#include <memory>
#include <array>
#include <vector>

#include "SpatialAudioClient.h"
#include <mmdeviceapi.h>
#include <wrl/client.h>

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include "Core/Math/CoreMath.h"
#include "Engine/ClassBody.h"
#include "WaveBankReader.h"
#include "WAVFileReader.h"
#include "Core/ThreadPool.h"

/*
* To do
* 3D Audio
* Effects
* Spatial
*/

class IAudio
{
	sBaseClassBody(sClassConstructor, IAudio)
public:
    virtual void BeginPlay() = 0;
    virtual void Tick(const double InDeltaTime) = 0;

    virtual void AddToPlayList(std::string Name, std::string path, bool loop = false, bool RunOnce = false) = 0;
    virtual void Play(std::string Name, std::string path, bool loop, bool PlayAsOverlap) = 0;
    virtual void Stop(bool immediate = true) = 0;
    virtual void Next() = 0;
    virtual void Resume() = 0;
    virtual void Pause() = 0;
    virtual void Remove(std::size_t index, bool IsOverlapSound = false) = 0;
    virtual void Remove(std::string Name, bool IsOverlapSound = false) = 0;
    virtual void DestroyAllVoice(bool IsOverlapSoundOnly = false) = 0;
    virtual void SetPlayListState(bool State) = 0;

    virtual bool IsLooped() const = 0;

    virtual float GetVolume() const = 0;
    virtual void SetVolume(float volume) = 0;

    virtual std::size_t GetPlayListCount(bool IsOverlapSoundOnly = false) const = 0;

    virtual std::size_t GetCurrentAudioIndex() const = 0;
    virtual std::size_t GetNextAudioIndex() const = 0;

    virtual void BindFunctionOnVoiceStart(std::function<void(std::string)> fOnVoiceStart) = 0;
    virtual void BindFunctionOnVoiceStop(std::function<void(std::string)> fOnVoiceStop) = 0;
};

class XAudio final : public IAudio
{
	sClassBody(sClassConstructor, XAudio, IAudio)
public:
    XAudio();
	virtual ~XAudio();

    virtual void BeginPlay() override final;
	virtual void Tick(const double InDeltaTime) override final;

    virtual void AddToPlayList(std::string Name, std::string path, bool loop = false, bool RunOnce = false) override final;
    virtual void Play(std::string Name, std::string path, bool loop, bool PlayAsOverlap) override final;
    virtual void Stop(bool immediate = true) override final;
    virtual void Next() override final;
    virtual void Resume() override final;
    virtual void Pause() override final;
    virtual void Remove(std::size_t index, bool IsOverlapSound = false) override final;
    virtual void Remove(std::string Name, bool IsOverlapSound = false) override final;
    virtual void DestroyAllVoice(bool IsOverlapSoundOnly = false) override final;
    virtual void SetPlayListState(bool State) override final;

    virtual bool IsLooped() const override final;

    virtual float GetVolume() const override final;
    virtual void SetVolume(float volume) override final;

    virtual std::size_t GetPlayListCount(bool IsOverlapSoundOnly = false) const override final;

    virtual std::size_t GetCurrentAudioIndex() const override final;
    virtual std::size_t GetNextAudioIndex() const override final;

    virtual void BindFunctionOnVoiceStart(std::function<void(std::string)> fOnVoiceStart) override final;
    virtual void BindFunctionOnVoiceStop(std::function<void(std::string)> fOnVoiceStop) override final;

private:
	Microsoft::WRL::ComPtr<IXAudio2>    XAudio2;
	IXAudio2MasteringVoice*				pMasteringVoice;
    //IXAudio2SubmixVoice*                pSubmixVoice;

	struct XAudioEngineCallback;
    XAudioEngineCallback*			    pEngineCallback;

    struct AudioData;
    std::shared_ptr<AudioData> CurrentVoice;
    std::vector<std::shared_ptr<AudioData>> PlayLists;
    std::vector<std::shared_ptr<AudioData>> OverlapPlayLists;

    bool bIsPlayListEnabled;

    DWORD dwChannelMask;
    UINT32 nChannels;
    UINT32 nSampleRate;

    std::function<void(std::string)> fOnVoiceStart;
    std::function<void(std::string)> fOnVoiceStop;

    //ISpatialAudioObjectRenderStream* SpatialAudio;
};
