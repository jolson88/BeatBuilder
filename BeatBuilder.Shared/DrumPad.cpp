#include "pch.h"
#include "DrumPad.h"
#include "BufferHelpers.h"

using namespace std;
using namespace BeatBuilder::Audio;
using namespace Windows::Storage::Streams;

DrumPad::DrumPad()
{
}


DrumPad::~DrumPad()
{
}

void DrumPad::SetDrumSound(DrumKind kind, Platform::String^ mediaPath)
{
	m_sounds[kind] = shared_ptr<Sound>(new Sound(mediaPath->Data()));
}

void DrumPad::PlayDrum(DrumKind kind)
{
	if (m_sounds.count(kind) == 0)
	{
		throw ref new Platform::InvalidArgumentException("Drum sound hasn't been configured yet");
	}

	m_sounds[kind]->current_sample = 0;
	m_sounds[kind]->is_playing = true;
}

void DrumPad::FillNextSamples(IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate)
{
	byte* sample_data_ptr = GetBytePointerFromBuffer(bufferToFill);
	float* sample_float_ptr = reinterpret_cast<float*>(sample_data_ptr);

	for (auto it = m_sounds.begin(); it != m_sounds.end(); it++)
	{
		// NOTE: This code currently assumes the source-wave channel count matches shared mode endpoint channel count
		auto _sound = it->second;
		if (_sound->is_playing)
		{
			auto _remaining_samples = _sound->samples.size() - _sound->current_sample;
			auto _requested_samples = frameCount * channels;
			auto _samples_to_fill = (_requested_samples > _remaining_samples) ? _remaining_samples : _requested_samples;

			for (int i = 0; i < _samples_to_fill; i++)
			{
				float existing_value = sample_float_ptr[i];
				float new_value = _sound->samples[_sound->current_sample + i];

				sample_float_ptr[i] = (existing_value + new_value);
			}
			_sound->current_sample += _samples_to_fill;

			// Stop playing if we have played all the samples
			if (_samples_to_fill < _requested_samples)
			{
				_sound->current_sample = 0;
				_sound->is_playing = false;
			}
		}
	}
}