// Copyright (c) 2019-2020 TungFai Fong 

#include "timer_manager.h"
#include "timer_manager_impl.h"

namespace usrv
{
    //TimerManagerImpl
    TimerManager::Impl::Impl()
    {
    }

    TimerManager::Impl::~Impl()
    {
    }

    void TimerManager::Impl::Update(clock_t interval)
    {
        for (auto & timer : timers_)
        {
            timer.time_ -= interval;
            if (timer.time_ <= 0)
            {
                timer.timer_->OnCallback();
            }
        }

        for (auto iter = timers_.begin(); iter != timers_.end();)
        {
            iter->time_ <= 0 ? iter = timers_.erase(iter) : ++iter;
        }
    }

    void TimerManager::Impl::CreateTimer(clock_t time, std::shared_ptr<Timer> && timer)
    {
        TimerImpl t(time, std::move(timer));
        timers_.emplace_back(std::move(t));
    }

    //TimerManager
    TimerManager::TimerManager() : Unit(),
        impl_(std::make_unique<Impl>())
    {
    }

    TimerManager::~TimerManager()
    {
    }

    bool TimerManager::Start()
    {
        return true;
    }

    void TimerManager::Update(clock_t interval)
    {
        impl_->Update(interval);
    }

    void TimerManager::Stop()
    {
    }

    void TimerManager::CreateTimer(clock_t time, std::shared_ptr<Timer> && timer)
    {
        impl_->CreateTimer(time, std::move(timer));
    }
}
