$(BUILD_DIR)/generic_target.o: $(INTERFACE_DIR)/generic_target.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) $(COV_FULL_FLAGS) -DHANDLE_EXCEPTIONS -c -o $@ $^

$(BUILD_DIR)/rust_target.o: $(INTERFACE_DIR)/rust_target.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CFLAGS_SHARED_O) $(CXX_FLAGS) $(COV_FULL_FLAGS) -c -o $@ $^