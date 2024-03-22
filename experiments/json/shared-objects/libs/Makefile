#
# Library Object Files
#

# test if coverage flags are used using -DCMAKE_VERBOSE_MAKEFILE=ON

$(BUILD_LIBS_DIR)/cjson.o: $(LIB_DIR)/cJSON/cJSON.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@

$(BUILD_LIBS_DIR)/libjson.o: $(LIB_DIR)/libjson/json.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@

$(BUILD_LIBS_DIR)/tiny-json.o: $(LIB_DIR)/tiny-json/tiny-json.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@

$(BUILD_LIBS_DIR)/ccan.o: $(LIB_DIR)/ccan/ccan/json/json.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@

$(BUILD_LIBS_DIR)/json-parser.o: $(LIB_DIR)/json-parser/json.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@

# json-builder (complement to json-parser)
$(BUILD_LIBS_DIR)/json-builder.o: $(LIB_DIR)/json-builder/json-builder.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -I$(LIB_DIR)/json-parser -c $< -o $@

$(BUILD_LIBS_DIR)/frozen.o: $(LIB_DIR)/frozen/frozen.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIBS) $(COV_FULL_FLAGS) -c $< -o $@


#
# Libraries
#

# Yajl
# LIBYAJL := $(LIB_DIR)/yajl/build/yajl-2.1.1/lib/libyajl.so
LIB_YAJL_LDLIBS := -lyajl -Wl,-rpath,$(ABSOLUTE_LIB_DIR)/yajl/build/yajl-2.1.1/lib
LIB_YAJL_LDFLAGS := -L$(ABSOLUTE_LIB_DIR)/yajl/build/yajl-2.1.1/lib
LIB_YAJL_CFLAGS := -I$(ABSOLUTE_LIB_DIR)/yajl/build/yajl-2.1.1/include
LIBYAJL_A := $(LIB_DIR)/yajl/build/yajl-2.1.1/lib/libyajl_s.a

# grep -qxF 'SET(CMAKE_C_FLAGS "$${CMAKE_C_FLAGS} ${COV_FULL_FLAGS}")' $(LIB_DIR)/yajl/CMakeLists.txt || echo 'SET(CMAKE_C_FLAGS "$${CMAKE_C_FLAGS} ${COV_FULL_FLAGS}")' >> $(LIB_DIR)/yajl/CMakeLists.txt
$(LIB_DIR)/yajl/build:
	grep -qxF 'add_compile_options(${COV_FULL_FLAGS})' $(LIB_DIR)/yajl/CMakeLists.txt || sed -i '18s/^/add_compile_options(${COV_FULL_FLAGS})\nlink_libraries(-fsanitize=address)\n/' $(LIB_DIR)/yajl/CMakeLists.txt
	mkdir -p $@
	cd $@ && cmake -DCMAKE_CXX_COMPILER=clang++-14 -DCMAKE_C_COMPILER=clang-14 ..

$(LIBYAJL_A): $(LIB_DIR)/yajl/build
	cd $< && make


# Jansson
LIBJANSSON := $(LIB_DIR)/jansson/build/lib/libjansson.so

LIB_JANSSON_LDLIBS := -ljansson -Wl,-rpath,$(ABSOLUTE_LIB_DIR)/jansson/build/lib
LIB_JANSSON_LDFLAGS := -L$(ABSOLUTE_LIB_DIR)/jansson/build/lib
LIB_JANSSON_CFLAGS := -I$(ABSOLUTE_LIB_DIR)/jansson/build/include

$(LIB_DIR)/jansson/build:
	mkdir -p $@
	grep -qxF 'SET(CMAKE_C_FLAGS "$${CMAKE_C_FLAGS} ${COV_FULL_FLAGS}")' $(LIB_DIR)/jansson/CMakeLists.txt || echo 'SET(CMAKE_C_FLAGS "$${CMAKE_C_FLAGS} ${COV_FULL_FLAGS}")' >> $(LIB_DIR)/jansson/CMakeLists.txt
	cd $@ && cmake -DJANSSON_BUILD_DOCS=OFF -DJANSSON_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14 ..

$(LIBJANSSON): $(LIB_DIR)/jansson/build
	cd $< && make

# JsonCpp
LIBJSONCPP := $(LIB_DIR)/jsoncpp/build/lib/libjsoncpp.so
LIB_JSONCPP_LDLIBS := -ljsoncpp -Wl,-rpath,$(ABSOLUTE_LIB_DIR)/jsoncpp/build/lib
LIB_JSONCPP_LDFLAGS := -L$(ABSOLUTE_LIB_DIR)/jsoncpp/build/lib
LIB_JSONCPP_CFLAGS := -I$(ABSOLUTE_LIB_DIR)/jsoncpp/include/json


