#pragma once
#include "pch.h"

//class ISoundSource
//{
//public:
//	virtual bool get_next_samples(std::vector<float>& _buffer_to_fill, const int _frame_count, const int _sample_rate, const int _channels) = 0;
//	virtual ~ISoundSource() {}
//};

namespace BeatBuilder {
	namespace Audio {
		public interface class ISoundSource
		{
			void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
		};
	}
}

