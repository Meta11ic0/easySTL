# 整个项目的入口。pool 仍可单独在自己的目录里构建。

PREFIX ?= /usr/local

.PHONY: all test run clean install

all:
	$(MAKE) -C pool

test run:
	$(MAKE) -C test run

clean:
	$(MAKE) -C pool clean
	$(MAKE) -C test clean

install:
	$(MAKE) -C pool install PREFIX=$(PREFIX)
	mkdir -p $(PREFIX)/include/easystl
	cp easySTL/*.h $(PREFIX)/include/easystl/
