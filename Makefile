BIN=bin
EXE=$(BIN)/fisk
CFLAGS=-Wall -O2
LDFLAGS=-lSDL -lGL -lGLEW -lSDL_image -lSDL_mixer -lm

SRC=$(wildcard *.c)
ODIR=obj
OBJS=$(SRC:%.c=$(ODIR)/%.o)

.PHONY: all clean re
.SECONDARY:

all: $(EXE)

$(EXE): $(OBJS) $(BIN)/.d
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(ODIR)/%.o: %.c $(ODIR)/.d fisk.h
	$(CC) -o $@ -c $< $(CFLAGS)

%/.d:
	mkdir `dirname $@`
	touch $@

clean:
	$(RM) $(EXE) -r $(ODIR) $(BIN)

re: clean all

