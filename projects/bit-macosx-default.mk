#
#   bit-macosx-default.mk -- Makefile to build Embedthis Bit for macosx
#

PRODUCT         := bit
VERSION         := 0.8.0
BUILD_NUMBER    := 0
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := macosx
CC              := /usr/bin/clang
LD              := /usr/bin/ld
CONFIG          := $(OS)-$(ARCH)-$(PROFILE)

BIT_ROOT_PREFIX := /
BIT_CFG_PREFIX  := $(BIT_ROOT_PREFIX)etc/bit
BIT_PRD_PREFIX  := $(BIT_ROOT_PREFIX)usr/lib/bit
BIT_VER_PREFIX  := $(BIT_ROOT_PREFIX)usr/lib/bit/0.8.0
BIT_BIN_PREFIX  := $(BIT_VER_PREFIX)/bin
BIT_INC_PREFIX  := $(BIT_VER_PREFIX)/inc
BIT_LOG_PREFIX  := $(BIT_ROOT_PREFIX)var/log/bit
BIT_SPL_PREFIX  := $(BIT_ROOT_PREFIX)var/spool/bit
BIT_SRC_PREFIX  := $(BIT_ROOT_PREFIX)usr/src/bit-0.8.0
BIT_WEB_PREFIX  := $(BIT_ROOT_PREFIX)var/www/bit-default
BIT_UBIN_PREFIX := $(BIT_ROOT_PREFIX)usr/local/bin
BIT_MAN_PREFIX  := $(BIT_ROOT_PREFIX)usr/local/share/man/man1

CFLAGS          += -w
DFLAGS          +=  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS        += -L$(CONFIG)/bin
LIBS            += -lpthread -lm -ldl

DEBUG           := debug
CFLAGS-debug    := -g
DFLAGS-debug    := -DBIT_DEBUG
LDFLAGS-debug   := -g
DFLAGS-release  := 
CFLAGS-release  := -O2
LDFLAGS-release := 
CFLAGS          += $(CFLAGS-$(DEBUG))
DFLAGS          += $(DFLAGS-$(DEBUG))
LDFLAGS         += $(LDFLAGS-$(DEBUG))

unexport CDPATH

all compile: prep \
        $(CONFIG)/bin/libest.dylib \
        $(CONFIG)/bin/ca.crt \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libmprssl.dylib \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/bin/libpcre.dylib \
        $(CONFIG)/bin/libhttp.dylib \
        $(CONFIG)/bin/http \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/ejs \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/bit.es \
        $(CONFIG)/bin/bit \
        $(CONFIG)/bin/bits

.PHONY: prep

prep:
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(BIT_PRD_PREFIX)" = "" ] ; then echo WARNING: BIT_PRD_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/bit-macosx-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/bit-macosx-default-bit.h >/dev/null ; then\
		echo cp projects/bit-macosx-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/bit-macosx-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
clean:
	rm -rf $(CONFIG)/bin/libest.dylib
	rm -rf $(CONFIG)/bin/ca.crt
	rm -rf $(CONFIG)/bin/libmpr.dylib
	rm -rf $(CONFIG)/bin/libmprssl.dylib
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/bin/libpcre.dylib
	rm -rf $(CONFIG)/bin/libhttp.dylib
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/bin/libejs.dylib
	rm -rf $(CONFIG)/bin/ejs
	rm -rf $(CONFIG)/bin/ejsc
	rm -rf $(CONFIG)/bin/ejs.mod
	rm -rf $(CONFIG)/obj/estLib.o
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/ejsLib.o
	rm -rf $(CONFIG)/obj/ejs.o
	rm -rf $(CONFIG)/obj/ejsc.o
	rm -rf $(CONFIG)/obj/removeFiles.o
	rm -rf $(CONFIG)/obj/bit.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/est.h: 
	rm -fr $(CONFIG)/inc/est.h
	cp -r src/deps/est/est.h $(CONFIG)/inc/est.h

$(CONFIG)/inc/bitos.h: 
	rm -fr $(CONFIG)/inc/bitos.h
	cp -r src/bitos.h $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/estLib.o -arch x86_64 $(DFLAGS) -I$(CONFIG)/inc src/deps/est/estLib.c

$(CONFIG)/bin/libest.dylib: \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/obj/estLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libest.dylib $(CONFIG)/obj/estLib.o $(LIBS)

$(CONFIG)/bin/ca.crt: src/deps/est/ca.crt
	rm -fr $(CONFIG)/bin/ca.crt
	cp -r src/deps/est/ca.crt $(CONFIG)/bin/ca.crt

