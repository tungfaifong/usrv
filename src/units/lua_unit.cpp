// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "lua_unit.h"

#include "interfaces/logger_interface.h"
#include "interfaces/server_interface.h"
#include "interfaces/timer_interface.h"

NAMESPACE_OPEN

LuaUnit::LuaUnit(const std::string & file, ExposeFunc func) : _file(PATH_ROOT + "/" + file), _lua_state(luaL_newstate()),
	_start(nullptr), _update(nullptr), _stop(nullptr), _on_conn(nullptr), _on_recv(nullptr), _on_disc(nullptr), _expose(func)
{
	
}

bool LuaUnit::Init()
{
	luaL_openlibs(_lua_state);

	if (luaL_dofile(_lua_state, _file.c_str()))
	{
		LOGGER_ERROR("LuaUnit::Init error: {}", lua_tostring(_lua_state, -1));
		return false;
	}

	if (!InitFunc(_start, "Start") || !InitFunc(_update, "Update") || !InitFunc(_stop, "Stop") ||
		!InitFunc(_on_conn, "OnConn") || !InitFunc(_on_recv, "OnRecv") || !InitFunc(_on_disc, "OnDisc"))
	{
		LOGGER_ERROR("LuaUnit::Init error: _InitFunc filed.");
		return false;
	}

	_Expose();

	if (_expose)
	{
		_expose(GetGlobalNamespace());
	}

	return true;
}

bool LuaUnit::Start()
{
	auto success = true;
	try
	{
		success = (*_start)();
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
	return success;
}

bool LuaUnit::Update(intvl_t interval)
{
	auto busy = true;
	try
	{
		busy = (*_update)(interval);
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
	return busy;
}

void LuaUnit::Stop()
{
	try
	{
		(*_stop)();
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
}

void LuaUnit::Release()
{
	_start = nullptr;
	_update = nullptr;
	_stop = nullptr;
	_on_conn = nullptr;
	_on_recv = nullptr;
	_on_disc = nullptr;
	lua_close(_lua_state);
	Unit::Release();
}

lua_State * LuaUnit::GetLuaState()
{
	return _lua_state;
}

luabridge::LuaRef LuaUnit::GetGlobal(const char * name)
{
	return luabridge::getGlobal(_lua_state, name);
}

luabridge::Namespace LuaUnit::GetGlobalNamespace()
{
	return luabridge::getGlobalNamespace(_lua_state);
}

void LuaUnit::OnConn(NETID net_id, const IP & ip, PORT port)
{
	try
	{
		(*_on_conn)(net_id, ip, port);
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
}

void LuaUnit::OnRecv(NETID net_id, const char * data, uint16_t size)
{
	try
	{
		(*_on_recv)(net_id, std::string(data, size));
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
}

void LuaUnit::OnDisc(NETID net_id)
{
	try
	{
		(*_on_disc)(net_id);
	}
	catch(const luabridge::LuaException & e)
	{
		OnException(e);
	}
}

void LuaUnit::OnException(const luabridge::LuaException & e)
{
	LOGGER_ERROR("{}", e.what());
}

bool LuaUnit::InitFunc(std::shared_ptr<luabridge::LuaRef> & func, const char * func_name)
{
	func = std::make_shared<luabridge::LuaRef>(GetGlobal(func_name));
	if (!func->isFunction())
	{
		LOGGER_ERROR("lua func error: missing {}", func_name);
		return false;
	}
	return true;
}

void LuaUnit::_Expose()
{
	GetGlobalNamespace()
		.beginNamespace("engine")
			.addFunction("Now", Now)
			.addFunction("NowMs", NowMs)
		.beginNamespace("logger")
			.addFunction("trace", logger::lua_trace)
			.addFunction("debug", logger::lua_debug)
			.addFunction("info", logger::lua_info)
			.addFunction("warn", logger::lua_warn)
			.addFunction("error", logger::lua_error)
			.addFunction("critical", logger::lua_critical)
			.addFunction("flush", logger::flush)
		.endNamespace()
		.beginNamespace("server")
			.addFunction("Connect", server::Connect)
			.addFunction("Disconnect", server::Disconnect)
			.addFunction("Send", server::Send)
		.endNamespace()
		.beginNamespace("timer")
			.addFunction("CreateTimer", timer::LuaCreateTimer)
			.addFunction("CallTimer", timer::CallTimer)
			.addFunction("RemoveTimer", timer::RemoveTimer)
		.endNamespace();
}

NAMESPACE_CLOSE
