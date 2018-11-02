#pragma once
namespace ParaEngine
{
	class ParaTimer
	{
	public:
		ParaTimer();
		~ParaTimer();
	public:
		void Reset();	// to reset the timer
		void Start();	// to start the timer
		void Stop();	// to stop (or pause) the timer
		void Advance();	// to advance the timer by 0.1 seconds
		static double GetAbsoluteTime(); // to get the absolute system time
		double GetAppTime() const;	// to get the current time
		double GetElapsedTime() const;	// to get the time that elapsed between GetElapsedTime() calls
	private:
		bool m_bTimerInitialized;
		bool m_bTimerStopped;
		double m_fBaseTime;
		double m_fStopTime;
		mutable double m_fLastElapsedTime;
	};
}