$(CONFIG)/inc/mpr.h: 
	rm -fr $(CONFIG)/inc/mpr.h
	cp -r src/deps/mpr/mpr.h $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.dylib: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/obj/mprLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/est.h
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.dylib: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libest.dylib \
    $(CONFIG)/obj/mprSsl.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libmprssl.dylib $(CONFIG)/obj/mprSsl.o -lest -lmpr $(LIBS)

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o -lmpr $(LIBS)

$(CONFIG)/inc/pcre.h: 
	rm -fr $(CONFIG)/inc/pcre.h
	cp -r src/deps/pcre/pcre.h $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.dylib: \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/obj/pcre.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	rm -fr $(CONFIG)/inc/http.h
	cp -r src/deps/http/http.h $(CONFIG)/inc/http.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.dylib: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libpcre.dylib \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/obj/httpLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o -lpcre -lmpr $(LIBS)

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/http.c

$(CONFIG)/bin/http: \
    $(CONFIG)/bin/libhttp.dylib \
    $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr

$(CONFIG)/inc/ejs.h: 
	rm -fr $(CONFIG)/inc/ejs.h
	cp -r src/deps/ejs/ejs.h $(CONFIG)/inc/ejs.h

$(CONFIG)/inc/ejs.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.slots.h
	cp -r src/deps/ejs/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/inc/ejsByteGoto.h: 
	rm -fr $(CONFIG)/inc/ejsByteGoto.h
	cp -r src/deps/ejs/ejsByteGoto.h $(CONFIG)/inc/ejsByteGoto.h

$(CONFIG)/obj/ejsLib.o: \
    src/deps/ejs/ejsLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/inc/bitos.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/ejs.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/ejs/ejsLib.c

$(CONFIG)/bin/libejs.dylib: \
    $(CONFIG)/bin/libhttp.dylib \
    $(CONFIG)/bin/libpcre.dylib \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejs.slots.h \
    $(CONFIG)/inc/ejsByteGoto.h \
    $(CONFIG)/obj/ejsLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ejsLib.o -lmpr -lpcre -lhttp $(LIBS) -lpcre -lmpr

$(CONFIG)/obj/ejs.o: \
    src/deps/ejs/ejs.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/ejs/ejs.c

$(CONFIG)/bin/ejs: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lmpr -lpcre -lhttp -ledit

$(CONFIG)/obj/ejsc.o: \
    src/deps/ejs/ejsc.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/ejs/ejsc.c

$(CONFIG)/bin/ejsc: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lmpr -lpcre -lhttp

$(CONFIG)/bin/ejs.mod: $(CONFIG)/bin/ejsc
	cd src/deps/ejs; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; cd ../../..

$(CONFIG)/bin/bit.es: src/bit.es
	rm -fr $(CONFIG)/bin/bit.es
	cp -r src/bit.es $(CONFIG)/bin/bit.es

$(CONFIG)/bin/bits: 
	rm -fr ./$(CONFIG)/bin/bits
	cp -r bits ./$(CONFIG)/bin

$(CONFIG)/obj/bit.o: \
    src/bit.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/bit.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/bit.c

$(CONFIG)/bin/bit: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libhttp.dylib \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/bits \
    $(CONFIG)/bin/bit.es \
    $(CONFIG)/inc/bitos.h \
    $(CONFIG)/obj/bit.o
	$(CC) -o $(CONFIG)/bin/bit -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/bit.o -lejs -lhttp -lmpr $(LIBS) -lpcre

version: 
	@cd bits; echo 0.8.0-0 ; cd ..

deploy: compile
	mkdir -p $(BIT_BIN_PREFIX) $(BIT_VER_PREFIX)/man/man1
	rm -f $(BIT_PRD_PREFIX)/latest $(BIT_UBIN_PREFIX)/bit $(BIT_VER_PREFIX)/man/man1/bit.1 $(BIT_MAN_PREFIX)/bit.1
	cp -R -P ./$(CONFIG)/bin/bit* ./$(CONFIG)/bin/ejs.mod ./$(CONFIG)/bin/lib* ./$(CONFIG)/bin/ca.crt $(BIT_BIN_PREFIX)
	cp doc/man/bit.1 $(BIT_VER_PREFIX)/man/man1
	ln -s $(BIT_VER_PREFIX)/man/man1/bit.1 $(BIT_MAN_PREFIX)/bit.1
	ln -s $(BIT_BIN_PREFIX)/bit $(BIT_UBIN_PREFIX)/bit
	ln -s $(VERSION) $(BIT_PRD_PREFIX)/latest

install: compile deploy
	

uninstall: 
	rm -rf $(BIT_UBIN_PREFIX)/bit $(BIT_MAN_PREFIX)/bit.1 '$(BIT_PRD_PREFIX)'

