CC=g++
CCFLAGS=-std=c++11
SRCS=$(wildcard *.cpp)
OUT=$(SRCS:.cpp=)
OUTERLIBS=../src/libcoroutine.so -lpthread -ldl

.PHONY:all
all:
	@for out in $(OUT); do \
		$(CC) $(CCFLAGS) $$out.cpp $(OUTERLIBS) -o $$out;\
		done
