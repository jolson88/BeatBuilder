#pragma once

#include "WaveFormat.h"

namespace BeatBuilder 
{
	namespace Audio 
	{
		public interface class ISoundSource
		{
			void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
			void SetWaveFormat(WaveFormat^ format);
		};
	}
}

