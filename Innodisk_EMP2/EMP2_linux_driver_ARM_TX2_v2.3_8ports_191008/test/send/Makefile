Q               := @
CC              := gcc
SRCS            := $(wildcard *.c)
OBJS            := $(SRCS:.c=.o)
TARGET          := emp2_send
CFLAGS          := -std=gnu99 -Wall -I./include
LIBS            := -lpthread
LDFLAGS         := $(LIBS)

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(Q)echo "  $(COLOR_G)Building '$@' VER=$(AP_VER)... $(COLOR_W)"
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(Q)rm -f .depend *~ *.bak *.res *.o
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f $(TARGET)

.depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
	include .depend
endif