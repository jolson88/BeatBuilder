#include "pch.h"
#include "AudioRenderer.h"
#include "AudioInterfaceActivator.h"
#include <sstream>
#include "BufferHelpers.h"

using namespace BeatBuilder::Audio;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;
using namespace concurrency;
using namespace std;

namespace {
	#undef PI
	const double PI = atan(1.0) * 4;
}

AudioRenderer::AudioRenderer(ComPtr<IAudioClient2> audioClient, bool rawIsSupported) :
m_audioClient(audioClient),
m_renderCallback(this, &AudioRenderer::OnRenderCallback),
m_rawIsSupported(rawIsSupported)
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

void AudioRenderer::ListenTo(ISoundSource^ source)
{
	m_soundSource = source;
	m_soundSource->SetWaveFormat(m_waveFormat);
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
	m_waveFormat = ref new WaveFormat(m_mixFormat->nSamplesPerSec, m_mixFormat->wBitsPerSample, m_mixFormat->nChannels);

	CHECK_AND_THROW(m_audioClient->Start());
}

void AudioRenderer::InitializeWasapi()
{
	// Set Raw Processing Mode
	if (m_rawIsSupported)
	{
		AudioClientProperties audioClientProperties = { 0 };
		audioClientProperties.cbSize = sizeof(audioClientProperties);
		audioClientProperties.bIsOffload = false;
		audioClientProperties.eCategory = AudioCategory_Other;
		audioClientProperties.Options = AUDCLNT_STREAMOPTIONS_RAW;
		CHECK_AND_THROW(m_audioClient->SetClientProperties(&audioClientProperties));		
	}
	
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
	int bufferSize;
	
	EnterCriticalSection(&m_renderCallbackCS);
	if (this->m_isTurnedOn)
	{
		int channels = this->m_mixFormat->nChannels;

		CHECK_AND_THROW(m_audioClient->GetCurrentPadding(&padding));
		availableFrames = this->m_bufferSize - padding;
		bufferSize = availableFrames * channels * (this->m_mixFormat->wBitsPerSample / 8);
		CHECK_AND_THROW(m_audioRenderClient->GetBuffer(availableFrames, &data));

		if (this->m_soundSource != nullptr)
		{
			auto sampleBuffer = ref new Buffer(bufferSize);
			byte* sample_data_ptr = GetBytePointerFromBuffer(sampleBuffer);
			float* sample_float_ptr = reinterpret_cast<float*>(sample_data_ptr);
			ZeroMemory(sample_data_ptr, bufferSize);

			this->m_soundSource->FillNextSamples(sampleBuffer, availableFrames, channels, this->m_mixFormat->nSamplesPerSec);
			float* data_ptr = reinterpret_cast<float *>(data);
			for (int i = 0; i < availableFrames * channels; i++)
			{
				data_ptr[i] = sample_float_ptr[i];
			}
		}

		CHECK_AND_THROW(m_audioRenderClient->ReleaseBuffer(availableFrames, 0));
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
			[defaultRenderDeviceId](ComPtr<IAudioClient2> renderClient)
		{
			Platform::String^ rawProcessingSupportedKey = L"System.Devices.AudioDevice.RawProcessingSupported";
			Platform::Collections::Vector<Platform::String ^> ^properties = ref new Platform::Collections::Vector<Platform::String ^>();
			properties->Append(rawProcessingSupportedKey);
			
			return create_task(Windows::Devices::Enumeration::DeviceInformation::CreateFromIdAsync(defaultRenderDeviceId, properties)).then(
				[rawProcessingSupportedKey, renderClient](Windows::Devices::Enumeration::DeviceInformation ^device)
			{
				bool rawIsSupported = false;
				if (device->Properties->HasKey(rawProcessingSupportedKey) == true)
				{
					rawIsSupported = safe_cast<bool>(device->Properties->Lookup(rawProcessingSupportedKey));
				}
			
				auto renderer = ref new AudioRenderer(renderClient, rawIsSupported);
				renderer->Initialize();
				return renderer;
			});

		}, task_continuation_context::use_current());
	});
}