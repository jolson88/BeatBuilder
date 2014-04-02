#pragma once
#include "pch.h"

namespace BeatBuilder
{
	namespace Audio
	{
		class Sound
		{
		public:
			Sound(LPCWSTR _media_path);
			~Sound();
	
			std::vector<float> samples;
			bool is_playing;
			int current_sample;
			WAVEFORMATEX* wave_format;

		private:
			void initialize_buffer(IMFSourceReader *_reader, IMFMediaType *_audio_type);
		};
	}
}
