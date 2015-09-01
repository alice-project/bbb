CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2
LDLIBS = -O -lrt -L./ -lpthread -ljpeg -lGL -lGLU -lglut -lm
LDFLAGS =
INC = -I ../../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = flight.o glm.o sgi.o

all: flight

All: clean flight

flight: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o flight $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o flight
