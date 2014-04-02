#include "pch.h"
#include "AudioInterfaceActivator.h"

using namespace BeatBuilder::Audio;

HRESULT AudioInterfaceActivator::ActivateCompleted(IActivateAudioInterfaceAsyncOperation  *pAsyncOp)
{
	// Set the completed event and return success
	m_ActivateCompleted.set();
	return S_OK;
}

task<ComPtr<IAudioClient2>> AudioInterfaceActivator::ActivateAudioClientAsync(LPCWCHAR deviceId)
{
	ComPtr<AudioInterfaceActivator> pActivator = Make<AudioInterfaceActivator>();

	ComPtr<IActivateAudioInterfaceAsyncOperation> pAsyncOp;
	ComPtr<IActivateAudioInterfaceCompletionHandler> pHandler = pActivator;

	HRESULT hr = ActivateAudioInterfaceAsync(
		deviceId,
		__uuidof(IAudioClient2),
		nullptr,
		pHandler.Get(),
		&pAsyncOp);

	if (FAILED(hr))
		throw ref new Platform::COMException(hr);


	// Wait for the activate completed event
	return create_task(pActivator->m_ActivateCompleted).then
		(
		// Once the wait is completed then pass the async operation (pAsyncOp) to a lambda function which
		// retrieves and returns the IAudioClient2 interface pointer
		[pAsyncOp]() -> ComPtr<IAudioClient2>
	{
		HRESULT hr = S_OK, hr2 = S_OK;
		ComPtr<IUnknown> pUnk;
		// Get the audio activation result as IUnknown pointer
		hr2 = pAsyncOp->GetActivateResult(&hr, &pUnk);

		// Activation failure
		if (FAILED(hr))
			throw ref new Platform::COMException(hr);
		// Failure to get activate result
		if (FAILED(hr2))
			throw ref new Platform::COMException(hr2);

		// Query for the activated IAudioClient2 interface
		ComPtr<IAudioClient2> pAudioClient2;
		hr = pUnk.As(&pAudioClient2);

		if (FAILED(hr))
			throw ref new Platform::COMException(hr);

		// Return retrieved interface
		return pAudioClient2;
	}, task_continuation_context::use_current());
}

