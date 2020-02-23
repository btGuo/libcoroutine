CC=g++
AR=ar
OUT=libcoroutine.so
CXXFLAGS=-std=c++11 -fPIC -Wall -g

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

$(OUT):$(OBJS)
	$(CC) -g -shared -o $(OUT) $(OBJS)

.c.o:
	$(CC) $< 

.PHONY:clean 
clean:
	rm *.o $(OUT)
