CC = gcc
CXX = g++

ifeq ($(MAKECMDGOALS), debug)
	CXX_FLAGS = -Wall -fdiagnostics-color=always -g -std=c++20
else
	CXX_FLAGS = -Wall -fdiagnostics-color=always -g -std=c++20 -O2
endif

# path
PATH_ROOT = ..
PATH_3RDPARTY = $(PATH_ROOT)/3rdparty
PATH_BUILD = out

TARGET = $(PATH_BUILD)/main
SUB_DIR = src src/interfaces src/units src/util demo

# include
INC_ASIO = -I$(PATH_3RDPARTY)/asio-1.20.0
INC_FMT = -I$(PATH_3RDPARTY)/fmt-8.1.0/include
INC_JEMALLOC = -I$(PATH_3RDPARTY)/jemalloc-5.3.0/include
INC_LUA = -I$(PATH_3RDPARTY)/lua-5.4.3/include
INC_LUA_BRIDGE = -I$(PATH_3RDPARTY)/LuaBridge-2.7
INC_SPDLOG = -I$(PATH_3RDPARTY)/spdlog-1.9.2
INC_ALL = -I$(PATH_ROOT)/src $(INC_ASIO) $(INC_FMT) $(INC_JEMALLOC) $(INC_LUA) $(INC_LUA_BRIDGE) $(INC_SPDLOG)

# lib
LIB_FMT = -L$(PATH_3RDPARTY)/fmt-8.1.0/lib -lfmt
LIB_JEMALLOC = -L$(PATH_3RDPARTY)/jemalloc-5.3.0/lib -ljemalloc
LIB_LUA = -L$(PATH_3RDPARTY)/lua-5.4.3/lib -llua
LIB_ALL = $(LIB_FMT) $(LIB_JEMALLOC) $(LIB_LUA)

LINK_FLAGS = $(LIB_ALL) -lpthread -ldl

# define
DEFINE_SPDLOG = -DSPDLOG_FMT_EXTERNAL
DEFINE += $(DEFINE_SPDLOG)

$(shell if [ ! -e $(PATH_BUILD) ];then mkdir -p $(PATH_BUILD); fi)
$(shell for dir in $(SUB_DIR); do if [ ! -e $(PATH_BUILD)/$$dir ];then mkdir -p $(PATH_BUILD)/$$dir; fi; done)

SOURCE_FILE := $(foreach d, $(SUB_DIR), $(wildcard $(PATH_ROOT)/$(d)/*.cpp))
SOURCE_FILE := $(patsubst $(PATH_ROOT)/%, %, $(SOURCE_FILE))

OBJ_FILE := $(addprefix $(PATH_BUILD)/, $(addsuffix .o, $(basename $(SOURCE_FILE))))
DEP_FILE := $(patsubst %.o, %.d, $(OBJ_FILE))

all: $(TARGET)
debug: $(TARGET)

$(TARGET): $(OBJ_FILE)
	$(CXX) -o $@ $(CXX_FLAGS) $(DEFINE) $(OBJ_FILE) $(LINK_FLAGS)
	@echo Build: $@ 

$(PATH_BUILD)/%.o: $(PATH_ROOT)/%.cpp
	$(CXX) -c $(CXX_FLAGS) $(DEFINE) -MMD $(INC_ALL) $< -o $@
	@echo compiled: $<

ifneq ($(MAKECMDGOALS), clean)
-include $(DEP_FILE)
endif

clean:
	@rm -rf $(PATH_BUILD)
	@echo clean all....