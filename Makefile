PROGNAME=synth
VERSION=0.1
DISTNAME=$(PROGNAME)-$(VERSION)
CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -O2
CC=gcc
LIBS=-lm -ljack
DEBUGFLAGS=-g -DDEBUG
NODEBUGFLAGS=-DNDEBUG
MAINDEPS=synth.o jackclient.o
PREFIX=/usr

ifndef DEBUG
	DEBUG=$(NODEBUGFLAGS)
endif

all: $(PROGNAME)

%.o : %.c
	$(CC) $(CFLAGS) $(DEBUG) -c $^

$(PROGNAME): main.c $(MAINDEPS)
	$(CC) $(CFLAGS) $(DEBUG) -o $@ main.c $(MAINDEPS) $(LIBS)

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
	rm -rf *.o $(PROGNAME) *.tar.gz *.pkg.tar.xz $(DISTNAME)/ *~

distclean: clean
