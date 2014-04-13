#include "pch.h"
#include "AudioDemultiplexer.h"
#include "BufferHelpers.h"

using namespace BeatBuilder::Audio;

AudioDemultiplexer::AudioDemultiplexer()
{
}

AudioDemultiplexer::~AudioDemultiplexer()
{

}

void AudioDemultiplexer::FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate)
{
	auto bufferSizeInBytes = frameCount * channels * (m_format->BitsPerSample / 8);

	float* sampleFloatPtr = reinterpret_cast<float*>(GetBytePointerFromBuffer(bufferToFill));

	auto tempBuffer = CreateBuffer(bufferSizeInBytes);
	float* tempFloatPtr = reinterpret_cast<float*>(GetBytePointerFromBuffer(tempBuffer));
	for (auto it = m_soundSources.begin(); it != m_soundSources.end(); ++it)
	{
		ResetBuffer(tempBuffer, bufferSizeInBytes);

		(*it)->FillNextSamples(tempBuffer, frameCount, channels, sampleRate);
		for (int i = 0; i < frameCount * channels; i++)
		{
			sampleFloatPtr[i] += tempFloatPtr[i];
		}
	}
}

void AudioDemultiplexer::ListenTo(ISoundSource^ source)
{
	m_soundSources.push_back(source);
}

void AudioDemultiplexer::SetWaveFormat(WaveFormat^ format)
{
	m_format = format;
	for (auto it = m_soundSources.begin(); it != m_soundSources.end(); ++it)
	{
		(*it)->SetWaveFormat(format);
	}
}
