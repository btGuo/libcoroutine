CC=g++
CXXFLAGS=-std=c++11
SRCS=$(wildcard *.cpp)
OUT=$(SRCS:.cpp=)
OUTERLIBS= -lcoroutine -lpthread -lgtest -lgtest_main

.PHONY:test clean
test:
	@for out in $(OUT); do \
		$(CC) $(CXXFLAGS) $$out.cpp $(OUTERLIBS) -o $$out;\
		done

clean:
	rm $(OUT)
