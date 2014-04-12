#include "pch.h"
#include "BufferHelpers.h"

byte* BeatBuilder::Audio::GetBytePointerFromBuffer(Windows::Storage::Streams::IBuffer^ buffer)
{
	using namespace Microsoft::WRL;
	using namespace Windows::Storage::Streams;

	ComPtr<IBufferByteAccess> bufferAccess;
	ComPtr<ABI::Windows::Storage::Streams::IBuffer> abiBuffer;
	abiBuffer = reinterpret_cast<ABI::Windows::Storage::Streams::IBuffer*>(buffer);
	CHECK_AND_THROW(abiBuffer.As(&bufferAccess));

	byte* bytePointer = nullptr;
	CHECK_AND_THROW(bufferAccess->Buffer(&bytePointer));
	return bytePointer;
}

Windows::Storage::Streams::IBuffer^ BeatBuilder::Audio::CreateBuffer(int bufferSizeInBytes)
{
	auto buffer = ref new Windows::Storage::Streams::Buffer(bufferSizeInBytes);
	byte* sample_data_ptr = GetBytePointerFromBuffer(buffer);
	ZeroMemory(sample_data_ptr, bufferSizeInBytes);

	return buffer;
}