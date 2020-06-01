// Copyright (c) 2019-2020 TungFai Fong 

#include "unit_manager.h"
#include "common/path.h"
#include "script/lua_manager.h"

class CPPClass
{
public:
    void print()
    {
        std::cout << "call CPPClass print" << std::endl;
    }
};

bool run_lua()
{
    usrv::UnitManager mgr(10);

    std::string path = usrv::PTAH_ROOT + "/../test/script/main.lua";
    usrv::LuaBindFunc bind_func = [](luabridge::Namespace ns)
    {
        ns.beginClass<CPPClass>("CPPClass")
            .addConstructor<void(*)(void)>()
            .addFunction("print", &CPPClass::print)
            .endClass();
    };

    auto lua_manager = std::make_shared<usrv::LuaManager>(path, bind_func);

    mgr.Register("lua_manager", std::move(lua_manager));

    mgr.Run();

    return true;
}
