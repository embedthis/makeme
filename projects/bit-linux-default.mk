#
#   bit-linux-default.mk -- Makefile to build Embedthis Bit for linux
#

PRODUCT         := bit
VERSION         := 0.8.2
BUILD_NUMBER    := 0
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := linux
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

CFLAGS          += -fPIC   -w
DFLAGS          += -D_REENTRANT -DPIC  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/' '-Wl,-rpath,$$ORIGIN/../bin' '-rdynamic'
LIBPATHS        += -L$(CONFIG)/bin
LIBS            += -lpthread -lm -lrt -ldl

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
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/bit-linux-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/bit-linux-default-bit.h >/dev/null ; then\
		echo cp projects/bit-linux-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/bit-linux-default-bit.h $(CONFIG)/inc/bit.h  ; \
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
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/ejsLib.o
	rm -rf $(CONFIG)/obj/ejs.o
	rm -rf $(CONFIG)/obj/ejsc.o
	rm -rf $(CONFIG)/obj/bit.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/est.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/est/est.h" "$(CONFIG)/inc/est.h"

$(CONFIG)/inc/bit.h: 

$(CONFIG)/inc/bitos.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

$(CONFIG)/bin/libest.so: \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/obj/estLib.o
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/estLib.o $(LIBS)

$(CONFIG)/bin/ca.crt: \
    src/deps/est/ca.crt
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

$(CONFIG)/inc/mpr.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.so: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/obj/mprLib.o
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/est.h
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.so: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libest.so \
    $(CONFIG)/obj/mprSsl.o
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprSsl.o -lest -lmpr $(LIBS)

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o -lmpr $(LIBS) -lmpr -lpthread -lm -lrt -ldl $(LDFLAGS)

$(CONFIG)/inc/pcre.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.so: \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/obj/pcre.o
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.so: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libpcre.so \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/obj/httpLib.o
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/httpLib.o -lpcre -lmpr $(LIBS)

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

$(CONFIG)/bin/http: \
    $(CONFIG)/bin/libhttp.so \
    $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr -lhttp -lpthread -lm -lrt -ldl -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/inc/ejs.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.h" "$(CONFIG)/inc/ejs.h"

$(CONFIG)/inc/ejs.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

$(CONFIG)/inc/ejsByteGoto.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejsByteGoto.h" "$(CONFIG)/inc/ejsByteGoto.h"

$(CONFIG)/obj/ejsLib.o: \
    src/deps/ejs/ejsLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/inc/bitos.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/ejs.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsLib.c

$(CONFIG)/bin/libejs.so: \
    $(CONFIG)/bin/libhttp.so \
    $(CONFIG)/bin/libpcre.so \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejs.slots.h \
    $(CONFIG)/inc/ejsByteGoto.h \
    $(CONFIG)/obj/ejsLib.o
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsLib.o -lmpr -lpcre -lhttp $(LIBS) -lpcre -lmpr

$(CONFIG)/obj/ejs.o: \
    src/deps/ejs/ejs.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejs.c

$(CONFIG)/bin/ejs: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lmpr -lpcre -lhttp -lejs -lpthread -lm -lrt -ldl -lmpr -lpcre -lhttp $(LDFLAGS)

$(CONFIG)/obj/ejsc.o: \
    src/deps/ejs/ejsc.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsc.c

$(CONFIG)/bin/ejsc: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lmpr -lpcre -lhttp -lejs -lpthread -lm -lrt -ldl -lmpr -lpcre -lhttp $(LDFLAGS)

$(CONFIG)/bin/ejs.mod: $(CONFIG)/bin/ejsc
	$(LBIN)/ejsc --out ./$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null src/deps/ejs/ejs.es

