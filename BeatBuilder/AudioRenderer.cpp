#include "pch.h"
#include "AudioRenderer.h"
#include "AudioInterfaceActivator.h"
#include <sstream>

using namespace BeatBuilder::Audio;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Media::Devices;
using namespace Windows::System::Threading;
using namespace concurrency;
using namespace std;

namespace {
	#undef PI
	const double PI = atan(1.0) * 4;
}

AudioRenderer::AudioRenderer(ComPtr<IAudioClient2> audioClient) :
m_audioClient(audioClient),
m_renderCallback(this, &AudioRenderer::OnRenderCallback)
{
}

AudioRenderer::~AudioRenderer()
{
	CHECK_AND_THROW(MFCancelWorkItem(m_renderCallbackKey));
	CHECK_AND_THROW(m_audioClient->Stop());

	CloseHandle(m_renderCallbackEvent);
	DeleteCriticalSection(&m_renderCallbackCS);
	
	// Unlock work queue
	MFUnlockWorkQueue(m_proAudioWorkQueueId);
}

void AudioRenderer::AddSoundSource(std::shared_ptr<ISoundSource> source)
{
	m_soundSource = source;
}

void AudioRenderer::Start()
{
	this->m_isTurnedOn = true;
}

void AudioRenderer::Stop()
{
	this->m_isTurnedOn = false;
}

void AudioRenderer::Initialize()
{
	if (!InitializeCriticalSectionEx(&m_renderCallbackCS, 0, 0))
	{
		throw ref new Platform::FailureException(L"Could not initialize Critical Section");
	}
	m_renderCallbackEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

	InitializeWasapi();
	InitializeMediaFoundation();

	CHECK_AND_THROW(m_audioClient->Start());
}

void AudioRenderer::InitializeWasapi()
{
	// Set Raw Processing Mode
	AudioClientProperties audioClientProperties = { 0 };
	audioClientProperties.cbSize = sizeof(audioClientProperties);
	audioClientProperties.bIsOffload = false;
	audioClientProperties.eCategory = AudioCategory_Other;
	audioClientProperties.Options = AUDCLNT_STREAMOPTIONS_RAW;
	CHECK_AND_THROW(m_audioClient->SetClientProperties(&audioClientProperties));

	// Now configure our event-based rendering model w/ WASAPI
	CHECK_AND_THROW(m_audioClient->GetMixFormat(&m_mixFormat));
	CHECK_AND_THROW(m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		0, 0, m_mixFormat, nullptr));
	CHECK_AND_THROW(m_audioClient->GetBufferSize(&m_bufferSize));
	CHECK_AND_THROW(m_audioClient->GetService(__uuidof(IAudioRenderClient), (void**) &m_audioRenderClient));
	CHECK_AND_THROW(m_audioClient->SetEventHandle(m_renderCallbackEvent));
}

void AudioRenderer::InitializeMediaFoundation()
{
	CHECK_AND_THROW(MFStartup(MF_VERSION));

	// Setup a Pro Audio work queue and async result
	DWORD taskId = 0;
	CHECK_AND_THROW(MFLockSharedWorkQueue(L"Pro Audio", 0, &taskId, &m_proAudioWorkQueueId));
	m_renderCallback.SetQueueID(m_proAudioWorkQueueId);
	CHECK_AND_THROW(MFCreateAsyncResult(nullptr, &m_renderCallback, nullptr, &m_renderCallbackResult));

	// Start our first work item to process audio
	CHECK_AND_THROW(MFPutWaitingWorkItem(m_renderCallbackEvent, 0, m_renderCallbackResult.Get(), &m_renderCallbackKey));
}

HRESULT AudioRenderer::OnRenderCallback(IMFAsyncResult *result)
{
	bool isPlaying = true;
	BYTE* data = nullptr;
	UINT32 padding;
	UINT availableFrames = 0;
	int bufferLength;
	vector<float> newSampleBuffer(0);
	bool soundSourcePlayed = false;

	EnterCriticalSection(&m_renderCallbackCS);
	if (this->m_isTurnedOn)
	{
		int channels = this->m_mixFormat->nChannels;

		CHECK_AND_THROW(m_audioClient->GetCurrentPadding(&padding));
		availableFrames = this->m_bufferSize - padding;
		bufferLength = availableFrames * channels;
		CHECK_AND_THROW(m_audioRenderClient->GetBuffer(availableFrames, &data));

		// Generate Sound
		newSampleBuffer.resize(bufferLength);
		std::fill(newSampleBuffer.begin(), newSampleBuffer.end(), 0);

		if (this->m_soundSource != nullptr)
		{
			soundSourcePlayed = this->m_soundSource->get_next_samples(newSampleBuffer,
				availableFrames,
				this->m_mixFormat->nSamplesPerSec,
				channels);

			float *data_ptr = reinterpret_cast<float *>(data);
			for (int i = 0; i < availableFrames * channels; i++)
			{
				data_ptr[i] = newSampleBuffer[i];
			}
		}

		// Release WASAPI buffer
		if (soundSourcePlayed)
		{
			CHECK_AND_THROW(m_audioRenderClient->ReleaseBuffer(availableFrames, 0));
		}
		else
		{
			// No sounds generated, so render silence
			CHECK_AND_THROW(m_audioRenderClient->ReleaseBuffer(availableFrames, AUDCLNT_BUFFERFLAGS_SILENT))
		}
	}

	// Prepare to render again
	CHECK_AND_THROW(MFPutWaitingWorkItem(m_renderCallbackEvent, 0, m_renderCallbackResult.Get(), &m_renderCallbackKey));

	LeaveCriticalSection(&m_renderCallbackCS);
	return S_OK;
}

IAsyncOperation<AudioRenderer^>^ AudioRenderer::CreateAsync()
{
	return create_async([]() -> task<AudioRenderer^>
	{
		auto defaultRenderDeviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);

		return AudioInterfaceActivator::ActivateAudioClientAsync(defaultRenderDeviceId->Data()).then(
			[](ComPtr<IAudioClient2> renderClient)
		{
			auto renderer = ref new AudioRenderer(renderClient);
			renderer->Initialize();
			return renderer;
		}, task_continuation_context::use_current());
	});
}
