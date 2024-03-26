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
BUILD_SPIDERMONKEY_DIR := /build/spidermonkey/debug
SPIDERMONKEY_DEBUG_FLAG := -DDEBUG
else
BUILD_SPIDERMONKEY_DIR := /build/spidermonkey/release
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

include $(LIB_DIR)/Makefile
include $(SRC_DIR)/Makefile

$(BUILD_SPIDERMONKEY_DIR)/js/src/build/libmozjs-114a1.so:
	./build_spidermonkey.sh

/build/v8/v8/out/paco_component/v8_allowlist.txt:
	cd /build/v8/v8 && gn gen out/paco_component --args='is_asan=true is_debug=false sanitizer_coverage_flags="inline-8bit-counters,pc-table" sanitizer_coverage_allowlist_flags="v8_allowlist.txt" target_cpu="x64" is_component_build=true v8_use_external_startup_data=false'
	/build/v8/v8/build && git apply ../../../../v8-build.patch
	cp v8_allowlist.txt /build/v8/v8/out/paco_component/v8_allowlist.txt

/build/v8/v8/paco/v8_adapter.cpp: src/adapter/v8_adapter.cpp
	mkdir -p $(dir $@)
	cp src/adapter/v8_adapter.cpp $@

/build/v8/v8/paco/generic_target.cpp: ../../../src/interface/generic_target.cpp
	mkdir -p $(dir $@)
	cp ../../../src/interface/generic_target.cpp $@

/build/v8/v8/paco/common.h: ../../../src/interface/common.h
	mkdir -p $(dir $@)
	cp ../../../src/interface/common.h $@

/build/v8/v8/out/paco_component/libpaco.so: /build/v8/v8/out/paco_component/v8_allowlist.txt /build/v8/v8/paco/v8_adapter.cpp /build/v8/v8/paco/generic_target.cpp /build/v8/v8/paco/common.h
	cd /build/v8 && ninja -C out/paco_component paco

prebuild: $(BUILD_SPIDERMONKEY_DIR)/js/src/build/libmozjs-114a1.so /build/v8/v8/out/paco_component/libpaco.so

shared_libs_build: $(SHARED_LIBS)

.PHONY: prebuild shared_libs_build