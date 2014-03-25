#include "pch.h"
#include "Sound.h"

using namespace Microsoft::WRL;
using namespace std;
using namespace Platform;
using namespace BeatBuilder::Audio;

Sound::Sound(LPCWSTR _media_path) :
is_playing(false),
current_sample(0)
{
    ComPtr<IMFSourceReader> _reader = NULL;
    ComPtr<IMFMediaType> _media_type = NULL;

    CHECK_AND_THROW(MFStartup(MF_VERSION));
    CHECK_AND_THROW(MFCreateSourceReaderFromURL(_media_path, NULL, &_reader));
    CHECK_AND_THROW(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &_media_type));

    UINT32 _cb_format = 0;
    CHECK_AND_THROW(MFCreateWaveFormatExFromMFMediaType(_media_type.Get(), &wave_format, &_cb_format));
    
    initialize_buffer(_reader.Get(), _media_type.Get());
}

Sound::~Sound()
{
	if (wave_format != nullptr)
	{
		delete wave_format;
	}
}

void Sound::initialize_buffer(IMFSourceReader* _reader, IMFMediaType* _media_type)
{	
	HRESULT _hr = S_OK;
    IMFSample* _sample = NULL;
    IMFMediaBuffer* _media_buffer = NULL;
	BYTE* _sample_audio_data = NULL;
    DWORD _sample_buffer_length = 0;
    UINT32 _bytes_written = 0;
    PROPVARIANT _var;

	CHECK_AND_THROW(_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &_var));
    
    LONGLONG _duration = _var.uhVal.QuadPart;
    double _duration_in_seconds = (_duration / (double)(10000 * 1000));
    unsigned int _buffer_length = (unsigned int)(_duration_in_seconds * wave_format->nAvgBytesPerSec);

    BYTE* _raw_buffer = new BYTE[_buffer_length];
    ZeroMemory((void*)_raw_buffer, _buffer_length);	

    // Read all the samples
	DWORD _flags;
    bool _still_reading = true;
    while (_still_reading)
    {
        _hr = _reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &_flags, NULL, &_sample);
        if (FAILED(_hr)) 
        { 
            _still_reading = false;
            break;
        }

        if (_flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            _still_reading = false;
            break; // Done reading samples
        }
        else if (_sample == NULL)
        {
            continue; // No sample
        }

        _hr = _sample->ConvertToContiguousBuffer(&_media_buffer);
        if (FAILED(_hr)) 
        { 
            _still_reading = false;
            break;
        }

        _hr = _media_buffer->Lock(&_sample_audio_data, NULL, &_sample_buffer_length);
        if (FAILED(_hr)) 
        { 
            _still_reading = false;
            break;
        }

        CopyMemory((void*)(_raw_buffer + _bytes_written), _sample_audio_data, _sample_buffer_length);
        _bytes_written += _sample_buffer_length;

        SafeRelease(&_sample);
        SafeRelease(&_media_buffer);
    }
    
	// Convert samples if necessary so we store in 32-bit (float)
	auto _bytes_per_sample = wave_format->wBitsPerSample / 8;
	auto _sample_count = _bytes_written / _bytes_per_sample;
	samples = vector<float>(_sample_count);
	if (_bytes_per_sample == 2)
	{
		// Convert from 16-bit (int16) to 32-bit (float)
		int16* _values = reinterpret_cast<int16*>(_raw_buffer);
		for (int i = 0; i < _sample_count; i++)
		{
			int16 _original_value = _values[i];
			float _sample_value = static_cast<float>(_values[i]) / 32768;
			samples[i] = _sample_value;
		}
	}
	else if (_bytes_per_sample == 4)
	{
		// No conversion to float needed
		float* _values = reinterpret_cast<float*>(_raw_buffer);
		for (int i = 0; i < _sample_count; i++)
		{
			samples[i] = _values[i];
		}
	}

	CHECK_AND_THROW(_hr);
}
