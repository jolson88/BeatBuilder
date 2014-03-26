//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "robuffer.h"
#include "windows.storage.streams.h"

// Helpers
namespace BeatBuilder
{
	namespace Audio
	{
		byte* GetBytePointerFromBuffer(Windows::Storage::Streams::IBuffer^ buffer);
	}
}