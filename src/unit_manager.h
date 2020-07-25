// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_UNIT_MANAGER_H
#define USRV_UNIT_MANAGER_H

#include "asio.hpp"

namespace usrv
{
    class Unit;

    class UnitManager
    {
    public:
        UnitManager(clock_t interval);
        ~UnitManager();

    public:
        asio::io_context & IOContext();
        clock_t Interval();

    public:
        bool Register(const std::string & name, std::shared_ptr<Unit> unit);
        std::shared_ptr<Unit> Get(const std::string & name);

        void Run();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif // USRV_UNIT_MANAGER_H
