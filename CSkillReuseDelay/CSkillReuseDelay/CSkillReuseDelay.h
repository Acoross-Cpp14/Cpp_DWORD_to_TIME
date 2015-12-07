#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <map>
#include <utility>
#include <mutex>
#include <ctime>
#include <Windows.h>
#include <exception>

#include "Assembler.h"
#include "TimeConverter.h"

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