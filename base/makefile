CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
CFLAGS = -O -g2 -DDEBUG
LDLIBS = -lrt -lpthread -lglib-2.0 -lcairo -lgthread-2.0 -lgdk-x11-2.0 -lgtk-x11-2.0
LDFLAGS =
INC = -I ../pub/ -I /usr/include/gtk-2.0 -I /usr/include/glib-2.0 -I /usr/lib/i386-linux-gnu/glib-2.0/include -I /usr/include/cairo -I /usr/include/pango-1.0 -I /usr/lib/i386-linux-gnu/gtk-2.0/include -I /usr/inclide/atk-1.0 -I /usr/include/gdk-pixbuf-2.0 -I /usr/include/atk-1.0
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = base.o watcher.o map.o

all: base

All: clean base

base: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o base $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o base
