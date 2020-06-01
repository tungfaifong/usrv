// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TIMER_MANAGER_IMPL_H
#define USRV_TIMER_MANAGER_IMPL_H

#include <vector>

namespace usrv
{
    class TimerManager::Impl
    {
    public:
        Impl();
        ~Impl();

        void Update(clock_t interval);

    public:
        void CreateTimer(clock_t time, std::shared_ptr<Timer> && timer);

    private:
        struct TimerImpl
        {
            TimerImpl(clock_t time, std::shared_ptr<Timer> && timer) : time_(time), timer_(std::move(timer)) {}
            clock_t time_;
            std::shared_ptr<Timer> timer_;
        };
        std::vector<TimerImpl> timers_;
    };
}

#endif // USRV_TIMER_MANAGER_IMPL_H
