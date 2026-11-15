# bouncebackhaven — raylib C++ build (macOS, g++)
#
#   make        build ./build/bouncebackhaven
#   make run    build and run
#   make clean  remove build artifacts
#
# raylib lives in ~/libs/raylib. Override with `make RAYLIB=/path/to/raylib`.

# `g++` on macOS is Apple clang. Override with `make CXX=clang++`.
CXX      := g++
NAME     ?= bouncebackhaven
SRC_DIR  := src
BUILD    := build
RAYLIB   ?= $(HOME)/libs/raylib

CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I$(RAYLIB)/include
LDFLAGS  := $(RAYLIB)/lib/libraylib.a \
            -framework OpenGL -framework Cocoa -framework IOKit \
            -framework CoreVideo -framework CoreAudio -framework QuartzCore

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.o,$(SRCS))

all: $(BUILD)/$(NAME)

$(BUILD)/$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD)/%.o: $(SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

run: all
	./$(BUILD)/$(NAME)

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
