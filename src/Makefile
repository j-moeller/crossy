C_INCLUDES := -I./src/ -I./libfuzzer
C_FLAGS := -c -g -O2 -Wall -Werror -Wpedantic -fsanitize=address

build/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(C_FLAGS) $(C_INCLUDES) $< -o $@

build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(C_INCLUDES) $^ -o $@