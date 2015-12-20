CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc -DUSE_LIBV4L2 -g -I /usr/include
LD = $(CROSS_COMPILE)ld
LDLIBS = -O -lrt -lpthread
LDFLAGS =
INC = -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = test.o

all: test

All: clean test

robot: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o test $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o test
