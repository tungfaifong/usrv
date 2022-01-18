# base
CC = gcc
CXX = g++

CXX_FLAGS = -Wall -fdiagnostics-color=always -g -std=c++20 -O2

# 文件单独
TARGET := main
SUB_DIR = src src/interfaces src/units src/util demo demo/network demo/timer

BUILD_DIR := out

PATH_3RDPARTY = ./3rdparty

INC_ASIO = -I$(PATH_3RDPARTY)/asio-1.20.0
INC_FMT = -I$(PATH_3RDPARTY)/fmt-8.1.0/include
INC_LUA = -I$(PATH_3RDPARTY)/lua-5.4.3/include
INC_LUA_BRIDGE = -I$(PATH_3RDPARTY)/LuaBridge-2.6
INC_ALL := -I./src $(INC_ASIO) $(INC_FMT) $(INC_LUA) $(INC_LUA_BRIDGE)

LIB_FMT = -L$(PATH_3RDPARTY)/fmt-8.1.0/lib -lfmt
LIB_LUA = -L$(PATH_3RDPARTY)/lua-5.4.3/lib -llua
LIB_ALL := $(LIB_FMT) $(LIB_LUA)

LINK_FLAGS := $(LIB_ALL) -lpthread

DEFINE += 

# mksvr
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)
$(shell for dir in $(SUB_DIR); do if [ ! -e $(BUILD_DIR)/$$dir ];then mkdir -p $(BUILD_DIR)/$$dir; fi; done)

SOURCE_FILE := $(foreach d, $(SUB_DIR), $(wildcard $(d)/*.cpp))
SOURCE_FILE += $(wildcard ./*.cpp)

OBJ_FILE := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(SOURCE_FILE))))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ_FILE)
	$(CXX) -o $@ $(CXX_FLAGS) $(DEFINE) $(OBJ_FILE) $(LINK_FLAGS)
	@echo Build: $@ 

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(DEFINE) -MMD $(INC_ALL) $< -o $@
	@echo compiled: $<

clean:
	@rm -rf $(BUILD_DIR)
	@echo clean all....