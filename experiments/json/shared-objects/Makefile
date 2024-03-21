CC := clang-14
CXX := clang++-14

BUILD_DIR := build
LIB_DIR := libs
SRC_DIR := src

BUILD_LIBS_DIR := $(BUILD_DIR)/$(LIB_DIR)
ABSOLUTE_LIB_DIR := $(shell pwd)/libs

# spidermonkey

# SPIDERMONKEY_DEBUG := 1

ifdef SPIDERMONKEY_DEBUG
BUILD_SPIDERMONKEY_DIR := $(BUILD_LIBS_DIR)/spidermonkey/debug
SPIDERMONKEY_DEBUG_FLAG := -DDEBUG
else
BUILD_SPIDERMONKEY_DIR := $(BUILD_LIBS_DIR)/spidermonkey/release
endif

#
# Flags
#
CFLAGS_NO_WERROR := -O2 -g -Wall
CFLAGS := $(CFLAGS_NO_WERROR) -Werror

CFLAGS_SHARED_O := -fPIC -fvisibility=hidden
CFLAGS_SHARED_LIB := $(CFLAGS_SHARED_O) -rdynamic -shared

COV_ASAN_FLAGS := -fsanitize=address
COV_FULL_FLAGS := $(COV_ASAN_FLAGS) -fsanitize-coverage=inline-8bit-counters,pc-table -fsanitize-recover=address -fno-omit-frame-pointer

CFLAGS_LIBS_NO_WERROR := $(CFLAGS_NO_WERROR) $(CFLAGS_SHARED_O)
CFLAGS_LIBS := $(CFLAGS) $(CFLAGS_SHARED_O)
CXX_FLAGS := --std=c++11 --stdlib=libstdc++
CXX_17_FLAGS := --std=c++17 --stdlib=libstdc++
LD_MAIN := -ldl -pthread -lc++

# We do not list v8 here, because it is built with v8's build tools
ALL_LIBS = \
	ccan \
	cjson \
	frozen \
	json-c \
	json-parser \
	jsoncpp \
	libjson \
	poco \
	tiny-json \
	yajl \
	json \
	jsonh \
	jsmn \
	rapidjson \
	boost \
	serde \
	jansson \
	spidermonkey \

all: build

include $(LIB_DIR)/Makefile
include $(SRC_DIR)/Makefile

build: $(SHARED_LIBS)
