// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include <iostream>

#include "unit_manager.h"
#include "units/timer_unit.h"
#include "util/time.h"

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
	usrv::UnitManager::Instance()->Init(UNITKEY::COUNT, UNITKEYSTR);
	usrv::UnitManager::Instance()->Register(UNITKEY::TIMER, std::move(std::make_shared<usrv::TimerUnit>()));

	static auto timer_manager = std::dynamic_pointer_cast<usrv::TimerUnit>(usrv::UnitManager::Instance()->Get(UNITKEY::TIMER));

	printf("main start\n");

	CallObj a;
	timer_manager->CreateTimer(3 * usrv::SEC2MILLISEC, call1);
	timer_manager->CreateTimer(4 * usrv::SEC2MILLISEC, [](){ printf("call timeout 2\n"); });
	timer_manager->CreateTimer(5 * usrv::SEC2MILLISEC, CallObj::call3);
	timer_manager->CreateTimer(6 * usrv::SEC2MILLISEC, std::bind(&CallObj::call4, &a));

	printf("main run\n");
	usrv::UnitManager::Instance()->Run(10);

	printf("main exit\n");

	return 0;
}