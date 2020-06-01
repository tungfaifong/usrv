// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TIMER_MANAGER_H
#define USRV_TIMER_MANAGER_H

#include <time.h>
#include <memory>

#include "unit.h"

namespace usrv
{
    class Timer
    {
    public:
        Timer() {}
        virtual ~Timer() {}

    public:
        virtual void OnCallback() = 0;
    };

    class TimerManager : public Unit
    {
    public:
        TimerManager();
        ~TimerManager();

        virtual bool Start() override final;
        virtual void Update(clock_t interval) override final;
        virtual void Stop() override final;

    public:
        void CreateTimer(clock_t time, std::shared_ptr<Timer> && timer);

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif // USRV_TIMER_MANAGER_H
