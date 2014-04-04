#pragma once

#include "ISoundSource.h"
#include "ISoundListener.h"
#include "Metronome.h"

namespace BeatBuilder
{
	namespace Audio
	{
		public delegate void StatusChangedEventHandler();

		public ref class Looper sealed : ISoundSource, ISoundListener
		{
		public:
			Looper();
			virtual ~Looper();

			void StartRecording();
			void StopRecording();
			void ResetLoops();
			virtual void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
			virtual void ListenTo(ISoundSource^ source);
			virtual void SetWaveFormat(WaveFormat^ format);

			event StatusChangedEventHandler^ RecordingStarted;
			event StatusChangedEventHandler^ RecordingStopped;

		private:
			ISoundSource^ m_source;
			WaveFormat^ m_format;
			bool m_startRecordRequested;
			bool m_stopRecordRequested;
			bool m_isRecording;
			std::vector<std::vector<float>> m_loops;
			std::vector<float> m_recordingLoop;
			int m_currentSample;

			bool ThereAreLoopsToPlay();
			bool ReadyToStartRecording();
			bool ReadyToStopRecording();
			void FinishRecording();
		};
	}
}
