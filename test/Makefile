# 命令行 NAME=程序名 可以覆盖默认值，对应同名的 cpp 和可执行文件。
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -g -MMD -MP
NAME := storage_lifetime
TARGET := ../build/$(NAME)

# all 只构建，run 构建后运行；伪目标不依赖是否有同名文件。
.PHONY: all run
all: $(TARGET)

run: $(TARGET)
	$(TARGET)

# $< 是第一个依赖，即选中的 cpp；$@ 是本条规则的目标文件。
# | 后面的目录必须先存在，但目录时间改变不会导致程序重新编译。
$(TARGET): $(NAME).cpp Makefile | ../build
	$(CXX) $(CXXFLAGS) $< -o $@

../build:
	mkdir -p $@

# -MMD -MP 生成头文件依赖；首次构建没有 .d 文件，-include 允许忽略它。
-include $(TARGET).d
