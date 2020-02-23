CC=g++
CCFLAGS=-std=c++11
SRCS=$(wildcard *.cpp)
OUT=$(SRCS:.cpp=)
OUTERLIBS= -lcoroutine -lpthread -ldl

.PHONY:all clean
all:
	@for out in $(OUT); do \
		$(CC) $(CCFLAGS) $$out.cpp $(OUTERLIBS) -o $$out;\
		done

clean:
	rm $(OUT)
