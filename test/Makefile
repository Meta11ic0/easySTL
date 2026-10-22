# -I pool/include 必须在 -I easySTL 前面，<allocator.h> 才是内存池的头文件。

CXX      ?= g++
CXXFLAGS ?= -std=c++14 -Wall -Wextra -MMD -MP -I ../pool/include -I ../easySTL
LDFLAGS  ?= -L ../pool -Wl,-rpath,'$$ORIGIN/../pool' -leasypool

.PHONY: all run clean

all: stltest

../pool/libeasypool.so: FORCE
	$(MAKE) -C ../pool

FORCE:
.PHONY: FORCE

stltest: test.cpp ../pool/libeasypool.so
	$(CXX) $(CXXFLAGS) test.cpp $(LDFLAGS) -o $@

-include stltest.d

run: stltest
	./stltest

clean:
	rm -f stltest stltest.d
