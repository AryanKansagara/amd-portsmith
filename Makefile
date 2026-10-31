CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test demo clean
all: build/portsmith
build:
	mkdir -p build
build/portsmith: main.cpp portsmith.hpp | build
	$(CXX) $(CXXFLAGS) main.cpp -o $@
build/tests: tests.cpp portsmith.hpp | build
	$(CXX) $(CXXFLAGS) tests.cpp -o $@
test: build/tests build/portsmith
	./build/tests
	./build/portsmith demo > /dev/null
demo: build/portsmith
	./build/portsmith demo
clean:
	rm -rf build
