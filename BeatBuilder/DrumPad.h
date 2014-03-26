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

		public ref class DrumPad sealed : ISoundSource
		{
		public:
			DrumPad();
			virtual ~DrumPad();

			void SetDrumSound(DrumKind kind, Platform::String^ mediaPath);
			void PlayDrum(DrumKind kind);
			void SetRenderer(AudioRenderer^ renderer) { renderer->SetSoundSource(this); }
			virtual void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
		
		private:
			std::map<DrumKind, std::shared_ptr<Sound>> m_sounds;
		};
	}
}

