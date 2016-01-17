CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
CFLAGS = -O -g2 -DDEBUG -rdynamic -pthread
LDLIBS = `pkg-config --libs gtk+-3.0 --libs cairo ` -lm -lxml2
LDFLAGS = 
INC = `pkg-config --cflags gtk+-3.0 cairo`
INC += -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = baseapp.o odometer.o mcast.o watcher.o cmd_util.o video.o

all: base

All: clean base

base: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o base $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o base