$(CONFIG)/bin/bit.es: \
    src/bit.es
	mkdir -p "$(CONFIG)/bin"
	cp "src/bit.es" "$(CONFIG)/bin/bit.es"

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
	mkdir -p "$(CONFIG)/bin/bits"
	cp "bits/embedthis-manifest.bit" "$(CONFIG)/bin/bits/embedthis-manifest.bit"
	cp "bits/embedthis.bit" "$(CONFIG)/bin/bits/embedthis.bit"
	cp "bits/embedthis.es" "$(CONFIG)/bin/bits/embedthis.es"
	cp "bits/gendoc.es" "$(CONFIG)/bin/bits/gendoc.es"
	mkdir -p "$(CONFIG)/bin/bits/os"
	cp "bits/os/freebsd.bit" "$(CONFIG)/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "$(CONFIG)/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "$(CONFIG)/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "$(CONFIG)/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "$(CONFIG)/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "$(CONFIG)/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "$(CONFIG)/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "$(CONFIG)/bin/bits/os/windows.bit"
	cp "bits/os/freebsd.bit" "$(CONFIG)/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "$(CONFIG)/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "$(CONFIG)/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "$(CONFIG)/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "$(CONFIG)/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "$(CONFIG)/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "$(CONFIG)/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "$(CONFIG)/bin/bits/os/windows.bit"
	mkdir -p "$(CONFIG)/bin/bits/packs"
	cp "bits/packs/compiler.pak" "$(CONFIG)/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "$(CONFIG)/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "$(CONFIG)/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "$(CONFIG)/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "$(CONFIG)/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "$(CONFIG)/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "$(CONFIG)/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "$(CONFIG)/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "$(CONFIG)/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "$(CONFIG)/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "$(CONFIG)/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "$(CONFIG)/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "$(CONFIG)/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "$(CONFIG)/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "$(CONFIG)/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "$(CONFIG)/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "$(CONFIG)/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "$(CONFIG)/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "$(CONFIG)/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "$(CONFIG)/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "$(CONFIG)/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "$(CONFIG)/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "$(CONFIG)/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "$(CONFIG)/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "$(CONFIG)/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "$(CONFIG)/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "$(CONFIG)/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "$(CONFIG)/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "$(CONFIG)/bin/bits/packs/zlib.pak"
	cp "bits/packs/compiler.pak" "$(CONFIG)/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "$(CONFIG)/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "$(CONFIG)/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "$(CONFIG)/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "$(CONFIG)/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "$(CONFIG)/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "$(CONFIG)/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "$(CONFIG)/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "$(CONFIG)/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "$(CONFIG)/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "$(CONFIG)/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "$(CONFIG)/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "$(CONFIG)/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "$(CONFIG)/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "$(CONFIG)/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "$(CONFIG)/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "$(CONFIG)/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "$(CONFIG)/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "$(CONFIG)/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "$(CONFIG)/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "$(CONFIG)/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "$(CONFIG)/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "$(CONFIG)/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "$(CONFIG)/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "$(CONFIG)/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "$(CONFIG)/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "$(CONFIG)/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "$(CONFIG)/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "$(CONFIG)/bin/bits/packs/zlib.pak"
	cp "bits/sample-main.bit" "$(CONFIG)/bin/bits/sample-main.bit"
	cp "bits/sample-start.bit" "$(CONFIG)/bin/bits/sample-start.bit"
	cp "bits/simple.bit" "$(CONFIG)/bin/bits/simple.bit"
	cp "bits/standard.bit" "$(CONFIG)/bin/bits/standard.bit"
	cp "bits/vstudio.es" "$(CONFIG)/bin/bits/vstudio.es"
	cp "bits/xcode.es" "$(CONFIG)/bin/bits/xcode.es"

$(CONFIG)/obj/bit.o: \
    src/bit.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/bit.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/bit.c

$(CONFIG)/bin/bit: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libhttp.so \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/bits \
    $(CONFIG)/bin/bit.es \
    $(CONFIG)/inc/bitos.h \
    $(CONFIG)/obj/bit.o
	$(CC) -o $(CONFIG)/bin/bit $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/bit.o -lejs -lhttp -lmpr $(LIBS) -lpcre -lejs -lhttp -lmpr -lpthread -lm -lrt -ldl -lpcre $(LDFLAGS)

