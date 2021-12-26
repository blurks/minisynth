PROGNAME=synth
VERSION=0.1
DISTNAME=$(PROGNAME)-$(VERSION)
CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -O2 -DPROGNAME='"$(PROGNAME)"'
CC=gcc
LIBS=-lm -ljack
DEBUGFLAGS=-g -DDEBUG
NODEBUGFLAGS=-DNDEBUG
MAINDEPS=synth.o jackclient.o
PREFIX=/usr

ifndef DEBUG
	DEBUG=$(NODEBUGFLAGS)
endif

all: $(PROGNAME) gui

%.o : %.c
	$(CC) $(CFLAGS) $(DEBUG) -c $^

$(PROGNAME): main.c $(MAINDEPS)
	$(CC) $(CFLAGS) $(DEBUG) -o $@-cli main.c $(MAINDEPS) $(LIBS)

gui: gui.c $(MAINDEPS)
	$(CC) $(CFLAGS) $(DEBUG) -o $(PROGNAME) gui.c $(MAINDEPS) $(LIBS) -lforms

debug:
	export DEBUG="$(DEBUGFLAGS)" && $(MAKE)

install: $(PROGNAME)
	install -sD $(PROGNAME) "$(DESTDIR)$(PREFIX)/bin/$(PROGNAME)"

uninstall:
	rm $(DESTDIR)/$(PREFIX)/bin/$(PROGNAME)

dist: $(wildcard *.c) $(wildcard *.h) Makefile PKGBUILD
	mkdir $(DISTNAME)
	cp $^ $(DISTNAME)
	tar -czf $(DISTNAME).tar.gz $(DISTNAME)
	updpkgsums
	makepkg -sc
	rm -rf $(DISTNAME)

clean:
	rm -rf *.o $(PROGNAME) $(PROGNAME)-cli *.tar.gz *.pkg.tar.xz $(DISTNAME)/ *~

distclean: clean
