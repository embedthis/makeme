#
#   bit-macosx-default.mk -- Makefile to build Embedthis Bit for macosx
#

PRODUCT         := bit
VERSION         := 0.8.2
BUILD_NUMBER    := 0
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := macosx
CC              := /usr/bin/clang
LD              := /usr/bin/ld
CONFIG          := $(OS)-$(ARCH)-$(PROFILE)
LBIN            := $(CONFIG)/bin

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

BIT_PACK_EST          := 1
BIT_PACK_EJSCRIPT     := 1
BIT_PACK_SSL          := 1

BIT_ROOT_PREFIX       := 
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


ifeq ($(BIT_PACK_EST),1)
TARGETS += $(CONFIG)/bin/libest.dylib
endif
TARGETS     += $(CONFIG)/bin/ca.crt
TARGETS     += $(CONFIG)/bin/libmpr.dylib
ifeq ($(BIT_PACK_SSL),1)
TARGETS += $(CONFIG)/bin/libmprssl.dylib
endif
TARGETS     += $(CONFIG)/bin/makerom
TARGETS     += $(CONFIG)/bin/libpcre.dylib
TARGETS     += $(CONFIG)/bin/libhttp.dylib
TARGETS     += $(CONFIG)/bin/http
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS += $(CONFIG)/bin/libejs.dylib
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS += $(CONFIG)/bin/ejs
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS += $(CONFIG)/bin/ejsc
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS += $(CONFIG)/bin/ejs.mod
endif
TARGETS     += $(CONFIG)/bin/bit.es
TARGETS     += $(CONFIG)/bin/bit
TARGETS     += $(CONFIG)/bin/bits

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(BIT_APP_PREFIX)" = "" ] ; then echo WARNING: BIT_APP_PREFIX not set ; exit 255 ; fi
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

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_1)
	@echo '      [File] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/est/est.h" "$(CONFIG)/inc/est.h"

#
#   bit.h
#
$(CONFIG)/inc/bit.h: $(DEPS_2)

#
#   bitos.h
#
$(CONFIG)/inc/bitos.h: $(DEPS_3)
	@echo '      [File] $(CONFIG)/inc/bitos.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

#
#   estLib.o
#
DEPS_4 += $(CONFIG)/inc/bit.h
DEPS_4 += $(CONFIG)/inc/est.h
DEPS_4 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c $(DEPS_4)
	@echo '   [Compile] src/deps/est/estLib.c'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

ifeq ($(BIT_PACK_EST),1)
#
#   libest
#
DEPS_5 += $(CONFIG)/inc/est.h
DEPS_5 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.dylib: $(DEPS_5)
	@echo '      [Link] libest'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libest.dylib $(CONFIG)/obj/estLib.o $(LIBS)
endif

#
#   ca-crt
#
DEPS_6 += src/deps/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_6)
	@echo '      [File] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_7)
	@echo '      [File] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

#
#   mprLib.o
#
DEPS_8 += $(CONFIG)/inc/bit.h
DEPS_8 += $(CONFIG)/inc/mpr.h
DEPS_8 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c $(DEPS_8)
	@echo '   [Compile] src/deps/mpr/mprLib.c'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

#
#   libmpr
#
DEPS_9 += $(CONFIG)/inc/mpr.h
DEPS_9 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.dylib: $(DEPS_9)
	@echo '      [Link] libmpr'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

#
#   mprSsl.o
#
DEPS_10 += $(CONFIG)/inc/bit.h
DEPS_10 += $(CONFIG)/inc/mpr.h
DEPS_10 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c $(DEPS_10)
	@echo '   [Compile] src/deps/mpr/mprSsl.c'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

ifeq ($(BIT_PACK_SSL),1)
#
#   libmprssl
#
DEPS_11 += $(CONFIG)/bin/libmpr.dylib
ifeq ($(BIT_PACK_EST),1)
    DEPS_11 += $(CONFIG)/bin/libest.dylib
endif
DEPS_11 += $(CONFIG)/obj/mprSsl.o

ifeq ($(BIT_PACK_EST),1)
    LIBS_11 += -lest
endif
LIBS_11 += -lmpr

$(CONFIG)/bin/libmprssl.dylib: $(DEPS_11)
	@echo '      [Link] libmprssl'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libmprssl.dylib $(CONFIG)/obj/mprSsl.o $(LIBS_11) $(LIBS_11) $(LIBS)
endif

#
#   makerom.o
#
DEPS_12 += $(CONFIG)/inc/bit.h
DEPS_12 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c $(DEPS_12)
	@echo '   [Compile] src/deps/mpr/makerom.c'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

#
#   makerom
#
DEPS_13 += $(CONFIG)/bin/libmpr.dylib
DEPS_13 += $(CONFIG)/obj/makerom.o

LIBS_13 += -lmpr

