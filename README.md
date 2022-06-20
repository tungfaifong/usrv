**usrv - 一个游戏服务器脚手架静态库，帮助开发者快速搭建游戏服务器框架。**

## 总览

## 依赖

- asio-1.20.0
- fmt-8.1.0
- lua-5.4.3
- LuaBridge-2.7
- spdlog-1.9.2

## 编译

#### linux

- gcc-11.2.0

```sh
cd usrv
make 
make install PATH_INSTALL = out/install
```

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
