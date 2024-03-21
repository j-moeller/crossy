JAVA_DIR := $(SRC_DIR)/java
include $(JAVA_DIR)/Makefile

.PHONY: clean-src

#
# Compiled:
#

C_INCLUDES := -I./src/ -I./ext/llvm-project/compiler-rt/lib/fuzzer
C_FLAGS := -c -g -O2 -Wall -Werror -Wpedantic -fsanitize=address

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(C_INCLUDES) $^ -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/runner.h $(SRC_DIR)/util.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/cli.o: $(SRC_DIR)/cli.cpp $(SRC_DIR)/cli.h $(SRC_DIR)/runner.h $(SRC_DIR)/util.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/coverage.o: $(SRC_DIR)/coverage.cpp $(SRC_DIR)/coverage.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/input-reader.o: $(SRC_DIR)/input-reader.cpp $(SRC_DIR)/input-reader.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/runner.o: $(SRC_DIR)/runner.cpp $(SRC_DIR)/runner.h $(SRC_DIR)/process.h $(SRC_DIR)/shared-library.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/util.o: $(SRC_DIR)/util.cpp $(SRC_DIR)/util.h $(SRC_DIR)/runner.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

$(BUILD_DIR)/targets.o: $(SRC_DIR)/targets.cpp $(SRC_DIR)/targets.h
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

#
# Linked:
#

$(BUILD_DIR)/poc: $(BUILD_DIR)/process.o $(BUILD_DIR)/main.o $(BUILD_DIR)/coverage.o $(BUILD_DIR)/runner.o $(BUILD_DIR)/util.o $(LIBFUZZER_LIB_BUILD)/libfuzzer.a $(BUILD_DIR)/targets.o $(BUILD_DIR)/cli.o $(BUILD_DIR)/input-reader.o
	mkdir -p $(dir $@)
	make $(JAVA_BUILD_DIR)/jvm_launcher
	$(CXX) -g -O2 -pthread -fsanitize=address $^ $(JVM_LINK) -o $@

clean-src:
	rm -f $(BUILD_DIR)/*.o
	rm -f $(BUILD_DIR)/poc $(BUILD_DIR)/minimizer $(JAVA_BUILD_DIR)/jvm_launcher