include ostools.mk
buildprefix ?= build
objprefix ?= obj

PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

FILES_SRC = $(wildcard *.c)
FILES_OBJ_NAME = $(patsubst %.c,%.o,$(FILES_SRC))
FILES_OBJ = $(addprefix $(objprefix)/,$(FILES_OBJ_NAME))

CFLAGS =-fPIC -Wall -Wextra -pedantic -std=c99 -I../libltfat/include
CXXFLAGS=-fPIC -Wall -Wextra -pedantic -std=c++11 -fno-exceptions -fno-rtti -I../libltfat/include
COMPTARGET ?= release
include comptarget.mk

ifdef USECPP
ifeq ($(USECPP),1)
	CC = $(CXX)
	CFLAGS = $(CXXFLAGS)
endif
endif

SO_LIB_NAME = libpv.so
SO_LIB = $(buildprefix)/$(SO_LIB_NAME)

RTPV_H = $(buildprefix)/pv.h

DEP = $(buildprefix) $(objprefix)

shared: $(SO_LIB)

$(SO_LIB): $(DEP) $(FILES_OBJ)
	$(CC) -shared -fPIC -o $@ $(FILES_OBJ) -Wl,--no-undefined -lltfatf -lm

$(objprefix)/%.o: %.c
	$(CC) $(CFLAGS) -DLTFAT_SINGLE -c $< -o $@

$(RTPV_H): $(DEP)
	gcc -E -P -DNOSYSTEMHEADERS -DLTFAT_SINGLE -I../libltfat/include -nostdinc pv.h -o $@ 
	sed -i '1 i #ifndef _RTPV_H' $@
	sed -i '1 a #define _RTPV_H' $@
	sed -i '2 a #include "ltfat.h"' $@
	sed -i '$$ a #endif' $@

munit: 
	$(MAKE) clean
	$(MAKE) $(SO_LIB) 
	$(MAKE) $(RTPV_H)


$(buildprefix):
	$(MKDIR) $(buildprefix)

$(objprefix):
	$(MKDIR) $(objprefix)

cleanobj:
	@$(RMDIR) $(objprefix)

clean: cleanobj
	@$(RMDIR) $(buildprefix)

.PNONY: clean cleanobj munit

install:
	install -d $(LIBDIR)
	install $(SO_LIB) $(LIBDIR)
	mkdir -p $(INCDIR)
	cp -r build/pv.h $(INCDIR)

uninstall:
	rm -f $(LIBDIR)/$(SO_LIB)
	rm -f $(INCDIR)/pv.h
