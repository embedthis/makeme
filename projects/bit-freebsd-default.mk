#
#   bit-freebsd-default.mk -- Makefile to build Embedthis Bit for freebsd
#

PRODUCT         := bit
VERSION         := 0.8.1
BUILD_NUMBER    := 0
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := freebsd
CC              := /usr/bin/gcc
LD              := /usr/bin/ld
CONFIG          := $(OS)-$(ARCH)-$(PROFILE)
LBIN            := $(CONFIG)/bin

BIT_ROOT_PREFIX       := /
BIT_BASE_PREFIX       := $(BIT_ROOT_PREFIX)/usr/local
BIT_DATA_PREFIX       := $(BIT_ROOT_PREFIX)/
BIT_STATE_PREFIX      := $(BIT_ROOT_PREFIX)/var
BIT_APP_PREFIX        := $(BIT_BASE_PREFIX)/lib/$(PRODUCT)
BIT_VAPP_PREFIX       := $(BIT_APP_PREFIX)/$(VERSION)
BIT_BIN_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/bin
BIT_INC_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/include
BIT_LIB_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/lib
BIT_MAN_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/share/man
BIT_SBIN_PREFIX       := $(BIT_ROOT_PREFIX)/usr/local/sbin
BIT_ETC_PREFIX        := $(BIT_ROOT_PREFIX)/etc/$(PRODUCT)
BIT_WEB_PREFIX        := $(BIT_ROOT_PREFIX)/var/www/$(PRODUCT)-default
BIT_LOG_PREFIX        := $(BIT_ROOT_PREFIX)/var/log/$(PRODUCT)
BIT_SPOOL_PREFIX      := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)
BIT_CACHE_PREFIX      := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)/cache
BIT_SRC_PREFIX        := $(BIT_ROOT_PREFIX)$(PRODUCT)-$(VERSION)

CFLAGS          += -fPIC  -w
DFLAGS          += -D_REENTRANT -DPIC  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += '-g'
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
        $(CONFIG)/bin/libest.so \
        $(CONFIG)/bin/ca.crt \
        $(CONFIG)/bin/libmpr.so \
        $(CONFIG)/bin/libmprssl.so \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/bin/libpcre.so \
        $(CONFIG)/bin/libhttp.so \
        $(CONFIG)/bin/http \
        $(CONFIG)/bin/libejs.so \
        $(CONFIG)/bin/ejs \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/bit.es \
        $(CONFIG)/bin/bit \
        $(CONFIG)/bin/bits

.PHONY: prep

prep:
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(BIT_APP_PREFIX)" = "" ] ; then echo WARNING: BIT_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/bit-freebsd-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/bit-freebsd-default-bit.h >/dev/null ; then\
		echo cp projects/bit-freebsd-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/bit-freebsd-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
clean:
	rm -rf $(CONFIG)/bin/libest.so
	rm -rf $(CONFIG)/bin/ca.crt
	rm -rf $(CONFIG)/bin/libmpr.so
	rm -rf $(CONFIG)/bin/libmprssl.so
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/bin/libpcre.so
	rm -rf $(CONFIG)/bin/libhttp.so
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/bin/libejs.so
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
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/est/est.h" "/Users/mob/git/bit/freebsd-x86-default/inc/est.h"

$(CONFIG)/inc/bit.h: 

$(CONFIG)/inc/bitos.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/bitos.h" "/Users/mob/git/bit/freebsd-x86-default/inc/bitos.h"

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c
	$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

$(CONFIG)/bin/libest.so: 
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/estLib.o $(LIBS)

$(CONFIG)/bin/ca.crt: \
    src/deps/est/ca.crt
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/bin"
	cp "src/deps/est/ca.crt" "/Users/mob/git/bit/freebsd-x86-default/bin/ca.crt"

$(CONFIG)/inc/mpr.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/mpr/mpr.h" "/Users/mob/git/bit/freebsd-x86-default/inc/mpr.h"

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.so: 
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.so: 
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprSsl.o -lest -lmpr $(LIBS)

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c
	$(CC) -c -o $(CONFIG)/obj/makerom.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom: 
	$(CC) -o $(CONFIG)/bin/makerom $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o -lmpr $(LIBS) -lmpr -lpthread -lm -ldl $(LDFLAGS)

