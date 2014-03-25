#include "pch.h"
#include "DrumPad.h"

using namespace std;
using namespace BeatBuilder::Audio;

DrumPad::DrumPad() :
m_source(new DrumPadSource(this))
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

DrumPad::DrumPadSource::DrumPadSource(DrumPad^ pad) :
m_pad(pad)
{
}

bool DrumPad::DrumPadSource::get_next_samples(vector<float>& _buffer_to_fill, const int _frame_count, const int _sample_rate, const int _channels)
{
	bool _sound_played = false;

	for (auto it = m_pad->m_sounds.begin(); it != m_pad->m_sounds.end(); it++)
	{
		// NOTE: This code currently assumes the source-wave channel count matches shared mode endpoint channel count
		auto _sound = it->second;
		if (_sound->is_playing)
		{
			_sound_played = true;
			auto _remaining_samples = _sound->samples.size() - _sound->current_sample;
			auto _requested_samples = _buffer_to_fill.size();
			auto _samples_to_fill = (_requested_samples > _remaining_samples) ? _remaining_samples : _requested_samples;

			for (int i = 0; i < _samples_to_fill; i++)
			{
				float existing_value = _buffer_to_fill[i];
				float new_value = _sound->samples[_sound->current_sample + i];

				_buffer_to_fill[i] = (existing_value + new_value);
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

	return _sound_played;
}