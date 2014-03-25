#pragma once

#include<wrl/client.h>
#include<wrl/implements.h>
#include<mfapi.h>

using namespace Microsoft::WRL;

namespace BeatBuilder
{
	namespace Audio
	{
		template<class T>
		class AsyncCallback : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFAsyncCallback>
		{
			DWORD m_queueId;
		public:
			typedef HRESULT(T::*InvokeFn)(IMFAsyncResult *asyncResult);

			AsyncCallback(T ^parent, InvokeFn fn) : m_parent(parent), m_invokeFn(fn), m_queueId(0)
			{
			}

			// IMFAsyncCallback methods
			STDMETHODIMP GetParameters(DWORD *flags, DWORD *queue)
			{
				// Implementation of this method is optional.
				*queue = m_queueId;
				*flags = 0;
				return S_OK;
			}

			STDMETHODIMP Invoke(IMFAsyncResult* asyncResult)
			{
				return (m_parent->*m_invokeFn)(asyncResult);
			}

			void SetQueueID(DWORD queueId)
			{
				m_queueId = queueId;
			}

			T ^m_parent;
			InvokeFn m_invokeFn;
		};
	}
}


