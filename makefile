CC = gcc
CXX = g++
AR = ar crv

CXX_FLAGS = -Wall -fdiagnostics-color=always -g -std=c++20 -O2

# path
PATH_BUILD = out
PATH_INSTALL = out/install
PATH_INSTALL_INCLUDE = $(PATH_INSTALL)/include
PATH_INSTALL_LIB = $(PATH_INSTALL)/lib
PATH_3RDPARTY = ./3rdparty

TARGET = libusrv.a

PATH_SRC = src
PATH_COMPONENTS = components
PATH_INTERFACES = interfaces
PATH_UNITS = units
PATH_UTIL = util

INSTALL_DIR = $(PATH_COMPONENTS) $(PATH_INTERFACES) $(PATH_UNITS) $(PATH_UTIL)
SUB_DIR = $(PATH_SRC) $(PATH_SRC)/$(PATH_COMPONENTS) $(PATH_SRC)/$(PATH_INTERFACES) $(PATH_SRC)/$(PATH_UNITS) $(PATH_SRC)/$(PATH_UTIL)

# include
INC_ASIO = -I$(PATH_3RDPARTY)/asio-1.20.0
INC_FMT = -I$(PATH_3RDPARTY)/fmt-8.1.0/include
INC_LUA = -I$(PATH_3RDPARTY)/lua-5.4.3/include
INC_LUA_BRIDGE = -I$(PATH_3RDPARTY)/LuaBridge-2.7
INC_SPDLOG = -I$(PATH_3RDPARTY)/spdlog-1.9.2
INC_ALL = -I./src $(INC_ASIO) $(INC_FMT) $(INC_LUA) $(INC_LUA_BRIDGE) $(INC_SPDLOG)

# lib
LIB_FMT = -L$(PATH_3RDPARTY)/fmt-8.1.0/lib -lfmt
LIB_LUA = -L$(PATH_3RDPARTY)/lua-5.4.3/lib -llua
LIB_ALL = $(LIB_FMT) $(LIB_LUA)

LINK_FLAGS = $(LIB_ALL) -lpthread -ldl

# define
DEFINE_SPDLOG = -DSPDLOG_FMT_EXTERNAL
DEFINE += $(DEFINE_SPDLOG)

INSTALL = install -p -m 0644

$(shell if [ ! -e $(PATH_BUILD) ];then mkdir -p $(PATH_BUILD); fi)
$(shell for dir in $(SUB_DIR); do if [ ! -e $(PATH_BUILD)/$$dir ];then mkdir -p $(PATH_BUILD)/$$dir; fi; done)

ifeq ($(MAKECMDGOALS), install)
$(shell if [ ! -e $(PATH_INSTALL_INCLUDE) ];then mkdir -p $(PATH_INSTALL_INCLUDE); fi)
$(shell for dir in $(INSTALL_DIR); do if [ ! -e $(PATH_INSTALL_INCLUDE)/$$dir ];then mkdir -p $(PATH_INSTALL_INCLUDE)/$$dir; fi; done)
$(shell if [ ! -e $(PATH_INSTALL_LIB) ];then mkdir -p $(PATH_INSTALL_LIB); fi)
endif

SOURCE_FILE := $(foreach d, $(SUB_DIR), $(wildcard $(d)/*.cpp))
SOURCE_FILE += $(wildcard ./*.cpp)

OBJ_FILE := $(addprefix $(PATH_BUILD)/, $(addsuffix .o, $(basename $(SOURCE_FILE))))
DEP_FILE := $(patsubst %.o, %.d, $(OBJ_FILE))

# install
INSTALL_SRC = $(wildcard $(PATH_SRC)/*.h $(PATH_SRC)/*.hpp)
INSTALL_COMPONENTS = $(wildcard $(PATH_SRC)/$(PATH_COMPONENTS)/*.h $(PATH_SRC)/$(PATH_COMPONENTS)/*.hpp)
INSTALL_INTERFACES = $(wildcard $(PATH_SRC)/$(PATH_INTERFACES)/*.h $(PATH_SRC)/$(PATH_INTERFACES)/*.hpp)
INSTALL_UNITS = $(wildcard $(PATH_SRC)/$(PATH_UNITS)/*.h $(PATH_SRC)/$(PATH_UNITS)/*.hpp)
INSTALL_UTIL = $(wildcard $(PATH_SRC)/$(PATH_UTIL)/*.h $(PATH_SRC)/$(PATH_UTIL)/*.hpp)

all: $(PATH_BUILD)/$(TARGET)

$(PATH_BUILD)/$(TARGET): $(OBJ_FILE)
	$(AR) -o $@ $(OBJ_FILE)
	@echo Build: $@ 

$(PATH_BUILD)/%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(DEFINE) -MMD $(INC_ALL) $< -o $@
	@echo compiled: $<

ifneq ($(MAKECMDGOALS), clean)
-include $(DEP_FILE)
endif

install:
	$(INSTALL) $(INSTALL_SRC) $(PATH_INSTALL_INCLUDE)
	$(INSTALL) $(INSTALL_COMPONENTS) $(PATH_INSTALL_INCLUDE)/$(PATH_COMPONENTS)
	$(INSTALL) $(INSTALL_INTERFACES) $(PATH_INSTALL_INCLUDE)/$(PATH_INTERFACES)
	$(INSTALL) $(INSTALL_UNITS) $(PATH_INSTALL_INCLUDE)/$(PATH_UNITS)
	$(INSTALL) $(INSTALL_UTIL) $(PATH_INSTALL_INCLUDE)/$(PATH_UTIL)
	$(INSTALL) $(PATH_BUILD)/$(TARGET) $(PATH_INSTALL_LIB)

clean:
	@rm -rf $(PATH_BUILD)
	@echo clean all....

.PHONY: all install clean