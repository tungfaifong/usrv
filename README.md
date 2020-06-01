**usrv - 一个游戏服务器脚手架静态库，帮助开发者快速搭建游戏服务器框架。**

## 总览

- 基于asio提供tcp服务器，裸二进制方式进行数据传输。
- 基于asio提供rpc服务器，裸二进制方式进行数据传输。
- 基于LuaBridge提供lua脚本嵌入。
- 提供简单计时器功能。
- UnitManager提供游戏循环。

## 依赖

- asio-1.12.2
- lua-5.2.0
- LuaBridge-2.4.1
- protobuf-3.11.3
- spdlog-1.5.0

## 编译

### 编译依赖

将依赖的第三方库按CMakeLists中的路径放到3rdparty目录中，或者通过软链方式链接。

### 编译环境

cmake-3.10.2

#### linux

- gcc-7.4.0

```sh
mkdir build 
cd build 
cmake -DCMAKE_INSTALL_PREFIX=./install ..
make
make install
```

#### Windows

- VisualStudio 2017

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=./install ..
```

打开build的项目，生成INSTALL
编译完之后会在install中获得include头文件和lib静态库，复制放置到项目include和lib中即可。

## 如何使用

### 游戏服务器

```c++
#include "unit_manager.h"
#include "unit.h"

class Game : public usrv::Unit
{
public:
    Game() : Unit() {}
    virtual ~Game() {}

    virtual bool Start() override {}
    virtual void Update(clock_t interval) override {}
    virtual void Stop() override {}
};

int main(int argc, char* argv[])
{
    // 每帧间隔10ms
    usrv::UnitManager mgr(10);

    auto game = std::make_shared<Game>();

    mgr.Register("game", std::move(game));

    // 启动mgr
    mgr.Run();
}
```

每个服务器需要一个UnitManager，所有的逻辑通过UnitManager来驱动，游戏逻辑服务器需要继承自Unit类，然后注册到UnitManager中。

### TcpServer

参考demo/network/tcp_server.cpp

### RpcServer

参考demo/rpc/rpc_server.cpp

### LuaManager

参考demo/script/test_lua.cpp

### TimerManager

参考demo/timer/test_timer.cpp

## 文档

TODO