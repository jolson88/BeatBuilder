#pragma once


namespace BeatBuilder
{
	namespace Audio
	{
		public ref class WaveFormat sealed
		{
		public:
			WaveFormat(uint32 samplesPerSecond, uint32 bitsPerSample, uint32 channels) :
				m_samplesPerSecond(samplesPerSecond),
				m_bitsPerSample(bitsPerSample),
				m_channels(channels)
			{
			}

			virtual ~WaveFormat() { }

			property uint32 SamplesPerSecond
			{
				uint32 get()
				{
					return m_samplesPerSecond;
				}
			}

			property uint32 BitsPerSample
			{
				uint32 get()
				{
					return m_bitsPerSample;
				}
			}

			property uint32 Channels
			{
				uint32 get()
				{
					return m_channels;
				}
			}

		private:
			uint32 m_samplesPerSecond;
			uint32 m_bitsPerSample;
			uint32 m_channels;
		};
	}
};
