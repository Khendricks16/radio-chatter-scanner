# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)

# Specify Compiler
CXX := g++
CC := gcc

# Specify important directories
SRC_DIR := ./src
INC_DIRS := ./include /usr/include/onnxruntime
BUILD_DIR := ./build

# Target Executable
TARGET_EXEC := rcs

# Set compiler flags
CXXFLAGS := -g
CFLAGS := -g
INC_FLAGS := $(addprefix -I, $(INC_DIRS))
LDFLAGS := -lonnxruntime -lsqlite3 -lrtlsdr

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIR) -name '*.cc' -or -name '*.c')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(INC_FLAGS) $(CXXFLAGS) -c $< -o $@


# Clean up what was compiled
clean:
	rm -rf $(BUILD_DIR)
