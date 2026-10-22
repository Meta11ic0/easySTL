# 内存池只使用自己的 include/。不依赖 easySTL。

CXX      ?= g++
CXXFLAGS ?= -std=c++14 -fPIC -Wall -Wextra -I include

.PHONY: all clean install

all: libeasypool.so

libeasypool.so: allocator.cpp include/allocator.h
	$(CXX) $(CXXFLAGS) -shared allocator.cpp -o $@

PREFIX ?= /usr/local

install: libeasypool.so
	mkdir -p $(PREFIX)/include/easypool $(PREFIX)/lib
	cp include/allocator.h $(PREFIX)/include/easypool/
	cp libeasypool.so $(PREFIX)/lib/

clean:
	rm -f libeasypool.so
