#include "pch.h"
#include "Looper.h"
#include "BufferHelpers.h"

using namespace BeatBuilder::Audio;

Looper::Looper() :
m_currentSample(0)
{
}


Looper::~Looper()
{
}

void Looper::StartRecording()
{
	m_startRecordRequested = true;
}

void Looper::StopRecording()
{
	m_stopRecordRequested = true;
}

void Looper::FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate)
{
	m_source->FillNextSamples(bufferToFill, frameCount, channels, sampleRate);
	byte* sampleDataPtr = GetBytePointerFromBuffer(bufferToFill);
	float* sampleFloatPtr = reinterpret_cast<float*>(sampleDataPtr);

	// Begin playing loops
	for (int i = 0; i < frameCount * channels; i++)
	{	
		// Do some recording if we need to 
		// TODO: Try refactoring code to current recording sample being separate from vector of recorded loops

		if (m_startRecordRequested && m_loops.size() == 0 && !m_isRecording)
		{
			RecordingStarted();
			m_currentSample = 0;
			m_isRecording = true;
		}
		else if (m_startRecordRequested && m_loops.size() > 0 && !m_isRecording && m_currentSample == 0)
		{
			// TODO: Extract the two above if checks into method (ReadyToStartRecording) and remove this duplicate code
			RecordingStarted();
			m_currentSample = 0;
			m_isRecording = true;
		}

		if (m_isRecording)
		{
			m_recordingLoop[m_currentSample] = sampleFloatPtr[i];
		}

		// Play recorded loops
		if (m_loops.size() > 0)
		{
			// This i/j stuff is brittle and confusing. Clean up iteration.
			for (auto j = 0; j != m_loops.size(); j++)
			{
				sampleFloatPtr[i] = sampleFloatPtr[i] + m_loops[j][m_currentSample];
			}
		}
		m_currentSample = (m_currentSample + 1) % (m_recordingLoop.size() - 1);

		// Do we need to stop recording?
		if (m_stopRecordRequested && m_loops.size() == 0)
		{
			OutputDebugString(L"First loop recorded \n");
			RecordingStopped();

			// Resize so ongoing loops are the same size as this
			m_recordingLoop.resize(m_currentSample);

			// TODO: Extract this method (FinishRecording) and remove duplicate code below
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;
			m_currentSample = 0;
			m_loops.push_back(m_recordingLoop);
			std::fill(m_recordingLoop.begin(), m_recordingLoop.end(), 0);
		}
		else if (m_currentSample == 0 && m_isRecording)
		{
			RecordingStopped();
			// Our current recording just reached the end of loop length
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;
			m_currentSample = 0;
			m_loops.push_back(m_recordingLoop);
			std::fill(m_recordingLoop.begin(), m_recordingLoop.end(), 0);
		}
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

	if (m_loops.size() == 0)
	{
		auto totalSamplesPerSecond = format->SamplesPerSecond * format->Channels;
		auto maximumSamplesForLoop = totalSamplesPerSecond * 5; // 5 seconds is maximum for loop
		m_recordingLoop = std::vector<float>(maximumSamplesForLoop);
	}
}