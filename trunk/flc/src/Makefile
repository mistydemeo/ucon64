.PHONY: all clean distclean install flc uninstall

CFLAGS=-I. -Wall -W -O3
LDFLAGS=-s #-pg

include config.mak

TARGET=flc
OBJECTS=flc.o flc_misc.o filter/id3.o filter/met.o filter/txt.o \
	misc/file.o misc/filter.o misc/getopt.o misc/getopt2.o \
	misc/misc.o misc/property.o misc/rmdir2.o misc/string.o

all: $(TARGET)

clean:
	rm -f $(OBJECTS) DEADJOE $(TARGET) *~ */*~ */*/*~ *.core 

distclean: clean
	rm -f Makefile config.log config.status config.cache config.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

install:
	cp $(TARGET) $(prefix)/bin

uninstall:
	rm -f $(prefix)/bin/$(TARGET)

# Dependencies

# Most source files include these
FLC_STD_H=flc.h flc_misc.h flc_defines.h config.h

flc.o: flc_misc.h misc/getopt.h $(FLC_STD_H)
flc_misc.o: $(FLC_STD_H)
filter/txt.o: filter/txt.h $(FLC_STD_H)
filter/met.o: filter/met.h $(FLC_STD_H)
filter/id3.o: filter/id3.h $(FLC_STD_H)
misc/cache.o: misc/cache.h
misc/file.o: misc/misc.h
misc/getopt.o: misc/getopt.h
misc/getopt2.o: misc/getopt2.h
misc/misc.o: misc/misc.h
misc/property.o: misc/property.h
misc/rmdir2.o: misc/rmdir2.h
misc/string.o: misc/string.h
misc/filter.o: misc/filter.h
