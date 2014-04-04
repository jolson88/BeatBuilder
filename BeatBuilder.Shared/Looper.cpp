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
		if (m_startRecordRequested && m_recordedLoops.size() == 1 && !m_isRecording)
		{
			OutputDebugString(L"Recording Started on First Loop... \n");
			m_currentSample = 0;
			m_isRecording = true;
		}
		else if (m_startRecordRequested && m_recordedLoops.size() > 1 && !m_isRecording && m_currentSample == 0)
		{
			OutputDebugString(L"Recording Started on Next Loop... \n");
			m_isRecording = true;
		}

		if (m_isRecording)
		{
			// We're always recording on just the back loop (the front ones are already recorded)
			m_recordedLoops[m_recordedLoops.size() - 1][m_currentSample] = sampleFloatPtr[i];
		}

		// Play recorded loops
		if (m_recordedLoops.size() > 1)
		{
			// This i/j stuff is brittle and confusing. Clean up iteration.
			for (auto j = 0; j != m_recordedLoops.size(); j++)
			{
				sampleFloatPtr[i] = sampleFloatPtr[i] + m_recordedLoops[j][m_currentSample];
			}
		}
		m_currentSample = (m_currentSample + 1) % (m_recordedLoops[0].size() - 1);

		// Do we need to stop recording?
		if (m_stopRecordRequested && m_recordedLoops.size() == 1)
		{
			OutputDebugString(L"First loop recorded \n");
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;

			// Resize so ongoing loops are the same size as this
			m_recordedLoops[0].resize(m_currentSample);
			m_currentSample = 0;

			// TODO: Extract out to method that is used
			m_recordedLoops.push_back(std::vector<float>(m_recordedLoops[0].size()));
		}
		else if (m_currentSample == 0 && m_isRecording)
		{
			// Our current recording just reached the end of loop length
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;

			// Get our next buffer ready
			m_recordedLoops.push_back(std::vector<float>(m_recordedLoops[0].size()));
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

	if (m_recordedLoops.size() == 0)
	{
		auto totalSamplesPerSecond = format->SamplesPerSecond * format->Channels;
		auto maximumSamplesForLoop = totalSamplesPerSecond * 5; // 5 seconds is maximum for loop
		m_recordedLoops = std::vector<std::vector<float>>();
		m_recordedLoops.push_back(std::vector<float>(maximumSamplesForLoop));
		
		// TODO: Check to see if I can remove this:
		std::fill(m_recordedLoops[0].begin(), m_recordedLoops[0].end(), 0);
	}
}