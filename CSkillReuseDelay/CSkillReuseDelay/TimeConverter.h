#pragma once

#include <ctime>
#include <Windows.h>

class TimeConverter
{
public:
	TimeConverter() = delete;

	// 100% 커버됨.
	static __time32_t TickToTime(DWORD tick)
	{
		__time32_t now = _time32(nullptr);
		DWORD nowTick = ::GetTickCount();
		__time32_t endTime = now + (__int64(tick) - __int64(nowTick)) / 1000;

		return endTime;
	}

	// 제한 있음.
	static DWORD TimeToTick(__time32_t tm)
	{
		__time32_t now = _time32(nullptr);
		double diff2 = difftime(tm, now);

		int limit = (DWORD(-1)) / 1000;
		_ASSERT(diff2 < limit && diff2 > -limit);

		DWORD nowTick = ::GetTickCount();
		DWORD endTick = nowTick + (DWORD)(diff2 * 1000);

		return endTick;
	}
};