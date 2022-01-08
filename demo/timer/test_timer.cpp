// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"
#include "units/timer_unit.h"
#include "util/time.h"

using namespace usrv;

enum UNITKEY
{
	TIMER = 0,
	COUNT,
};

const std::string UNITKEYSTR[UNITKEY::COUNT] = 
{
	"TIMER"
};

void call1()
{
	printf("call timeout 1\n");
}

class CallObj
{
public:
	static void call3() { printf("call timeout 3\n"); }
	void call4() { printf("call timeout 4\n"); }
};

int run_timer()
{
	UnitManager::Instance()->Init(UNITKEY::COUNT, UNITKEYSTR);
	UnitManager::Instance()->Register(UNITKEY::TIMER, std::move(std::make_shared<TimerUnit>()));

	static auto timer_manager = std::dynamic_pointer_cast<TimerUnit>(UnitManager::Instance()->Get(UNITKEY::TIMER));

	CallObj a;
	timer_manager->CreateTimer(3 * SEC2MILLISEC, call1);
	timer_manager->CreateTimer(4 * SEC2MILLISEC, [](){ printf("call timeout 2\n"); });
	timer_manager->CreateTimer(5 * SEC2MILLISEC, CallObj::call3);
	timer_manager->CreateTimer(6 * SEC2MILLISEC, std::bind(&CallObj::call4, &a));

	UnitManager::Instance()->Run(10);

	return 0;
}