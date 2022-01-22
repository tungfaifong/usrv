// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LUA_UNIT_H
#define USRV_LUA_UNIT_H

#include <functional>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "unit.h"

NAMESPACE_OPEN

class LuaUnit : public Unit
{
public:
	using LuaBindFunc = std::function<void(luabridge::Namespace)>;

	LuaUnit(const std::string & path, LuaBindFunc bind_func = nullptr);
	virtual ~LuaUnit() = default;

	virtual bool Init() override final;
	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;
	virtual void Release() override final;

public:
	lua_State * GetLuaState();
	inline luabridge::LuaRef GetGlobal(const char * name);
	inline luabridge::Namespace GetGlobalNamespace();

	void OnRecvFunc(NETID net_id, char * data, uint16_t size);

private:
	bool _InitFunc(luabridge::LuaRef & func, const char * func_name);

private:
	std::string _file;
	lua_State * _lua_state;
	luabridge::LuaRef _start;
	luabridge::LuaRef _update;
	luabridge::LuaRef _stop;
	luabridge::LuaRef _on_recv;
	LuaBindFunc _bind_func;
};
	
NAMESPACE_CLOSE

#endif // USRV_LUA_UNIT_H
