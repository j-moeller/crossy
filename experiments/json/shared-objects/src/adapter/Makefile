$(BUILD_ADAPTER_DIR)/%.o: $(SRC_ADAPTER_DIR)/%.c $(SRC_ADAPTER_DIR)/*.h $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_O) -I $(LIB_DIR) $(COV_FULL_FLAGS) -c $< -o $@

$(BUILD_ADAPTER_DIR)/%.o: $(SRC_ADAPTER_DIR)/%.cpp $(SRC_ADAPTER_DIR)/*.h $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) -I $(LIB_DIR) $(COV_FULL_FLAGS) -c $< -o $@

# nlohmann json adapter, normally built by generic target, this is for testing purposes
$(BUILD_ADAPTER_DIR)/json_adapter.o: $(SRC_ADAPTER_DIR)/json_adapter.cpp $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) -I$(LIB_DIR)/json/single_include $(COV_FULL_FLAGS) -c $< -o $@

# yajl adapter
$(BUILD_ADAPTER_DIR)/yajl_adapter.o: $(SRC_ADAPTER_DIR)/yajl_adapter.c $(INTERFACE_DIR)/common.h $(LIBYAJL_A)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_O) $(LIB_YAJL_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# json-parser adapter
$(BUILD_ADAPTER_DIR)/json-parser_adapter.o: $(SRC_ADAPTER_DIR)/json-parser_adapter.c $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_O) -I$(LIB_DIR)/json-parser -I$(LIB_DIR)/json-builder $(COV_FULL_FLAGS) -c $< -o $@

# jansson adapter
$(BUILD_ADAPTER_DIR)/jansson_adapter.o: $(SRC_ADAPTER_DIR)/jansson_adapter.c $(INTERFACE_DIR)/common.h $(LIBJANSSON)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_O) $(LIB_JANSSON_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# JsonCpp adapter
$(BUILD_ADAPTER_DIR)/jsoncpp_adapter.o: $(SRC_ADAPTER_DIR)/jsoncpp_adapter.cpp $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) $(LIB_JSONCPP_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# Boost.JSON adapter include Boost header files
$(BUILD_ADAPTER_DIR)/boost_adapter.o: $(SRC_ADAPTER_DIR)/boost_adapter.cpp $(INTERFACE_DIR)/common.h $(BOOST_HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) $(LIB_BOOST_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# Poco adapter: cannot use -Werror
$(BUILD_ADAPTER_DIR)/poco_adapter.o: $(SRC_ADAPTER_DIR)/poco_adapter.cpp $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_NO_WERROR) $(CFLAGS_SHARED_O) $(CXX_FLAGS) $(LIB_POCO_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# json-c
$(BUILD_ADAPTER_DIR)/json-c_adapter.o: $(SRC_ADAPTER_DIR)/json-c_adapter.c $(INTERFACE_DIR)/common.h $(LIBJSONC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_SHARED_O) $(LIB_JSONC_CFLAGS) $(COV_FULL_FLAGS) -c $< -o $@

# spidermonkey
$(BUILD_ADAPTER_DIR)/spidermonkey_adapter.o: $(SRC_ADAPTER_DIR)/spidermonkey_adapter.cpp $(SRC_ADAPTER_DIR)/*.h $(INTERFACE_DIR)/common.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_NO_WERROR) $(SPIDERMONKEY_DEBUG_FLAG) $(CFLAGS_SHARED_O) $(CXX_17_FLAGS) -I $(LIB_DIR) -I$(BUILD_SPIDERMONKEY_DIR)/dist/include/ -I$(LIB_DIR)/spidermonkey/js/src $(COV_FULL_FLAGS) -c $< -o $@
