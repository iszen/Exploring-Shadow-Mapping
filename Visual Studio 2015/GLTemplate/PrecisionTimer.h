#pragma once

#ifdef WIN32
#include <Windows.h>
#else
#include <chrono>
#endif

class PrecisionTimer
{
public:
	enum Unit
	{
		hours,
		minutes,
		seconds,
		milliseconds,
		microseconds,
		nanoseconds
	};
	PrecisionTimer() { start(); m_unit = 1.f; }

	void start();
	float stop();
	float restart();

	void setUnit(float inSeconds) { m_unit = inSeconds; }
	void setUnit(const Unit &unit);

private:
#ifdef WIN32
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_freq;
#else
	std::chrono::time_point<std::chrono::system_clock> m_start;
#endif
	float m_unit;
};