#include "pch.h"
#include "Metronome.h"

using namespace BeatBuilder::Audio;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Diagnostics;
using namespace Windows::System::Threading;
using namespace Platform;

Metronome::Metronome(int tempo, TickResolution tickResolution)
	: m_tickInMs(0)
{
	double beatsPerSecond = tempo / 60.0;
	int millisecondsPerBeat = (int) (1000 / beatsPerSecond);
	m_tickInMs = millisecondsPerBeat / static_cast<int>(tickResolution);
}

Metronome::~Metronome(void)
{
}

void Metronome::AddTickListener(TickHandler^ callback)
{
	m_tickHandlers.push_back(callback);
}

void Metronome::Start()
{
	TimeSpan span;
	span.Duration = m_tickInMs * 10000;

	m_timer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([this](ThreadPoolTimer^ timer) {
		HRESULT hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
		if (SUCCEEDED(hr))
		{
			for (auto it = this->m_tickHandlers.begin(); it != this->m_tickHandlers.end(); ++it)
			{
				(*it)();
			}
		}
	}), span);
}

void Metronome::Stop()
{
	m_timer->Cancel();
}
