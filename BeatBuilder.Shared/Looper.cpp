#include "pch.h"
#include "Looper.h"
#include "BufferHelpers.h"

using namespace BeatBuilder::Audio;

Looper::Looper() { }
Looper::~Looper() { }

void Looper::StartRecording()
{
	m_startRecordRequested = true;
}

void Looper::StopRecording()
{
	m_stopRecordRequested = true;
}

void Looper::ResetLoops()
{
	m_resetRequested = true;
}

void Looper::FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate)
{
	m_source->FillNextSamples(bufferToFill, frameCount, channels, sampleRate);
	byte* sampleDataPtr = GetBytePointerFromBuffer(bufferToFill);
	float* sampleFloatPtr = reinterpret_cast<float*>(sampleDataPtr);

	for (int i = 0; i < frameCount * channels; i++)
	{	
		if (m_startRecordRequested && ReadyToStartRecording())
		{
			RecordingStarted();
			m_currentSample = 0;
			m_isRecording = true;
		}

		if (m_isRecording)
		{
			m_recordingLoop[m_currentSample] = sampleFloatPtr[i];
		}

		if (ThereAreLoopsToPlay())
		{
			for (auto it = m_loops.begin(); it != m_loops.end(); ++it)
			{
				sampleFloatPtr[i] = sampleFloatPtr[i] + (*it)[m_currentSample];
			}
		}
		m_currentSample = (m_currentSample + 1) % (m_recordingLoop.size() - 1);

		if (ReadyToStopRecording())
		{
			if (m_loops.size() == 0)
			{
				m_recordingLoop.resize(m_currentSample); // Resize so future loops are this size
			}

			FinishRecording();
		}
	}

	if (m_resetRequested)
	{
		ClearLoops();
	}
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

	ClearLoops();
}

void Looper::ClearLoops()
{
	FinishRecording();
	auto totalSamplesPerSecond = m_format->SamplesPerSecond * m_format->Channels;
	auto maximumSamplesForLoop = totalSamplesPerSecond * 5; // 5 seconds is maximum for loop
	
	m_loops = std::vector<std::vector<float>>();
	m_recordingLoop = std::vector<float>(maximumSamplesForLoop);
	m_resetRequested = false;
}

bool Looper::ThereAreLoopsToPlay()
{
	return m_loops.size() > 0;
}

bool Looper::ReadyToStartRecording()
{
	return (m_loops.size() == 0 && !m_isRecording) || (m_loops.size() > 0 && !m_isRecording && m_currentSample == 0);
}

bool Looper::ReadyToStopRecording()
{
	return (m_stopRecordRequested && m_loops.size() == 0) || (m_currentSample == 0 && m_isRecording);
}

void Looper::FinishRecording()
{
	RecordingStopped();
	m_isRecording = false;
	m_startRecordRequested = false;
	m_stopRecordRequested = false;
	m_currentSample = 0;
	
	// Start recording and get next loop ready to record
	m_loops.push_back(m_recordingLoop);
	std::fill(m_recordingLoop.begin(), m_recordingLoop.end(), 0);
}