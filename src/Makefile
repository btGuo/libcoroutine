CC=g++
CCFLAGS = -std=c++11 
AR=ar
OUT=libcoroutine.a
SRCS=scheduler.c processor.c 
OBJS=scheduler.o processor.o

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

$(OUT):$(OBJS)
	$(AR) rcs $(OUT) $(OBJS)

clean:
	rm *.o $(OUT)
