CC=/usr/bin/gcc
CFLAGS=-m32 -shared -fPIC -Wall -O2 -Wno-deprecated
CXX=/usr/bin/g++
CXXFLAGS=-m32 -shared -fPIC -Wall -O2 -Wno-deprecated
LD=/usr/bin/ld

OBJ = adom-sage.o library.o states.o command.o options.o io.o msg_handlers.o \
	spells.o loader.o starsign.o autosave.o shm.o item_list.o roller.o
TARGET = adom-sage.so

all: $(TARGET) adom-sage

clean:
	/bin/rm -f $(TARGET) $(OBJ) adom-sage config.h

adom-sage: sage-frontend.cc
	$(CXX) -m32 -o adom-sage sage-frontend.cc -lncurses -g

# HACK: Gratuitously link ncurses to adom-sage, then run ldd on adom-sage to
# find its path.  Adom-sage will try running ldd on adom, so this code is
# hopefully redundant.
# Newer Ubuntu refuses to link adom-sage with libncurses.
# In this version, assume that ncurses is in the same directory as libc.
config.h: adom-sage
	ldd adom-sage | perl -ne '$$_ = (/^\s+libc.*=>\s+(\S+)\//) and $$path = $$1 and print "#define LIBC \"$$path/libc.so.6\"\n#define LIBNCURSES \"$$path/libncurses.so.5\"\n"' >config.h
	@grep LIBNCURSES config.h > /dev/null || \
		(echo Unable to find libncurses && exit 1)
	@grep LIBC config.h > /dev/null || \
		(echo Unable to find libc && exit 1)

adom-sage.so: $(OBJ)
	$(CXX) -m32 -shared -o adom-sage.so $(OBJ) -ldl -O2
#	$(CXX) -Wl,-Bshareable -shared -fPIC -o adom-sage.so $(OBJ) -ldl

library.o : library.cc config.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o : %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

watcher.so: watcher.c
	$(CC) $(CFLAGS) -c -o watcher.o watcher.c
	$(LD) -Bshareable -o watcher.so watcher.o -ldl

