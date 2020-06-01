// Copyright (c) 2019-2020 TungFai Fong 

#include "unit_manager.h"
#include "unit_manager_impl.h"

#include "spdlog/spdlog.h"

#include "common/time.h"
#include "unit.h"

namespace usrv
{
    //UnitManagerImpl
    UnitManager::Impl::Impl(clock_t interval) :interval_(interval),
        io_context_()
    {
    }

    UnitManager::Impl::~Impl()
    {
        Stop();
    }

    bool UnitManager::Impl::Register(const std::string & name, std::shared_ptr<Unit> unit)
    {
        if (units_.find(name) != units_.end())
        {
            return false;
        }

        units_.insert(std::make_pair(name, std::move(unit)));

        return true;
    }

    std::shared_ptr<Unit> UnitManager::Impl::Get(std::string name)
    {
        auto iter = units_.find(name);
        if (iter == units_.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    void UnitManager::Impl::Run()
    {
        if (!Start())
        {
            spdlog::error("UnitManager start failed.");
            return;
        }

        std::thread thread_io([this]() { io_context_.run(); });

        MainLoop();

        thread_io.join();
    }

    bool UnitManager::Impl::Start()
    {
        spdlog::info("UnitManager starting.");
        for (auto & unit : units_)
        {
            if (!unit.second->Start())
            {
                spdlog::error("{} start failed.", unit.first);
                return false;
            }
        }
        spdlog::info("UnitManager started.");
        return true;
    }

    void UnitManager::Impl::Update(clock_t interval)
    {
        for (auto & unit : units_)
        {
            unit.second->Update(interval);
        }
    }

    void UnitManager::Impl::Stop()
    {
        for (auto & unit : units_)
        {
            unit.second->Stop();
        }
    }

    void UnitManager::Impl::MainLoop()
    {
        auto start = Clock();
        while (true)
        {
            auto now = Clock();
            auto interval = now - start;
            if (interval >= interval_)
            {
                Update(interval);
                start = now;
            }
        }
    }

    //UnitManager
    UnitManager::UnitManager(clock_t interval) : impl_(std::make_unique<Impl>(interval))
    {
    }

    UnitManager::~UnitManager()
    {
    }

    asio::io_context & UnitManager::IOContext()
    {
        return impl_->IOContext();
    }

    bool UnitManager::Register(const std::string & name, std::shared_ptr<Unit> unit)
    {
        return impl_->Register(name, unit);
    }

    std::shared_ptr<Unit> UnitManager::Get(const std::string & name)
    {
        return impl_->Get(name);
    }

    void UnitManager::Run()
    {
        impl_->Run();
    }
}
