// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_LUA_MANAGER_H
#define USRV_LUA_MANAGER_H

#include <time.h>
#include <string>
#include <memory>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "unit.h"

namespace usrv
{
    using LuaBindFunc = std::function<void(luabridge::Namespace)>;
    class LuaManager : public Unit
    {
    public:
        LuaManager(const std::string & path, LuaBindFunc bind_func = nullptr);
        virtual ~LuaManager();

        virtual bool Start() override final;
        virtual void Update(clock_t interval) override final;
        virtual void Stop() override final;

    public:
        lua_State * getLuaState();
        luabridge::LuaRef GetGlobal(const char * name);
        luabridge::Namespace GetGlobalNamespace();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif // USRV_LUA_MANAGER_H
