#include "pch.h"
#include "Looper.h"

using namespace BeatBuilder::Audio;

Looper::Looper(Metronome^ metronome) :
m_metronome(metronome),
m_countdown(8)
{
	metronome->AddTickListener(ref new TickHandler(this, &Looper::OnTick));
}


Looper::~Looper()
{
}

void Looper::StartRecording()
{
	m_recordRequested = true;
}

void Looper::OnTick()
{
	if (m_recordRequested)
	{
		CountdownChanged(ref new CountdownChangedEventArgs(m_countdown));
		
		if (m_countdown > 0)
		{
			m_countdown--;
		}
		else
		{
			m_countdown = 8;
			m_recordRequested = false;

			// TODO: We are ready to start recording
		}
	}
}

void Looper::FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate)
{
	m_source->FillNextSamples(bufferToFill, frameCount, channels, sampleRate);
}

void Looper::ListenTo(ISoundSource^ source)
{
	m_source = source;
}

void Looper::SetWaveFormat(WaveFormat^ format)
{
	m_format = format;
	if (m_source != nullptr)
	{
		m_source->SetWaveFormat(format);
	}

	// TODO: Determine how big of a buffer to allocate based on format and how long a single loop is

}