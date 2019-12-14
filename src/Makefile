CC=g++
AR=ar
OUT=libcoroutine.a
CXXFLAGS=-std=c++11

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

$(OUT):$(OBJS)
	$(AR) rcs $(OUT) $(OBJS)

.c.o:
	$(CC) $< 

.PHONY:clean 
clean:
	rm *.o $(OUT)