version: 
	@echo 0.8.2-0

stop: 
	

installBinary: stop
	mkdir -p "$(BIT_VAPP_PREFIX)"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits"
	cp "bits/embedthis-manifest.bit" "$(BIT_VAPP_PREFIX)/bin/bits/embedthis-manifest.bit"
	cp "bits/embedthis.bit" "$(BIT_VAPP_PREFIX)/bin/bits/embedthis.bit"
	cp "bits/embedthis.es" "$(BIT_VAPP_PREFIX)/bin/bits/embedthis.es"
	cp "bits/gendoc.es" "$(BIT_VAPP_PREFIX)/bin/bits/gendoc.es"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits/os"
	cp "bits/os/freebsd.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/macosx.bit"
	cp "bits/os/posix.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/posix.bit"
	cp "bits/os/solaris.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/solaris.bit"
	cp "bits/os/vxworks.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "$(BIT_VAPP_PREFIX)/bin/bits/os/windows.bit"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits/packs"
	cp "bits/packs/compiler.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/compiler.pak"
	cp "bits/packs/doxygen.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/doxygen.pak"
	cp "bits/packs/dsi.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/dsi.pak"
	cp "bits/packs/dumpbin.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/dumpbin.pak"
	cp "bits/packs/ejs.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/ejs.pak"
	cp "bits/packs/ejscript.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/ejscript.pak"
	cp "bits/packs/est.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/est.pak"
	cp "bits/packs/http.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/http.pak"
	cp "bits/packs/lib.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/lib.pak"
	cp "bits/packs/link.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/link.pak"
	cp "bits/packs/man.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/man.pak"
	cp "bits/packs/man2html.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/man2html.pak"
	cp "bits/packs/matrixssl.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/matrixssl.pak"
	cp "bits/packs/md5.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/md5.pak"
	cp "bits/packs/mocana.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/mocana.pak"
	cp "bits/packs/openssl.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/openssl.pak"
	cp "bits/packs/pcre.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/pcre.pak"
	cp "bits/packs/pmaker.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/pmaker.pak"
	cp "bits/packs/ranlib.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/ranlib.pak"
	cp "bits/packs/rc.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/rc.pak"
	cp "bits/packs/sqlite.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/sqlite.pak"
	cp "bits/packs/ssl.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/ssl.pak"
	cp "bits/packs/strip.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/strip.pak"
	cp "bits/packs/tidy.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/tidy.pak"
	cp "bits/packs/utest.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/utest.pak"
	cp "bits/packs/vxworks.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/vxworks.pak"
	cp "bits/packs/winsdk.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/winsdk.pak"
	cp "bits/packs/zip.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/zip.pak"
	cp "bits/packs/zlib.pak" "$(BIT_VAPP_PREFIX)/bin/bits/packs/zlib.pak"
	cp "bits/sample-main.bit" "$(BIT_VAPP_PREFIX)/bin/bits/sample-main.bit"
	cp "bits/sample-start.bit" "$(BIT_VAPP_PREFIX)/bin/bits/sample-start.bit"
	cp "bits/simple.bit" "$(BIT_VAPP_PREFIX)/bin/bits/simple.bit"
	cp "bits/standard.bit" "$(BIT_VAPP_PREFIX)/bin/bits/standard.bit"
	cp "bits/vstudio.es" "$(BIT_VAPP_PREFIX)/bin/bits/vstudio.es"
	cp "bits/xcode.es" "$(BIT_VAPP_PREFIX)/bin/bits/xcode.es"
	mkdir -p "$(BIT_VAPP_PREFIX)/doc/man/man1"
	cp "doc/man/bit.1" "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/bit.1"
	mkdir -p "$(BIT_MAN_PREFIX)/man1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1" "$(BIT_MAN_PREFIX)/man1/bit.1"
	rm -f "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_APP_PREFIX)"
	ln -s "0.8.2" "$(BIT_APP_PREFIX)/latest"


start: 
	

install: stop installBinary start
	

uninstall: stop


