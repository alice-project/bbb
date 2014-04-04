CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
CFLAGS = -O -g2 -DDEBUG
LDLIBS = -lrt -lpthread 
LDFLAGS =
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = base.o

all: base

All: clean base

base: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o base $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o base
