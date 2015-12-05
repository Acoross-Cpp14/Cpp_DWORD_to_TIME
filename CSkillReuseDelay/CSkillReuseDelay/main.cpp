#include "CSkillReuseDelay.h"

#include <iostream>

int main(int argc, char* argv[])
{
	/*__time32_t tm = TimeConverter::TickToTime(0);
	DWORD tick = TimeConverter::TimeToTick(0);*/

	CSkillReuseDelay skillreusedelay;

	for (int i = 0; i < 10; ++i)
	{
		ReuseDelay rd{ i, GetTickCount() + 1000 * i, (DWORD)(1000 * i) };
		skillreusedelay.Insert(rd);
	}
	
	char buf[2000];
	ZeroMemory(buf, sizeof(buf));

	char* packet = buf;
	skillreusedelay.WriteToPacket(packet);
	int cnt = (int)(packet - buf);
	for (int i = 0; i < cnt; ++i)
	{
		char& c = buf[i];
		std::cout << (int)c;
	}
	std::cout << '\n';

	CSkillReuseDelay srd2;
	srd2.SetFromDBPacket(buf);
	
	srd2.TraverseDo([](ReuseDelay& rd)
	{
		printf("%d, %d, %d\n", rd.skillId, rd.nEndTick, rd.nDelayAmount);
	});


	int a;
	Disassemble("", "ddd", &a, &a, &a);

	return 0;
}