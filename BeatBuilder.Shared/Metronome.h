#pragma once

namespace BeatBuilder
{
	namespace Audio
	{
		public delegate void TickHandler();

		public enum class TickResolution
		{
			QuarterNote = 1,
			EightNote = 2,
			SixteenthNote = 4,
		};

		public ref class Metronome sealed
		{
		public:
			Metronome(int tempo, TickResolution tickResolution);
			virtual ~Metronome(void);

			void AddTickListener(TickHandler^ callback);
			void Start();
			void Stop();

		private:

			std::list<TickHandler^> m_tickHandlers;
			Windows::System::Threading::ThreadPoolTimer^ m_timer;
			int m_tickInMs;
		};
	}
}