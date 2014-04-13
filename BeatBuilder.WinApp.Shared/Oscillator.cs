using System;
using System.Collections.Generic;
using System.Text;
using Windows.Storage.Streams;

namespace BeatBuilder.Audio
{
    public class Oscillator : ISoundSource
    {
        private static int LOOKUP_SAMPLE_COUNT = 2048;

        private bool m_isOn = false;
        private double m_frequency = 440;
        private double m_phaseDelta;
        private double m_phaseAccumulator;
        private float[] m_sampleLookup;
        private WaveFormat m_waveFormat;

        public Oscillator()
        {
            m_sampleLookup = new float[LOOKUP_SAMPLE_COUNT];

            // Build just a sin-wave lookup table for now
            for(int i = 0; i < m_sampleLookup.Length; i++)
            {
                m_sampleLookup[i] = (float)Math.Sin((2 * Math.PI) * (i / (double)LOOKUP_SAMPLE_COUNT));
            }
        }

        private WaveFormat WaveFormat
        {
            set
            {
                m_waveFormat = value;
                Recalibrate();
            }
        }

        public void TurnOn()
        {
            m_isOn = true;
        }

        public void TurnOff()
        {
            m_isOn = false;
        }

        public void FillNextSamples(IBuffer bufferToFill, int frameCount, int channels, int sampleRate)
        {
            if (m_isOn)
            {
                var floatBuffer = new FloatBuffer(bufferToFill);

                // The lookup table is just Mono, so need to output appropriate channels
                for (int i = 0; i < floatBuffer.Length; i=i+channels)
                {
                    for (int j = 0; j < channels; j++)
                    {
                        floatBuffer[i+j] = m_sampleLookup[(int)m_phaseAccumulator];
                    }

                    m_phaseAccumulator = (m_phaseAccumulator + m_phaseDelta) % LOOKUP_SAMPLE_COUNT;
                }
            }
        }

        public void SetWaveFormat(WaveFormat format)
        {
            this.WaveFormat = format;    
        }

        private void Recalibrate()
        {
            // The wave format of oscillator frequency has changed, so recalibrate oscillator
            m_phaseDelta = (LOOKUP_SAMPLE_COUNT * m_frequency) / m_waveFormat.SamplesPerSecond;
        }
    }
}