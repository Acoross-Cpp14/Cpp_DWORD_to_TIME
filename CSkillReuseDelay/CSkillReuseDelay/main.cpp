#include <iostream>

#include "CSkillReuseDelay.h"
#include "FormatChecker.h"

void AssemblerTest()
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
}

template <typename _CountofType, size_t N>
char	(*	
			__countof_helper2(	_CountofType(&_Array)[N]	)	
		)[N];

#define __fff_countof(_Array) (sizeof(*__countof_helper2(_Array)) + 0)


int main(int argc, char* argv[])
{
	TestFormatChecker();

	_countof("haha");
	//static_assert(::foobar<0>("df", 1), "no");
		
	/*static_assert(GetArrayVal<0>(arr) == 0, "ok");*/
	//_CheckFormatString_One_impl<0>("df", 1);

	return 0;
}