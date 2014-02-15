#include "Timer.h"


namespace NBody
{
	NBody::Timer::Timer(void)
	{
		ZeroMemory(&this->time, sizeof(LARGE_INTEGER));

		QueryPerformanceFrequency( &frequency ) ;
	}

	/*void NBody::Timer::Reset(void)
	{
		QueryPerformanceCounter(&timer.start);
		timer.stop = timer.start;
	}*/

	Timer::~Timer(void)
	{
	}

	void Timer::Start(void)
	{
		QueryPerformanceCounter(&this->time) ;
	}

	bool Timer::Tick(void)
	{
		LARGE_INTEGER tmp = this->time;
		QueryPerformanceCounter(&this->time);

		LARGE_INTEGER delta;
		ZeroMemory(&delta, sizeof(LARGE_INTEGER));
		
		delta.QuadPart = this->time.QuadPart - tmp.QuadPart;

		this->elapsed += LIToSecs(delta);

		if(this->elapsed >= 16.0 / 1000.0) //framerate of 60fps
		{
			this->frameCount++;
			this->framerateElapsed += this->elapsed;
			this->lastElapsed = this->elapsed;
			this->elapsed = 0.0;

			if(this->framerateElapsed >= 1.0)
			{
				this->lastFramerate = this->frameCount;
				this->frameCount = 0;
				this->framerateElapsed = 0.0;
			}

			return true;
		}

		return false;
	}

	double Timer::LIToSecs( LARGE_INTEGER & L)
	{
		return ((double)L.QuadPart /(double)frequency.QuadPart);
	}
}