$(LIB_DIR)/jsoncpp/build:
	mkdir -p $@
	grep -qxF 'add_compile_options(${COV_FULL_FLAGS})' $(LIB_DIR)/jsoncpp/CMakeLists.txt || sed -i '1s/^/add_compile_options(${COV_FULL_FLAGS})\nlink_libraries(-fsanitize=address)\n/' $(LIB_DIR)/jsoncpp/CMakeLists.txt
	cd $@ && cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_OBJECT_LIBS=OFF -DJSONCPP_WITH_CMAKE_PACKAGE=OFF -DJSONCPP_WITH_TESTS=OFF -DCMAKE_CXX_COMPILER=clang++-14 ..

$(LIBJSONCPP): $(LIB_DIR)/jsoncpp/build
	cd $< && make

# Boost
BOOST_HEADERS:= $(LIB_DIR)/boost/boost/json.hpp
LIB_BOOST_CFLAGS := -I$(LIB_DIR)/boost

$(BOOST_HEADERS):
	cd $(LIB_DIR)/boost && ./bootstrap.sh && ./b2 headers

# Poco
# the build folder cannot be named build because of the existing folder with that name
LIBPOCO := $(LIB_DIR)/poco/cmake-build/lib/libPocoJSON.so
LIB_POCO_LDLIBS := -lPocoJSON -lPocoFoundation -Wl,-rpath,$(ABSOLUTE_LIB_DIR)/poco/cmake-build/lib
LIB_POCO_LDFLAGS := -L$(ABSOLUTE_LIB_DIR)/poco/cmake-build/lib
LIB_POCO_CFLAGS := -I$(ABSOLUTE_LIB_DIR)/poco/JSON/include -I$(ABSOLUTE_LIB_DIR)/poco/Foundation/include

$(LIB_DIR)/poco/cmake-build:
	mkdir -p $@
#	grep -qxF 'add_compile_options(${COV_FULL_FLAGS})' $(LIB_DIR)/poco/Foundation/CMakeLists.txt || sed -i '1s/^/add_compile_options(${COV_FULL_FLAGS})\nlink_libraries(-fsanitize=address)\n/' $(LIB_DIR)/poco/Foundation/CMakeLists.txt
	grep -qxF 'add_compile_options(${COV_FULL_FLAGS})' $(LIB_DIR)/poco/JSON/CMakeLists.txt || sed -i '1s/^/add_compile_options(${COV_FULL_FLAGS})\nlink_libraries(-fsanitize=address)\n/' $(LIB_DIR)/poco/JSON/CMakeLists.txt
	cd $@ && cmake -DENABLE_ENCODINGS=OFF -DENABLE_XML=OFF -DENABLE_MONGODB=OFF \
	-DENABLE_DATA_SQLITE=OFF -DENABLE_REDIS=OFF -DENABLE_PROMETHEUS=OFF -DENABLE_UTIL=OFF -DENABLE_NET=OFF \
	-DENABLE_ZIP=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF \
	-DENABLE_ACTIVERECORD=OFF -DENABLE_ACTIVERECORD_COMPILER=OFF -DENABLE_NETSSL=OFF \
	-DENABLE_CRYPTO=OFF -DENABLE_JWT=OFF -DENABLE_ACTIVERECORD=OFF -DENABLE_ACTIVERECORD_COMPILER=OFF \
	-DCMAKE_CXX_COMPILER=clang++-14 -DCMAKE_C_COMPILER=clang-14 ..

$(LIBPOCO): $(LIB_DIR)/poco/cmake-build
	cd $< && cmake --build . --config Debug

# json-c
LIBJSONC := $(LIB_DIR)/json-c/build/libjson-c.so
LIB_JSONC_LDLIBS := -ljson-c -Wl,-rpath,$(ABSOLUTE_LIB_DIR)/json-c/build
LIB_JSONC_LDFLAGS := -L$(ABSOLUTE_LIB_DIR)/json-c/build
LIB_JSONC_CFLAGS := -I$(ABSOLUTE_LIB_DIR)/json-c -I$(ABSOLUTE_LIB_DIR)/json-c/build

$(LIBJSONC): 
	mkdir -p $(dir $@)
	cd $(dir $@) && \
		cmake -DBUILD_TESTING=OFF -DCMAKE_C_COMPILER=clang-14 -DBUILD_STATIC_LIBS=OFF .. && \
		make