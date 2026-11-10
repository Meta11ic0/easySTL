CXX := g++
CPPFLAGS := -I ../easystl/include
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -MMD -MP
NAME := storage_lifetime_demo
TARGET := ../build/$(NAME)

.PHONY: all demo run

all: $(TARGET)

demo: $(TARGET)
	$(TARGET)

run: demo

$(TARGET): $(NAME).cpp Makefile | ../build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

../build:
	mkdir -p ../build

-include $(wildcard ../build/$(NAME).d)
