// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "interfaces/timer_interface.hpp"
#include "unit_manager.h"
#include "units/logger_unit.h"
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
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(1 Mi)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));

	CallObj a;
	timer::CreateTimer(3 * SEC2MILLISEC, call1);
	auto key2 = timer::CreateTimer(4 * SEC2MILLISEC, [](){ printf("call timeout 2\n"); });
	auto key3 = timer::CreateTimer(5 * SEC2MILLISEC, CallObj::call3);
	timer::CreateTimer(6 * SEC2MILLISEC, std::bind(&CallObj::call4, &a));
	timer::CallTimer(key3);
	timer::RemoveTimer(key2);

	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Run();
	UnitManager::Instance()->Release();

	return 0;
}