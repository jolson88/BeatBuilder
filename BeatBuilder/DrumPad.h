#pragma once

#include "Sound.h"
#include "ISoundSource.h"
#include "AudioRenderer.h"

namespace BeatBuilder
{
	namespace Audio
	{
		public enum class DrumKind
		{
			Bass,
			Snare,
			Shaker,
			ClosedHiHat,
			OpenHiHat,
			Cowbell,
			RideCymbal,
			FloorTom,
			HighTom
		};

		public ref class DrumPad sealed
		{
		public:
			DrumPad();
			virtual ~DrumPad();

			void SetDrumSound(DrumKind kind, Platform::String^ mediaPath);
			void PlayDrum(DrumKind kind);
			void SetRenderer(AudioRenderer^ renderer) { renderer->AddSoundSource(m_source); }

		private:
			std::map<DrumKind, std::shared_ptr<Sound>> m_sounds;
			std::shared_ptr<ISoundSource> m_source;

			class DrumPadSource : public ISoundSource
			{
			public:
				DrumPadSource(DrumPad^ pad);
				virtual bool get_next_samples(std::vector<float>& _buffer_to_fill, const int _frame_count, const int _sample_rate, const int _channels);

			private:
				DrumPad^ m_pad;
			};
		};
	}
}

