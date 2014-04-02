#pragma once
#include<wrl/client.h>
#include<wrl/implements.h>
#include<audioclient.h>
#include<mmdeviceapi.h>
#include<mfapi.h>
#include<ppltasks.h>

using namespace Microsoft::WRL;
using namespace concurrency;

namespace BeatBuilder
{
	namespace Audio
	{
		class AudioInterfaceActivator : public RuntimeClass < RuntimeClassFlags< ClassicCom >, FtmBase, IActivateAudioInterfaceCompletionHandler >
		{
			task_completion_event<void> m_ActivateCompleted;
			STDMETHODIMP ActivateCompleted(IActivateAudioInterfaceAsyncOperation  *pAsyncOp);

		public:
			static task<ComPtr<IAudioClient2>> AudioInterfaceActivator::ActivateAudioClientAsync(LPCWCHAR deviceId);
		};
	}
};