$(CONFIG)/bin/makerom: $(DEPS_13)
	@echo '      [Link] makerom'
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS_13) $(LIBS_13) $(LIBS)

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_14)
	@echo '      [File] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

#
#   pcre.o
#
DEPS_15 += $(CONFIG)/inc/bit.h
DEPS_15 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c $(DEPS_15)
	@echo '   [Compile] src/deps/pcre/pcre.c'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

#
#   libpcre
#
DEPS_16 += $(CONFIG)/inc/pcre.h
DEPS_16 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.dylib: $(DEPS_16)
	@echo '      [Link] libpcre'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_17)
	@echo '      [File] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

#
#   httpLib.o
#
DEPS_18 += $(CONFIG)/inc/bit.h
DEPS_18 += $(CONFIG)/inc/http.h
DEPS_18 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c $(DEPS_18)
	@echo '   [Compile] src/deps/http/httpLib.c'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

#
#   libhttp
#
DEPS_19 += $(CONFIG)/bin/libmpr.dylib
DEPS_19 += $(CONFIG)/bin/libpcre.dylib
DEPS_19 += $(CONFIG)/inc/http.h
DEPS_19 += $(CONFIG)/obj/httpLib.o

LIBS_19 += -lpcre
LIBS_19 += -lmpr

$(CONFIG)/bin/libhttp.dylib: $(DEPS_19)
	@echo '      [Link] libhttp'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o $(LIBS_19) $(LIBS_19) $(LIBS)

#
#   http.o
#
DEPS_20 += $(CONFIG)/inc/bit.h
DEPS_20 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c $(DEPS_20)
	@echo '   [Compile] src/deps/http/http.c'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

#
#   http
#
DEPS_21 += $(CONFIG)/bin/libhttp.dylib
DEPS_21 += $(CONFIG)/obj/http.o

LIBS_21 += -lhttp
LIBS_21 += -lpcre
LIBS_21 += -lmpr

$(CONFIG)/bin/http: $(DEPS_21)
	@echo '      [Link] http'
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS_21) $(LIBS_21) $(LIBS) -lmpr

#
#   ejs.h
#
$(CONFIG)/inc/ejs.h: $(DEPS_22)
	@echo '      [File] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.h" "$(CONFIG)/inc/ejs.h"

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_23)
	@echo '      [File] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

#
#   ejsByteGoto.h
#
$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_24)
	@echo '      [File] $(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejsByteGoto.h" "$(CONFIG)/inc/ejsByteGoto.h"

#
#   ejsLib.o
#
DEPS_25 += $(CONFIG)/inc/bit.h
DEPS_25 += $(CONFIG)/inc/ejs.h
DEPS_25 += $(CONFIG)/inc/mpr.h
DEPS_25 += $(CONFIG)/inc/pcre.h
DEPS_25 += $(CONFIG)/inc/bitos.h
DEPS_25 += $(CONFIG)/inc/http.h
DEPS_25 += $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/obj/ejsLib.o: \
    src/deps/ejs/ejsLib.c $(DEPS_25)
	@echo '   [Compile] src/deps/ejs/ejsLib.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsLib.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   libejs
#
DEPS_26 += $(CONFIG)/bin/libhttp.dylib
DEPS_26 += $(CONFIG)/bin/libpcre.dylib
DEPS_26 += $(CONFIG)/bin/libmpr.dylib
DEPS_26 += $(CONFIG)/inc/ejs.h
DEPS_26 += $(CONFIG)/inc/ejs.slots.h
DEPS_26 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_26 += $(CONFIG)/obj/ejsLib.o

LIBS_26 += -lmpr
LIBS_26 += -lpcre
LIBS_26 += -lhttp
LIBS_26 += -lpcre
LIBS_26 += -lmpr

$(CONFIG)/bin/libejs.dylib: $(DEPS_26)
	@echo '      [Link] libejs'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib $(LDFLAGS) -compatibility_version 0.8.2 -current_version 0.8.2 $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ejsLib.o $(LIBS_26) $(LIBS_26) $(LIBS) -lmpr
endif

#
#   ejs.o
#
DEPS_27 += $(CONFIG)/inc/bit.h
DEPS_27 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/deps/ejs/ejs.c $(DEPS_27)
	@echo '   [Compile] src/deps/ejs/ejs.c'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejs.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs
#
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_28 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_28 += $(CONFIG)/obj/ejs.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_28 += -lejs
endif
LIBS_28 += -lmpr
LIBS_28 += -lpcre
LIBS_28 += -lhttp

$(CONFIG)/bin/ejs: $(DEPS_28)
	@echo '      [Link] ejs'
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS_28) $(LIBS_28) $(LIBS) -ledit
endif

#
#   ejsc.o
#
DEPS_29 += $(CONFIG)/inc/bit.h
DEPS_29 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/deps/ejs/ejsc.c $(DEPS_29)
	@echo '   [Compile] src/deps/ejs/ejsc.c'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsc.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejsc
