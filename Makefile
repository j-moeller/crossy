CC := clang
CXX := clang++

SRC_DIR := src
BUILD_DIR := build

#
#
#

ifdef IN_DOCKER_CONTAINER
all: build;
else
all: docker;
endif

LIBFUZZER_SRC := ext/llvm-project/compiler-rt/lib/fuzzer
LIBFUZZER_LIB_BUILD := $(BUILD_DIR)/libfuzzer

CPP_FILES := $(shell find $(LIBFUZZER_SRC) -maxdepth 1 -name '*.cpp')
LIBFUZZER_OBJ_FILES := $(patsubst $(LIBFUZZER_SRC)/%.cpp,$(LIBFUZZER_LIB_BUILD)/%.o,$(CPP_FILES))

$(LIBFUZZER_LIB_BUILD)/%.o: $(LIBFUZZER_SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -g -c -O2 -I$(LIBFUZZER_SRC)/nezha/ -DDIFFERENTIAL_TESTING $< -o $@ 

$(LIBFUZZER_LIB_BUILD)/libfuzzer.a: $(LIBFUZZER_OBJ_FILES)
	ar rv $@ $^

#
#
#

include $(SRC_DIR)/Makefile

.PHONY: run docker shared_libs java_libs

build: $(BUILD_DIR)/poc java java_libs shared_libs

shared_libs:
	cd experiments/json/shared-objects && make
# cd experiments/wasm/shared-objects && make

clean: clean-java
	rm $(BUILD_DIR)/libfuzzer/*

run: $(BUILD_DIR)/poc shared_libs
	LSAN_OPTIONS="suppressions=/app/suppr.txt" \
	./$(BUILD_DIR)/poc \
	./configs/json/so-* \
	./configs/json/gold-parser/* \
	-- -seed=42 -detect_leaks=0 -max_total_time=28800

java_libs:
	cd experiments/json/java/java-adapter && make

docker:
	./scripts/dev/start_or_attach_container.sh