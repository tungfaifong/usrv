// Copyright (c) 2019-2020 TungFai Fong 

#include "lua_manager.h"
#include "lua_manager_impl.h"

#include "spdlog/spdlog.h"

namespace usrv
{
    //LuaManagerImpl
    LuaManager::Impl::Impl(const std::string & file, LuaBindFunc bind_func) : file_(file),
        lua_state_(luaL_newstate()),
        start_(nullptr),
        update_(nullptr),
        stop_(nullptr),
        bind_func_(bind_func)
    {
        luaL_openlibs(lua_state_);
    }

    LuaManager::Impl::~Impl()
    {
        lua_close(lua_state_);
    }

    bool LuaManager::Impl::Start()
    {
        if (luaL_dofile(lua_state_, file_.c_str()))
        {
            spdlog::error("file error: missing {}", file_);
            return false;
        }

        if (!InitFunc(start_, "Start") ||
            !InitFunc(update_, "Update") ||
            !InitFunc(stop_, "Stop"))
        {
            return false;
        }

        if (bind_func_ != nullptr)
        {
            bind_func_(GetGlobalNamespace());
        }

        return start_();
    }

    void LuaManager::Impl::Update(clock_t interval)
    {
        update_(interval);
    }

    void LuaManager::Impl::Stop()
    {
        stop_();
    }

    luabridge::LuaRef LuaManager::Impl::GetGlobal(const char * name)
    {
        return luabridge::getGlobal(lua_state_, name);
    }

    luabridge::Namespace LuaManager::Impl::GetGlobalNamespace()
    {
        return luabridge::getGlobalNamespace(lua_state_);
    }

    inline bool LuaManager::Impl::InitFunc(luabridge::LuaRef & func, const char * func_name)
    {
        func = GetGlobal(func_name);
        if (!func.isFunction())
        {
            spdlog::error("lua func error: missing {}", func_name);
            return false;
        }
        return true;
    }

    //LuaManager
    LuaManager::LuaManager(const std::string & file, LuaBindFunc bind_func) : Unit(),
        impl_(std::make_unique<Impl>(file, bind_func))
    {
    }

    LuaManager::~LuaManager()
    {
    }

    bool LuaManager::Start()
    {
        return impl_->Start();
    }

    void LuaManager::Update(clock_t interval)
    {
        impl_->Update(interval);
    }

    void LuaManager::Stop()
    {
        impl_->Stop();
    }

    lua_State * LuaManager::getLuaState()
    {
        return impl_->getLuaState();
    }

    luabridge::LuaRef LuaManager::GetGlobal(const char * name)
    {
        return impl_->GetGlobal(name);
    }

    luabridge::Namespace LuaManager::GetGlobalNamespace()
    {
        return impl_->GetGlobalNamespace();
    }
}
