//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <valarray>
#include <mmreg.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// macros
#define CHECK_HR_RETURN(hr)        \
	{                              \
		if (FAILED(hr)) return hr; \
	}
#define CHECK_AND_THROW(hr) \
	if (FAILED(##hr))\
{\
	throw ref new Platform::COMException(##hr##);\
}
#define CHECK_HR_EXIT(hr)          \
	{                              \
		if (FAILED(hr)) goto exit; \
	}
#define SAFE_RELEASE(x) \
	{ SafeRelease(&x); }
template <class T>
void SafeRelease(T **ppT) {
	if (*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}
