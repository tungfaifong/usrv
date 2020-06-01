// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>

#include "unit_manager.h"
#include "timer/timer_manager.h"

bool run_timer()
{
    usrv::UnitManager mgr(10);
    mgr.Register("timer_manager", std::move(std::make_shared<usrv::TimerManager>()));

    static auto timer_manager = std::dynamic_pointer_cast<usrv::TimerManager>(mgr.Get("timer_manager"));

    class TestTimer : public usrv::Timer
    {
    public:
        virtual void OnCallback() override
        {
            std::cout << "call OnCallback" << std::endl;
            timer_manager->CreateTimer(5 * 1000, std::make_shared<TestTimer>());
        }
    };

    timer_manager->CreateTimer(5 * 1000, std::make_shared<TestTimer>());

    mgr.Run();

    return true;
}
