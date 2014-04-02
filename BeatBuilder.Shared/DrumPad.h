#pragma once

#include "Sound.h"
#include "ISoundSource.h"
#include "ISoundListener.h"
#include "AudioRenderer.h"
#include "WaveFormat.h"

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
			virtual void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
			virtual void SetWaveFormat(WaveFormat^ format) { m_format = format; }

		private:
			std::map<DrumKind, std::shared_ptr<Sound>> m_sounds;
			WaveFormat^ m_format;
		};
	}
}

