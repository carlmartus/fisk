BIN=bin
EMCC=emcc
EXE=$(BIN)/fisk
HDIR=html
HTML=$(HDIR)/fisk.js
CFLAGS=-Wall -O2 -g
LDFLAGS=-lSDL -lGL -lGLEW -lSDL_image -lSDL_mixer -lm
HTML_ADD=index.html art/logo.png

SRC=$(wildcard *.c)
ODIR=obj
OBJS=$(SRC:%.c=$(ODIR)/%.o)

.PHONY: all html clean re
.SECONDARY:

all: $(EXE)

$(EXE): $(OBJS) $(BIN)/.d
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(ODIR)/%.o: %.c $(ODIR)/.d fisk.h
	$(CC) -o $@ -c $< $(CFLAGS)

%/.d:
	mkdir `dirname $@`
	touch $@

html: $(HTML)

$(HTML): $(SRC) $(HDIR)/.d $(HTML_ADD)
	$(EMCC) -o $@ -Wall -O2 $(SRC) --preload-file res
	cp $(HTML_ADD) $(HDIR)

clean:
	$(RM) $(EXE) -r $(ODIR) $(BIN) $(HDIR)

re: clean all

