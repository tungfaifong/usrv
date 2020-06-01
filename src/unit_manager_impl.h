// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_UNIT_MANAGER_IMPL_H
#define USRV_UNIT_MANAGER_IMPL_H

#include <map>

namespace usrv
{
    class UnitManager::Impl
    {
    public:
        Impl(clock_t interval);
        ~Impl();

    public:
        asio::io_context & IOContext() { return io_context_; }

    public:
        bool Register(const std::string & name, std::shared_ptr<Unit> unit);
        std::shared_ptr<Unit> Get(std::string name);

        void Run();

    private:
        bool Start();
        void Update(clock_t interval);
        void Stop();
        void MainLoop();

    private:
        clock_t interval_;
        asio::io_context io_context_;
        std::map<std::string, std::shared_ptr<Unit>> units_;
    };
}

#endif // USRV_UNIT_MANAGER_IMPL_H