$(CONFIG)/inc/pcre.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/pcre/pcre.h" "/Users/mob/git/bit/freebsd-x86-default/inc/pcre.h"

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c
	$(CC) -c -o $(CONFIG)/obj/pcre.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.so: 
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/http/http.h" "/Users/mob/git/bit/freebsd-x86-default/inc/http.h"

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.so: 
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/httpLib.o -lpcre -lmpr $(LIBS)

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c
	$(CC) -c -o $(CONFIG)/obj/http.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

$(CONFIG)/bin/http: 
	$(CC) -o $(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr -lhttp -lpthread -lm -ldl -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/inc/ejs.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/ejs/ejs.h" "/Users/mob/git/bit/freebsd-x86-default/inc/ejs.h"

$(CONFIG)/inc/ejs.slots.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/ejs/ejs.slots.h" "/Users/mob/git/bit/freebsd-x86-default/inc/ejs.slots.h"

$(CONFIG)/inc/ejsByteGoto.h: 
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/inc"
	cp "src/deps/ejs/ejsByteGoto.h" "/Users/mob/git/bit/freebsd-x86-default/inc/ejsByteGoto.h"

$(CONFIG)/obj/ejsLib.o: \
    src/deps/ejs/ejsLib.c
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsLib.c

$(CONFIG)/bin/libejs.so: 
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsLib.o -lmpr -lpcre -lhttp $(LIBS) -lpcre -lmpr

$(CONFIG)/obj/ejs.o: \
    src/deps/ejs/ejs.c
	$(CC) -c -o $(CONFIG)/obj/ejs.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejs.c

$(CONFIG)/bin/ejs: 
	$(CC) -o $(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lmpr -lpcre -lhttp -lejs -lpthread -lm -ldl -lmpr -lpcre -lhttp $(LDFLAGS)

$(CONFIG)/obj/ejsc.o: \
    src/deps/ejs/ejsc.c
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsc.c

$(CONFIG)/bin/ejsc: 
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lmpr -lpcre -lhttp -lejs -lpthread -lm -ldl -lmpr -lpcre -lhttp $(LDFLAGS)

$(CONFIG)/bin/ejs.mod: 
	$(LBIN)/ejsc --out ./$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null src/deps/ejs/ejs.es

$(CONFIG)/bin/bit.es: \
    src/bit.es
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/bin"
	cp "src/bit.es" "/Users/mob/git/bit/freebsd-x86-default/bin/bit.es"

$(CONFIG)/bin/bits: \
    bits/embedthis-manifest.bit \
    bits/embedthis.bit \
    bits/embedthis.es \
    bits/gendoc.es \
    bits/os \
    bits/os/freebsd.bit \
    bits/os/gcc.bit \
    bits/os/linux.bit \
    bits/os/macosx.bit \
    bits/os/posix.bit \
    bits/os/solaris.bit \
    bits/os/vxworks.bit \
    bits/os/windows.bit \
    bits/packs \
    bits/packs/compiler.pak \
    bits/packs/doxygen.pak \
    bits/packs/dsi.pak \
    bits/packs/dumpbin.pak \
    bits/packs/ejs.pak \
    bits/packs/ejscript.pak \
    bits/packs/est.pak \
    bits/packs/http.pak \
    bits/packs/lib.pak \
    bits/packs/link.pak \
    bits/packs/man.pak \
    bits/packs/man2html.pak \
    bits/packs/matrixssl.pak \
    bits/packs/md5.pak \
    bits/packs/mocana.pak \
    bits/packs/openssl.pak \
    bits/packs/pcre.pak \
    bits/packs/pmaker.pak \
    bits/packs/ranlib.pak \
    bits/packs/rc.pak \
    bits/packs/sqlite.pak \
    bits/packs/ssl.pak \
    bits/packs/strip.pak \
    bits/packs/tidy.pak \
    bits/packs/utest.pak \
    bits/packs/vxworks.pak \
    bits/packs/winsdk.pak \
    bits/packs/zip.pak \
    bits/packs/zlib.pak \
    bits/sample-main.bit \
    bits/sample-start.bit \
    bits/simple.bit \
    bits/standard.bit \
    bits/vstudio.es \
    bits/xcode.es
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/bin/bits"
	cp "bits/embedthis-manifest.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/embedthis-manifest.bit"
	cp "bits/embedthis.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/embedthis.bit"
	cp "bits/embedthis.es" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/embedthis.es"
	cp "bits/gendoc.es" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/gendoc.es"
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os"
	cp "bits/os/freebsd.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/windows.bit"
	cp "bits/os/freebsd.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/os/windows.bit"
	mkdir -p "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs"
	cp "bits/packs/compiler.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/zlib.pak"
	cp "bits/packs/compiler.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/packs/zlib.pak"
	cp "bits/sample-main.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/sample-main.bit"
	cp "bits/sample-start.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/sample-start.bit"
	cp "bits/simple.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/simple.bit"
	cp "bits/standard.bit" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/standard.bit"
	cp "bits/vstudio.es" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/vstudio.es"
	cp "bits/xcode.es" "/Users/mob/git/bit/freebsd-x86-default/bin/bits/xcode.es"

$(CONFIG)/obj/bit.o: \
    src/bit.c
	$(CC) -c -o $(CONFIG)/obj/bit.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/bit.c

$(CONFIG)/bin/bit: 
	$(CC) -o $(CONFIG)/bin/bit $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/bit.o -lejs -lhttp -lmpr $(LIBS) -lpcre -lejs -lhttp -lmpr -lpthread -lm -ldl -lpcre $(LDFLAGS)

version: 
	@cd bits; echo 0.8.1-0 ; cd ..

stop: 
	

installBinary: 
	mkdir -p "/usr/local/lib/bit/0.8.1/bin"
	mkdir -p "/usr/local/lib/bit/0.8.1/bin/bits"
	cp "bits/embedthis-manifest.bit" "/usr/local/lib/bit/0.8.1/bin/bits/embedthis-manifest.bit"
	cp "bits/embedthis.bit" "/usr/local/lib/bit/0.8.1/bin/bits/embedthis.bit"
	cp "bits/embedthis.es" "/usr/local/lib/bit/0.8.1/bin/bits/embedthis.es"
	cp "bits/gendoc.es" "/usr/local/lib/bit/0.8.1/bin/bits/gendoc.es"
	mkdir -p "/usr/local/lib/bit/0.8.1/bin/bits/os"
	cp "bits/os/freebsd.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "/usr/local/lib/bit/0.8.1/bin/bits/os/windows.bit"
	mkdir -p "/usr/local/lib/bit/0.8.1/bin/bits/packs"
	cp "bits/packs/compiler.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "/usr/local/lib/bit/0.8.1/bin/bits/packs/zlib.pak"
	cp "bits/sample-main.bit" "/usr/local/lib/bit/0.8.1/bin/bits/sample-main.bit"
	cp "bits/sample-start.bit" "/usr/local/lib/bit/0.8.1/bin/bits/sample-start.bit"
	cp "bits/simple.bit" "/usr/local/lib/bit/0.8.1/bin/bits/simple.bit"
	cp "bits/standard.bit" "/usr/local/lib/bit/0.8.1/bin/bits/standard.bit"
	cp "bits/vstudio.es" "/usr/local/lib/bit/0.8.1/bin/bits/vstudio.es"
	cp "bits/xcode.es" "/usr/local/lib/bit/0.8.1/bin/bits/xcode.es"
	mkdir -p "/usr/local/lib/bit/0.8.1/doc/man/man1"
	cp "doc/man/bit.1" "/usr/local/lib/bit/0.8.1/doc/man/man1/bit.1"
	rm -f "/usr/local/share/man/man1/bit.1"
	mkdir -p "/usr/local/share/man/man1"
	ln -s "/usr/local/lib/bit/0.8.1/doc/man/man1/bit.1" "/usr/local/share/man/man1/bit.1"
	rm -f "/usr/local/lib/bit/latest"
	mkdir -p "/usr/local/lib/bit"
	ln -s "0.8.1" "/usr/local/lib/bit/latest"


start: 
	

install: 
	

uninstall: 


