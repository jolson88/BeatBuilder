#pragma once

#include "ISoundSource.h"
#include "ISoundListener.h"
#include "Metronome.h"

namespace BeatBuilder
{
	namespace Audio
	{
		public ref class CountdownChangedEventArgs sealed
		{
		public:
			CountdownChangedEventArgs(int ticksLeft) { this->TicksLeft = ticksLeft; }
			virtual ~CountdownChangedEventArgs() { }

			property int TicksLeft;
		};

		public delegate void CountdownChangedEventHandler(CountdownChangedEventArgs^ args);

		public ref class Looper sealed : ISoundSource, ISoundListener
		{
		public:
			Looper();
			virtual ~Looper();

			void StartRecording();
			void StopRecording();
			virtual void FillNextSamples(Windows::Storage::Streams::IBuffer^ bufferToFill, int frameCount, int channels, int sampleRate);
			virtual void ListenTo(ISoundSource^ source);
			virtual void SetWaveFormat(WaveFormat^ format);

			event CountdownChangedEventHandler^ CountdownChanged;

		private:
			ISoundSource^ m_source;
			WaveFormat^ m_format;
			Metronome^ m_metronome;
			bool m_startRecordRequested;
			bool m_stopRecordRequested;
			bool m_isRecording;
			int m_loopsRecorded;
			std::vector<std::vector<float>> m_recordedLoops;
			int m_loopSampleCount;
			int m_currentSample;

			void OnTick();
		};
	}
}
