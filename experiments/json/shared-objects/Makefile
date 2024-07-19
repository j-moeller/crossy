CC := clang-14
CXX := clang++-14

CFLAGS_NO_WERROR := -O2 -g -Wall
CFLAGS := $(CFLAGS_NO_WERROR) -Werror

CFLAGS_SHARED_O := -fPIC -fvisibility=hidden
CFLAGS_SHARED_LIB := $(CFLAGS_SHARED_O) -rdynamic -shared

COV_ASAN_FLAGS := -fsanitize=address
COV_FULL_FLAGS := $(COV_ASAN_FLAGS) -fsanitize-coverage=inline-8bit-counters,pc-table -fsanitize-recover=address -fno-omit-frame-pointer

CFLAGS_LIBS := $(CFLAGS) $(CFLAGS_SHARED_O)
CXX_FLAGS := --std=c++11 --stdlib=libstdc++
CXX_17_FLAGS := --std=c++17 --stdlib=libstdc++
LD_MAIN := -ldl -pthread -lc++

GLOBALS := \
	CC="${CC}" \
	CXX="${CXX}" \
	CFLAGS_NO_WERROR="${CFLAGS_NO_WERROR}" \
	CFLAGS="${CFLAGS}" \
	CFLAGS_SHARED_O="${CFLAGS_SHARED_O}" \
	CFLAGS_SHARED_LIB="${CFLAGS_SHARED_LIB}" \
	COV_ASAN_FLAGS="${COV_ASAN_FLAGS}" \
	COV_FULL_FLAGS="${COV_FULL_FLAGS}" \
	CFLAGS_LIBS="${CFLAGS_LIBS}" \
	CXX_FLAGS="${CXX_FLAGS}" \
	CXX_17_FLAGS="${CXX_17_FLAGS}" \
	LD_MAIN="${LD_MAIN}"

WORKDIR := $(shell pwd)

all: \
	build/boost/boost.so \
	build/ccan/ccan.so \
	build/cjson/cjson.so \
	build/frozen/frozen.so \
	build/jansson/jansson.so \
	build/jsmn/jsmn.so \
	build/json-c/json-c.so \
	build/json-parser/json-parser.so \
	build/jsonh/jsonh.so \
	build/nlohmann/nlohmann.so \
	build/jsoncpp/jsoncpp.so \
	build/libjson/libjson.so \
	build/poco/poco.so \
	build/spidermonkey/spidermonkey.so \
	build/rapidjson/rapidjson.so \
	build/v8/libv8-crossy.so \
	build/yajl/yajl.so \
	build/serde/serde.so

include libs/boost/Makefile
include libs/ccan/Makefile
include libs/cjson/Makefile
include libs/frozen/Makefile
include libs/jansson/Makefile
include libs/jsmn/Makefile
include libs/json-c/Makefile
include libs/json-parser/Makefile
include libs/jsonh/Makefile
include libs/nlohmann/Makefile
include libs/jsoncpp/Makefile
include libs/libjson/Makefile
include libs/poco/Makefile
include libs/spidermonkey/Makefile
include libs/rapidjson/Makefile
include libs/v8/Makefile
include libs/yajl/Makefile

include libs/serde/Makefile