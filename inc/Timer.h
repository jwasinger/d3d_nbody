#ifndef  TIMER_H
#define TIMER_H

#include<Windows.h>

namespace NBody
{
	class Timer
	{
	private:
		LARGE_INTEGER time;
		LARGE_INTEGER frequency;

		double LIToSecs( LARGE_INTEGER & L);
		
		int lastFramerate;
		double framerateElapsed;
		int frameCount;

		double elapsed;
		double lastElapsed;
	public:
		Timer(void);
		~Timer(void);

		int GetFramerate(void) const { return lastFramerate; }

		//get the elapsed milliseconds between the last two frames
		double GetElapsed(void) const { return lastElapsed * 1000.0; }

		// updates the frame timer.  returns true if the timer has elapsed.  returns false if the timer has not elapsed.
		bool Tick(void);
		void Start(void);
		//void Reset(void);
	};
}

#endif

