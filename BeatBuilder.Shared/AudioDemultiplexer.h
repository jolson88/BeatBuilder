#pragma once

#include "ISoundSource.h"
#include "ISoundListener.h"
#include "WaveFormat.h"

namespace BeatBuilder
{
	namespace Audio
	{
		public ref class AudioDemultiplexer sealed : ISoundSource, ISoundListener
		{
		public:
			AudioDemultiplexer();
			virtual ~AudioDemultiplexer();

			virtual void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
			virtual void ListenTo(ISoundSource^ source);
			virtual void SetWaveFormat(WaveFormat^ format);

		private:
			std::list<ISoundSource^> m_soundSources;
			WaveFormat^ m_format;
		};
	}
}

