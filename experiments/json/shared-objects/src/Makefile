TARGETS_DIR := targets
ADAPTER_DIR := adapter
INTERFACE_DIR := ../../../src/interface

SRC_TARGETS_DIR := $(SRC_DIR)/$(TARGETS_DIR)
SRC_ADAPTER_DIR := $(SRC_DIR)/$(ADAPTER_DIR)

BUILD_TARGETS_DIR := $(BUILD_DIR)/$(TARGETS_DIR)
BUILD_ADAPTER_DIR := $(BUILD_DIR)/$(ADAPTER_DIR)

SHARED_LIBS := $(foreach l,$(ALL_LIBS),$(BUILD_TARGETS_DIR)/$(l).so)

include $(SRC_ADAPTER_DIR)/Makefile

RUST_DIR := rust
include $(RUST_DIR)/Makefile

include $(INTERFACE_DIR)/Makefile

#
# Shared object file
#

# generic target using compiled library
$(BUILD_TARGETS_DIR)/%.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/%_adapter.o $(BUILD_LIBS_DIR)/%.o $(BUILD_ADAPTER_DIR)/print-buffer.o
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) -o $@ $^

# generic target using header-only library
$(BUILD_TARGETS_DIR)/%.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/%_adapter.o $(BUILD_ADAPTER_DIR)/print-buffer.o
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) -o $@ $^

# generic rust target
# -l$(basename $(notdir $@))_adapter -L$(ABSOLUTE_RUST_DIR)/$(basename $(notdir $@))_adapter/target/debug -Wl,-rpath=$(ABSOLUTE_RUST_DIR)/$(basename $(notdir $@))_adapter/target/debug
$(BUILD_TARGETS_DIR)/%.so: $(BUILD_DIR)/generic_target.o $(BUILD_DIR)/rust_target.o $(RUST_DIR)/%_adapter/target/debug/libadapter.a
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS)  -o $@ $^


# boost
# ccan
# cjson
# frozen

# jansson
$(BUILD_TARGETS_DIR)/jansson.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/jansson_adapter.o $(LIBJANSSON)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(LIB_JANSSON_LDLIBS) $(LIB_JANSSON_LDFLAGS) -o $@ $^

# jsmn
# json

# json-c
$(BUILD_TARGETS_DIR)/json-c.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/json-c_adapter.o $(LIBJSONC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(LIB_JSONC_LDLIBS) $(LIB_JSONC_LDFLAGS) -o $@ $^

# json-parser, json-builder
$(BUILD_TARGETS_DIR)/json-parser.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/json-parser_adapter.o $(BUILD_LIBS_DIR)/json-parser.o $(BUILD_LIBS_DIR)/json-builder.o
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) -o $@ $^

# jsoncpp
$(BUILD_TARGETS_DIR)/jsoncpp.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/jsoncpp_adapter.o $(LIBJSONCPP)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(LIB_JSONCPP_LDLIBS) $(LIB_JSONCPP_LDFLAGS) -o $@ $^

# jsonh
# libjson

# poco
$(BUILD_TARGETS_DIR)/poco.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/poco_adapter.o $(LIBPOCO)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(LIB_POCO_LDLIBS) $(LIB_POCO_LDFLAGS) -o $@ $^

# rapidjson

# spidermonkey
# SPIDERMONKEY_BUILD
$(BUILD_TARGETS_DIR)/spidermonkey.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/spidermonkey_adapter.o $(BUILD_SPIDERMONKEY_DIR)/js/src/build/libmozjs-114a1.so
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -Wl,-rpath=/app/experiments/json/shared-objects/$(BUILD_SPIDERMONKEY_DIR)/js/src/build/ -L$(BUILD_SPIDERMONKEY_DIR)/js/src/build/ -lmozjs-114a1 $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/spidermonkey_adapter.o -o $@

#$(BUILD_TARGETS_DIR)/spidermonkey.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/spidermonkey_adapter.o $(BUILD_SPIDERMONKEY_DIR)/js/src/build/libjs_static.a
#	@mkdir -p $(dir $@)
#	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $^ -o $@

# tiny-json

# yajl
$(BUILD_TARGETS_DIR)/yajl.so: $(BUILD_DIR)/generic_target.o $(BUILD_ADAPTER_DIR)/yajl_adapter.o $(LIBYAJL_A)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_LIB) $(COV_FULL_FLAGS) $(LIB_YAJL_LDLIBS) $(LIB_YAJL_LDFLAGS) -o $@ $^

