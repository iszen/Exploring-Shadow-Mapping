#include "PrecisionTimer.h"

void PrecisionTimer::start()
{
#ifdef WIN32
	QueryPerformanceFrequency(&m_freq);
	QueryPerformanceCounter(&m_start);
#else
	m_start = std::chrono::system_clock::now();
#endif
}

float PrecisionTimer::stop()
{
#ifdef WIN32
	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);

	return static_cast<float>(curr.QuadPart - m_start.QuadPart) / m_freq.QuadPart * m_unit;
#else
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - m_start).count() * 1.0e-9f * m_unit;
#endif
}

float PrecisionTimer::restart()
{
#ifdef WIN32
	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);

	float retVal = static_cast<float>(curr.QuadPart - m_start.QuadPart) / m_freq.QuadPart * m_unit;
	m_start = curr;
	QueryPerformanceFrequency(&m_freq);
#else
	float retVal = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - m_start).count() * 1.0e-9f * m_unit;
	m_start = std::chrono::system_clock::now();
#endif
	return retVal;
}

void PrecisionTimer::setUnit(const Unit &unit)
{
	switch (unit)
	{
	case Unit::hours:
		m_unit = 1.f / 60.f / 60.f;
		break;
	case Unit::minutes:
		m_unit = 1.f / 60.f;
		break;
	case Unit::seconds:
		m_unit = 1.f;
		break;
	case Unit::milliseconds:
		m_unit = 1000.f;
		break;
	case Unit::microseconds:
		m_unit = 1000000.f;
		break;
	case Unit::nanoseconds:
		m_unit = 1000000000.f;
		break;
	}
}