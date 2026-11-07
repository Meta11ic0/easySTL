CXX := g++
CPPFLAGS := -I easystl/include
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -MMD -MP
NAME ?= vector_reserve_demo
TARGET := build/$(NAME)

.PHONY: all run

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): test/$(NAME).cpp Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

build:
	mkdir build

-include $(wildcard build/*.d)
