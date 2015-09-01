CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc -DCONFIG_LIBNL30 -g
LD = $(CROSS_COMPILE)ld
LDLIBS = -O -lrt -lpthread -ljpeg -liw
LDFLAGS =
INC = -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = test.o

all: robot

All: clean test

robot: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o test $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o test
