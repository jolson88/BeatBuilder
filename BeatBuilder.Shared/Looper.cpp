#include "pch.h"
#include "Looper.h"
#include "BufferHelpers.h"

using namespace BeatBuilder::Audio;

Looper::Looper() :
m_currentSample(0),
m_loopSampleCount(1)
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
		if (m_startRecordRequested && m_loopsRecorded == 0 && !m_isRecording)
		{
			OutputDebugString(L"Recording Started on First Loop... \n");
			m_currentSample = 0;
			m_isRecording = true;
		}
		else if (m_startRecordRequested && m_loopsRecorded > 0 && !m_isRecording && m_currentSample == 0)
		{
			OutputDebugString(L"Recording Started on Next Loop... \n");
			m_isRecording = true;
			m_recordedLoops.push_back(std::vector<float>(m_loopSampleCount));
		}

		if (m_isRecording)
		{
			// We're always recording on just the back loop (the front ones are already recorded)
			m_recordedLoops[m_recordedLoops.size() - 1][m_currentSample] = sampleFloatPtr[i];
		}

		// Play recorded loops
		if (m_loopsRecorded > 0)
		{
			// This i/j stuff is brittle and confusing. Clean up iteration.
			for (auto j = 0; j != m_recordedLoops.size(); j++)
			{
				sampleFloatPtr[i] = sampleFloatPtr[i] + m_recordedLoops[j][m_currentSample];
			}
		}
		m_currentSample = (m_currentSample + 1) % m_loopSampleCount;

		// Do we need to stop recording?
		if (m_stopRecordRequested && m_loopsRecorded == 0)
		{
			OutputDebugString(L"First loop recorded \n");
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;

			// Resize so ongoing loops are the same size as this
			m_loopSampleCount = m_currentSample + 1;
			m_recordedLoops[0].resize(m_loopSampleCount);
			m_loopsRecorded++;
		}
		else if (m_currentSample == m_loopSampleCount - 1 && m_isRecording)
		{
			// Our current recording just reached the end of loop length
			m_isRecording = false;
			m_startRecordRequested = false;
			m_stopRecordRequested = false;
			m_loopsRecorded++;
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

	if (m_loopsRecorded == 0)
	{
		auto totalSamplesPerSecond = format->SamplesPerSecond * format->Channels;
		auto maximumSamplesForLoop = totalSamplesPerSecond * 5; // 5 seconds is maximum for loop
		m_recordedLoops = std::vector<std::vector<float>>();
		m_recordedLoops.push_back(std::vector<float>(maximumSamplesForLoop));
		std::fill(m_recordedLoops[0].begin(), m_recordedLoops[0].end(), 0);
		
		// TODO: This is brittle, move this to method that looks up based on first sample in loop vector (and get rid of this variable)	
		m_loopSampleCount = maximumSamplesForLoop;
	}
}