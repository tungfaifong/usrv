// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_UNIT_H
#define USRV_UNIT_H

namespace usrv
{
    class Unit
    {
    public:
        Unit() {}
        virtual ~Unit() {}

        virtual bool Start() = 0;
        virtual void Update(clock_t interval) = 0;
        virtual void Stop() = 0;
    };
}

#endif // USRV_UNIT_H
