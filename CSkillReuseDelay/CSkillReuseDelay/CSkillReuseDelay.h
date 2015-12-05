#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <map>
#include <utility>
#include <mutex>
#include <ctime>
#include <Windows.h>
#include <exception>

typedef char* packet_t;

template<class... Args>
char* Assemble(char* packet, const char* const format, Args... args);

template<class Arg>
char* Assemble(char* packet, const char* format, Arg arg)
{
	const char format_c = format[0];
	switch (format_c)
	{
	case 'd':
		{
			int* target = (int*)packet;
			*target = arg;
			packet += sizeof(int);
		}	
		break;
	default:
		_ASSERT(0);
		break;
	}

	return packet;
}

template<class Arg, class...Args>
char* Assemble(char* packet, const char* const format, Arg arg, Args... args)
{
	packet = Assemble(packet, format, arg);
	packet = Assemble(packet, &format[1], args...);
	return packet;
}


constexpr int const_strlen(const char* const format)
{
	return *format != '\0' ? 1 + const_strlen(format + 1) : 0;
}

template<size_t N, class... Args>
char* Disassemble(char* packet, const char (&format)[N], Args*... pArgs)
{
	static_assert(N == sizeof...(pArgs) + 1, "Disassemble assert: format size differs from argument count");
	_ASSERT(N == sizeof...(pArgs) + 1);

	return Disassemble_impl(packet, format, pArgs...);
}

template<class Arg, class... Args>
char* Disassemble_impl(char* packet, const char* const format, Arg* pArg, Args*... pArgs);

template<class Arg, class... Args>
char* Disassemble_impl(char* packet, const char* const format, Arg* pArg, Args*... pArgs)
{
	packet = Disassemble_impl(packet, format, pArg);
	packet = Disassemble_impl(packet, &format[1], pArgs...);
	return packet;
}

template<class Arg>
char* Disassemble_impl(char* packet, const char* format, Arg* pArg)
{
	const char format_c = *format;
	switch (format_c)
	{
	case 'd':
	{
		int* target = (int*)packet;
		*pArg = *target;
		packet += sizeof(int);
	}
	break;
	default:
		_ASSERT(0);
		break;
	}

	return packet;
}

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

typedef int skillid_t;
struct ReuseDelay
{
	skillid_t skillId;
	DWORD nEndTick;
	DWORD nDelayAmount;
	bool bDirtyFlag;
};

class ReuseDelayConverter
{
public:
	ReuseDelayConverter() = delete;

	static packet_t WriteOneToPacket(packet_t& packet, ReuseDelay& rd)
	{
		__time32_t endTime = TimeConverter::TickToTime(rd.nEndTick);
		packet = Assemble(packet, "ddd", rd.skillId, endTime, rd.nDelayAmount);
		return packet;
	}

	static packet_t ParsePacketToReuseDelay(packet_t& packet, ReuseDelay& rd)
	{
		__time32_t endTime;
		packet = Disassemble(packet, "ddd", &rd.skillId, &endTime, &rd.nDelayAmount);
		rd.nEndTick = TimeConverter::TimeToTick(endTime);
		return packet;
	}
};

class CSkillReuseDelay
{
public:
	CSkillReuseDelay() = default;

	CSkillReuseDelay(const CSkillReuseDelay& rhs) = delete;
	CSkillReuseDelay& operator=(const CSkillReuseDelay& rhs) = delete;
	
	void Insert(ReuseDelay& rd)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		_Insert(rd);
	}

	void WriteToPacket(packet_t& packet)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		packet = Assemble(packet, "d", m_mapReuseDelay.size());
		for (auto& it : m_mapReuseDelay)
		{
			auto& reusedelay = it.second;
			packet = ReuseDelayConverter::WriteOneToPacket(packet, reusedelay);
			if (packet == nullptr)
				throw std::exception("WriteToPacket");
		}
	}

	void SetFromDBPacket(const packet_t packet)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		
		int count = 0;
		packet_t tmp = nullptr;

		tmp = Disassemble(packet, "d", &count);

		for (int i = 0; i < count; ++i)
		{
			ReuseDelay rd;
			tmp = ReuseDelayConverter::ParsePacketToReuseDelay(tmp, rd);
			if (tmp == nullptr)
				throw std::exception("SetFromDBPacket");
			else
				_Insert(rd);
		}
	}

	template <class Func>
	void TraverseDo(Func f)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto& it : m_mapReuseDelay)
		{
			f(it.second);
		}
	}

private:
	void _Insert(ReuseDelay& rd)
	{
		m_mapReuseDelay.insert(std::pair<skillid_t, ReuseDelay>(rd.skillId, rd));
	}

	std::mutex m_mutex;
	std::map<skillid_t, ReuseDelay> m_mapReuseDelay;
};