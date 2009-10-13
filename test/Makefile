CC := gcc

objects := test.o

sources := $(objects:.o=.c)


# from Lauri Leukkunen's build system
ifdef V
  Q = 
  P = @printf "" # <- space before hash is important!!!
else
  P = @printf "[%s] $@\n" # <- space before hash is important!!!
  Q = @
endif

PURPLE_CFLAGS := $(shell pkg-config --cflags purple)
PURPLE_LIBS := $(shell pkg-config --libs purple)

CFLAGS := $(CFLAGS) $(PURPLE_CFLAGS) $(GOBJECT_CFLAGS)
LIBS := $(LIBS) $(PURPLE_LIBS)

EXTRA_WARNINGS := -Wall -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
	-Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
	-Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal -ansi -std=c99

SIMPLE_WARNINGS := -Wextra -ansi -std=c99 -Wno-unused-parameter

OTHER_WARNINGS := -D_FORTIFY_SOURCE=2 -fstack-protector -g3 -Wdisabled-optimization \
	-Wendif-labels -Wformat=2 -Wstack-protector -Wswitch

CFLAGS += $(EXTRA_WARNINGS) $(OTHER_WARNINGS)


all: $(objects)

%.o:: %.c
	$(P)CC
	$(Q)$(CC) $(CFLAGS) $(LIBS) -MMD -o $@ $<
#	gcc `pkg-config --libs glib-2.0` `pkg-config --cflags glib-2.0` -o test test.c

clean:
	rm -f $(objects) $(objects:.o=.d)
