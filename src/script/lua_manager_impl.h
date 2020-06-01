// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_LUA_MANAGER_IMPL_H
#define USRV_LUA_MANAGER_IMPL_H

namespace usrv
{
    class LuaManager::Impl
    {
    public:
        Impl(const std::string & path, LuaBindFunc bind_func = nullptr);
        virtual ~Impl();

        bool Start();
        void Update(clock_t interval);
        void Stop();

    public:
        lua_State * getLuaState() { return lua_state_; }
        luabridge::LuaRef GetGlobal(const char * name);
        luabridge::Namespace GetGlobalNamespace();

    private:
        inline bool InitFunc(luabridge::LuaRef & func, const char * func_name);

    private:
        std::string file_;
        lua_State * lua_state_;
        luabridge::LuaRef start_;
        luabridge::LuaRef update_;
        luabridge::LuaRef stop_;
        LuaBindFunc bind_func_;
    };
}

#endif // USRV_LUA_MANAGER_IMPL_H