#
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_30 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_30 += $(CONFIG)/obj/ejsc.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_30 += -lejs
endif
LIBS_30 += -lmpr
LIBS_30 += -lpcre
LIBS_30 += -lhttp

$(CONFIG)/bin/ejsc: $(DEPS_30)
	@echo '      [Link] ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS_30) $(LIBS_30) $(LIBS) -lhttp
endif

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs.mod
#
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_31 += $(CONFIG)/bin/ejsc
endif

$(CONFIG)/bin/ejs.mod: $(DEPS_31)
	$(LBIN)/ejsc --out ./$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null src/deps/ejs/ejs.es
endif

#
#   bit.es
#
DEPS_32 += src/bit.es

$(CONFIG)/bin/bit.es: $(DEPS_32)
	@echo '      [File] $(CONFIG)/bin/bit.es'
	mkdir -p "$(CONFIG)/bin"
	cp "src/bit.es" "$(CONFIG)/bin/bit.es"

#
#   bits
#
DEPS_33 += bits

$(CONFIG)/bin/bits: $(DEPS_33)
	@echo '      [File] $(CONFIG)/bin/bits'
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

#
#   bit.o
#
DEPS_34 += $(CONFIG)/inc/bit.h
DEPS_34 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/bit.o: \
    src/bit.c $(DEPS_34)
	@echo '   [Compile] src/bit.c'
	$(CC) -c -o $(CONFIG)/obj/bit.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/bit.c

#
#   bit
#
DEPS_35 += $(CONFIG)/bin/libmpr.dylib
DEPS_35 += $(CONFIG)/bin/libhttp.dylib
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_35 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_35 += $(CONFIG)/bin/bits
DEPS_35 += $(CONFIG)/bin/bit.es
DEPS_35 += $(CONFIG)/inc/bitos.h
DEPS_35 += $(CONFIG)/obj/bit.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_35 += -lejs
endif
LIBS_35 += -lhttp
LIBS_35 += -lmpr
LIBS_35 += -lpcre

$(CONFIG)/bin/bit: $(DEPS_35)
	@echo '      [Link] bit'
	$(CC) -o $(CONFIG)/bin/bit -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/bit.o $(LIBS_35) $(LIBS_35) $(LIBS) -lpcre

#
#   version
#
version: $(DEPS_36)
	@echo 0.8.2-0

#
#   stop
#
stop: $(DEPS_37)
	

#
#   installBinary
#
DEPS_38 += stop

installBinary: $(DEPS_38)
	mkdir -p "$(BIT_APP_PREFIX)"
	mkdir -p "$(BIT_VAPP_PREFIX)"
	mkdir -p "$(BIT_APP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	ln -s "0.8.2" "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin"
	cp "$(CONFIG)/bin/bit" "$(BIT_VAPP_PREFIX)/bin/bit"
	mkdir -p "$(BIT_BIN_PREFIX)"
	rm -f "$(BIT_BIN_PREFIX)/bit"
	ln -s "$(BIT_VAPP_PREFIX)/bin/bit" "$(BIT_BIN_PREFIX)/bit"
	cp "$(CONFIG)/bin/bit.es" "$(BIT_VAPP_PREFIX)/bin/bit.es"
	cp "$(CONFIG)/bin/ca.crt" "$(BIT_VAPP_PREFIX)/bin/ca.crt"
	cp "$(CONFIG)/bin/ejs.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.mod"
	cp "$(CONFIG)/bin/libejs.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.dylib"
	cp "$(CONFIG)/bin/libest.dylib" "$(BIT_VAPP_PREFIX)/bin/libest.dylib"
	cp "$(CONFIG)/bin/libhttp.dylib" "$(BIT_VAPP_PREFIX)/bin/libhttp.dylib"
	cp "$(CONFIG)/bin/libmpr.dylib" "$(BIT_VAPP_PREFIX)/bin/libmpr.dylib"
	cp "$(CONFIG)/bin/libmprssl.dylib" "$(BIT_VAPP_PREFIX)/bin/libmprssl.dylib"
	cp "$(CONFIG)/bin/libpcre.dylib" "$(BIT_VAPP_PREFIX)/bin/libpcre.dylib"
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
	mkdir -p "$(BIT_MAN_PREFIX)/man1"
	rm -f "$(BIT_MAN_PREFIX)/man1/bit.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1" "$(BIT_MAN_PREFIX)/man1/bit.1"

#
#   start
#
start: $(DEPS_39)
	

#
#   install
#
DEPS_40 += stop
DEPS_40 += installBinary
DEPS_40 += start

install: $(DEPS_40)
	

#
#   uninstall
#
DEPS_41 += stop

uninstall: $(DEPS_41)
	rm -fr "$(BIT_VAPP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	rmdir -p "$(BIT_APP_PREFIX)"

