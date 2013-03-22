#
#   bit-macosx-default.mk -- Makefile to build Embedthis Bit for macosx
#

PRODUCT            := bit
VERSION            := 0.8.5
BUILD_NUMBER       := 0
PROFILE            := default
ARCH               := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS                 := macosx
CC                 := /usr/bin/clang
LD                 := /usr/bin/ld
CONFIG             := $(OS)-$(ARCH)-$(PROFILE)
LBIN               := $(CONFIG)/bin

BIT_PACK_EJSCRIPT  := 1
BIT_PACK_EST       := 1
BIT_PACK_SSL       := 1

ifeq ($(BIT_PACK_EST),1)
    BIT_PACK_SSL := 1
endif
ifeq ($(BIT_PACK_LIB),1)
    BIT_PACK_COMPILER := 1
endif
ifeq ($(BIT_PACK_MATRIXSSL),1)
    BIT_PACK_SSL := 1
endif
ifeq ($(BIT_PACK_NANOSSL),1)
    BIT_PACK_SSL := 1
endif
ifeq ($(BIT_PACK_OPENSSL),1)
    BIT_PACK_SSL := 1
endif

CFLAGS             += -w
DFLAGS             +=  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS))) -DBIT_PACK_EJSCRIPT=$(BIT_PACK_EJSCRIPT) -DBIT_PACK_EST=$(BIT_PACK_EST) -DBIT_PACK_SSL=$(BIT_PACK_SSL) 
IFLAGS             += -I$(CONFIG)/inc
LDFLAGS            += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS           += -L$(CONFIG)/bin
LIBS               += -lpthread -lm -ldl

DEBUG              := debug
CFLAGS-debug       := -g
DFLAGS-debug       := -DBIT_DEBUG
LDFLAGS-debug      := -g
DFLAGS-release     := 
CFLAGS-release     := -O2
LDFLAGS-release    := 
CFLAGS             += $(CFLAGS-$(DEBUG))
DFLAGS             += $(DFLAGS-$(DEBUG))
LDFLAGS            += $(LDFLAGS-$(DEBUG))

BIT_ROOT_PREFIX    := 
BIT_BASE_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local
BIT_DATA_PREFIX    := $(BIT_ROOT_PREFIX)/
BIT_STATE_PREFIX   := $(BIT_ROOT_PREFIX)/var
BIT_APP_PREFIX     := $(BIT_BASE_PREFIX)/lib/$(PRODUCT)
BIT_VAPP_PREFIX    := $(BIT_APP_PREFIX)/$(VERSION)
BIT_BIN_PREFIX     := $(BIT_ROOT_PREFIX)/usr/local/bin
BIT_INC_PREFIX     := $(BIT_ROOT_PREFIX)/usr/local/include
BIT_LIB_PREFIX     := $(BIT_ROOT_PREFIX)/usr/local/lib
BIT_MAN_PREFIX     := $(BIT_ROOT_PREFIX)/usr/local/share/man
BIT_SBIN_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local/sbin
BIT_ETC_PREFIX     := $(BIT_ROOT_PREFIX)/etc/$(PRODUCT)
BIT_WEB_PREFIX     := $(BIT_ROOT_PREFIX)/var/www/$(PRODUCT)-default
BIT_LOG_PREFIX     := $(BIT_ROOT_PREFIX)/var/log/$(PRODUCT)
BIT_SPOOL_PREFIX   := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)
BIT_CACHE_PREFIX   := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)/cache
BIT_SRC_PREFIX     := $(BIT_ROOT_PREFIX)$(PRODUCT)-$(VERSION)


ifeq ($(BIT_PACK_EST),1)
TARGETS            += $(CONFIG)/bin/libest.dylib
endif
TARGETS            += $(CONFIG)/bin/ca.crt
TARGETS            += $(CONFIG)/bin/libmpr.dylib
ifeq ($(BIT_PACK_SSL),1)
TARGETS            += $(CONFIG)/bin/libmprssl.dylib
endif
TARGETS            += $(CONFIG)/bin/makerom
TARGETS            += $(CONFIG)/bin/libpcre.dylib
TARGETS            += $(CONFIG)/bin/libhttp.dylib
TARGETS            += $(CONFIG)/bin/http
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS            += $(CONFIG)/bin/libejs.dylib
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS            += $(CONFIG)/bin/ejs
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS            += $(CONFIG)/bin/ejsc
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS            += $(CONFIG)/bin/ejs.mod
endif
TARGETS            += $(CONFIG)/bin/bit

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
	@if ! diff $(CONFIG)/inc/bitos.h src/bitos.h >/dev/null ; then\
		cp src/bitos.h $(CONFIG)/inc/bitos.h  ; \
	fi; true
	@if ! diff $(CONFIG)/inc/bit.h projects/bit-macosx-default-bit.h >/dev/null ; then\
		cp projects/bit-macosx-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags
clean:
	rm -fr "$(CONFIG)/bin/libest.dylib"
	rm -fr "$(CONFIG)/bin/ca.crt"
	rm -fr "$(CONFIG)/bin/libmpr.dylib"
	rm -fr "$(CONFIG)/bin/libmprssl.dylib"
	rm -fr "$(CONFIG)/bin/makerom"
	rm -fr "$(CONFIG)/bin/libpcre.dylib"
	rm -fr "$(CONFIG)/bin/libhttp.dylib"
	rm -fr "$(CONFIG)/bin/http"
	rm -fr "$(CONFIG)/bin/libejs.dylib"
	rm -fr "$(CONFIG)/bin/ejs"
	rm -fr "$(CONFIG)/bin/ejsc"
	rm -fr "$(CONFIG)/obj/estLib.o"
	rm -fr "$(CONFIG)/obj/mprLib.o"
	rm -fr "$(CONFIG)/obj/mprSsl.o"
	rm -fr "$(CONFIG)/obj/makerom.o"
	rm -fr "$(CONFIG)/obj/pcre.o"
	rm -fr "$(CONFIG)/obj/httpLib.o"
	rm -fr "$(CONFIG)/obj/http.o"
	rm -fr "$(CONFIG)/obj/ejsLib.o"
	rm -fr "$(CONFIG)/obj/ejs.o"
	rm -fr "$(CONFIG)/obj/ejsc.o"
	rm -fr "$(CONFIG)/obj/bit.o"

clobber: clean
	rm -fr ./$(CONFIG)



#
#   version
#
version: $(DEPS_1)
	@cd bits; echo NN 0.8.5-0 ; cd ..

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/est/est.h" "$(CONFIG)/inc/est.h"

#
#   bit.h
#
$(CONFIG)/inc/bit.h: $(DEPS_3)
	@echo '      [Copy] $(CONFIG)/inc/bit.h'

#
#   bitos.h
#
$(CONFIG)/inc/bitos.h: $(DEPS_4)
	@echo '      [Copy] $(CONFIG)/inc/bitos.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

#
#   estLib.o
#
DEPS_5 += $(CONFIG)/inc/bit.h
DEPS_5 += $(CONFIG)/inc/est.h
DEPS_5 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c $(DEPS_5)
	@echo '   [Compile] src/deps/est/estLib.c'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

ifeq ($(BIT_PACK_EST),1)
#
#   libest
#
DEPS_6 += $(CONFIG)/inc/est.h
DEPS_6 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.dylib: $(DEPS_6)
	@echo '      [Link] libest'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libest.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/estLib.o $(LIBS) 
endif

#
#   ca-crt
#
DEPS_7 += src/deps/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_8)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

#
#   mprLib.o
#
DEPS_9 += $(CONFIG)/inc/bit.h
DEPS_9 += $(CONFIG)/inc/mpr.h
DEPS_9 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c $(DEPS_9)
	@echo '   [Compile] src/deps/mpr/mprLib.c'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

#
#   libmpr
#
DEPS_10 += $(CONFIG)/inc/mpr.h
DEPS_10 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.dylib: $(DEPS_10)
	@echo '      [Link] libmpr'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/mprLib.o $(LIBS) 

#
#   mprSsl.o
#
DEPS_11 += $(CONFIG)/inc/bit.h
DEPS_11 += $(CONFIG)/inc/mpr.h
DEPS_11 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c $(DEPS_11)
	@echo '   [Compile] src/deps/mpr/mprSsl.c'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

ifeq ($(BIT_PACK_SSL),1)
#
#   libmprssl
#
DEPS_12 += $(CONFIG)/bin/libmpr.dylib
ifeq ($(BIT_PACK_EST),1)
    DEPS_12 += $(CONFIG)/bin/libest.dylib
endif
DEPS_12 += $(CONFIG)/obj/mprSsl.o

ifeq ($(BIT_PACK_EST),1)
    LIBS_12 += -lest
endif
LIBS_12 += -lmpr

$(CONFIG)/bin/libmprssl.dylib: $(DEPS_12)
	@echo '      [Link] libmprssl'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmprssl.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/mprSsl.o $(LIBS_12) $(LIBS_12) $(LIBS) 
endif

#
#   makerom.o
#
DEPS_13 += $(CONFIG)/inc/bit.h
DEPS_13 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c $(DEPS_13)
	@echo '   [Compile] src/deps/mpr/makerom.c'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

#
#   makerom
#
DEPS_14 += $(CONFIG)/bin/libmpr.dylib
DEPS_14 += $(CONFIG)/obj/makerom.o

LIBS_14 += -lmpr

$(CONFIG)/bin/makerom: $(DEPS_14)
	@echo '      [Link] makerom'
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS_14) $(LIBS_14) $(LIBS) 

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_15)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

#
#   pcre.o
#
DEPS_16 += $(CONFIG)/inc/bit.h
DEPS_16 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c $(DEPS_16)
	@echo '   [Compile] src/deps/pcre/pcre.c'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

#
#   libpcre
#
DEPS_17 += $(CONFIG)/inc/pcre.h
DEPS_17 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.dylib: $(DEPS_17)
	@echo '      [Link] libpcre'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib $(LDFLAGS) -compatibility_version 0.8.5 -current_version 0.8.5 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/pcre.o $(LIBS) 

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_18)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

#
#   httpLib.o
#
DEPS_19 += $(CONFIG)/inc/bit.h
DEPS_19 += $(CONFIG)/inc/http.h
DEPS_19 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c $(DEPS_19)
	@echo '   [Compile] src/deps/http/httpLib.c'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

#
#   libhttp
#
DEPS_20 += $(CONFIG)/bin/libmpr.dylib
DEPS_20 += $(CONFIG)/bin/libpcre.dylib
DEPS_20 += $(CONFIG)/inc/http.h
DEPS_20 += $(CONFIG)/obj/httpLib.o

LIBS_20 += -lpcre
LIBS_20 += -lmpr

$(CONFIG)/bin/libhttp.dylib: $(DEPS_20)
	@echo '      [Link] libhttp'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/httpLib.o $(LIBS_20) $(LIBS_20) $(LIBS) 

#
#   http.o
#
DEPS_21 += $(CONFIG)/inc/bit.h
DEPS_21 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c $(DEPS_21)
	@echo '   [Compile] src/deps/http/http.c'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

#
#   http
#
DEPS_22 += $(CONFIG)/bin/libhttp.dylib
DEPS_22 += $(CONFIG)/obj/http.o

LIBS_22 += -lhttp
LIBS_22 += -lpcre
LIBS_22 += -lmpr

$(CONFIG)/bin/http: $(DEPS_22)
	@echo '      [Link] http'
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS_22) $(LIBS_22) $(LIBS) 

#
#   ejs.h
#
$(CONFIG)/inc/ejs.h: $(DEPS_23)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.h" "$(CONFIG)/inc/ejs.h"

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_24)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

#
#   ejsByteGoto.h
#
$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_25)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/ejs/ejsByteGoto.h" "$(CONFIG)/inc/ejsByteGoto.h"

#
#   ejsLib.o
#
DEPS_26 += $(CONFIG)/inc/bit.h
DEPS_26 += $(CONFIG)/inc/ejs.h
DEPS_26 += $(CONFIG)/inc/mpr.h
DEPS_26 += $(CONFIG)/inc/pcre.h
DEPS_26 += $(CONFIG)/inc/bitos.h
DEPS_26 += $(CONFIG)/inc/http.h
DEPS_26 += $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/obj/ejsLib.o: \
    src/deps/ejs/ejsLib.c $(DEPS_26)
	@echo '   [Compile] src/deps/ejs/ejsLib.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsLib.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   libejs
#
DEPS_27 += $(CONFIG)/bin/libhttp.dylib
DEPS_27 += $(CONFIG)/bin/libpcre.dylib
DEPS_27 += $(CONFIG)/bin/libmpr.dylib
DEPS_27 += $(CONFIG)/inc/ejs.h
DEPS_27 += $(CONFIG)/inc/ejs.slots.h
DEPS_27 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_27 += $(CONFIG)/obj/ejsLib.o

LIBS_27 += -lmpr
LIBS_27 += -lpcre
LIBS_27 += -lhttp
LIBS_27 += -lpcre
LIBS_27 += -lmpr
ifeq ($(BIT_PACK_EST),1)
    LIBS_27 += -lest
endif

$(CONFIG)/bin/libejs.dylib: $(DEPS_27)
	@echo '      [Link] libejs'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libejs.dylib -compatibility_version 0.8.5 -current_version 0.8.5 $(CONFIG)/obj/ejsLib.o $(LIBS_27) $(LIBS_27) $(LIBS) 
endif

#
#   ejs.o
#
DEPS_28 += $(CONFIG)/inc/bit.h
DEPS_28 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/deps/ejs/ejs.c $(DEPS_28)
	@echo '   [Compile] src/deps/ejs/ejs.c'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejs.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs
#
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_29 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_29 += $(CONFIG)/obj/ejs.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_29 += -lejs
endif
LIBS_29 += -lmpr
LIBS_29 += -lpcre
LIBS_29 += -lhttp
ifeq ($(BIT_PACK_EST),1)
    LIBS_29 += -lest
endif

$(CONFIG)/bin/ejs: $(DEPS_29)
	@echo '      [Link] ejs'
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS_29) $(LIBS_29) $(LIBS) -ledit 
endif

#
#   ejsc.o
#
DEPS_30 += $(CONFIG)/inc/bit.h
DEPS_30 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/deps/ejs/ejsc.c $(DEPS_30)
	@echo '   [Compile] src/deps/ejs/ejsc.c'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/ejs/ejsc.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejsc
#
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_31 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_31 += $(CONFIG)/obj/ejsc.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_31 += -lejs
endif
LIBS_31 += -lmpr
LIBS_31 += -lpcre
LIBS_31 += -lhttp
ifeq ($(BIT_PACK_EST),1)
    LIBS_31 += -lest
endif

$(CONFIG)/bin/ejsc: $(DEPS_31)
	@echo '      [Link] ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS_31) $(LIBS_31) $(LIBS) 
endif

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs.mod
#
DEPS_32 += src/deps/ejs/ejs.es
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_32 += $(CONFIG)/bin/ejsc
endif

$(CONFIG)/bin/ejs.mod: $(DEPS_32)
	$(LBIN)/ejsc --out ./$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null src/deps/ejs/ejs.es
endif

#
#   bits
#
DEPS_33 += bits/bit.es
DEPS_33 += bits/configure.es
DEPS_33 += bits/embedthis-manifest.bit
DEPS_33 += bits/embedthis.bit
DEPS_33 += bits/embedthis.es
DEPS_33 += bits/gendoc.es
DEPS_33 += bits/generate.es
DEPS_33 += bits/os
DEPS_33 += bits/os/freebsd.bit
DEPS_33 += bits/os/gcc.bit
DEPS_33 += bits/os/linux.bit
DEPS_33 += bits/os/macosx.bit
DEPS_33 += bits/os/solaris.bit
DEPS_33 += bits/os/unix.bit
DEPS_33 += bits/os/vxworks.bit
DEPS_33 += bits/os/windows.bit
DEPS_33 += bits/packs
DEPS_33 += bits/packs/compiler.bit
DEPS_33 += bits/packs/doxygen.bit
DEPS_33 += bits/packs/dsi.bit
DEPS_33 += bits/packs/dumpbin.bit
DEPS_33 += bits/packs/ejs.bit
DEPS_33 += bits/packs/ejscript.bit
DEPS_33 += bits/packs/est.bit
DEPS_33 += bits/packs/http.bit
DEPS_33 += bits/packs/lib.bit
DEPS_33 += bits/packs/link.bit
DEPS_33 += bits/packs/man.bit
DEPS_33 += bits/packs/man2html.bit
DEPS_33 += bits/packs/matrixssl.bit
DEPS_33 += bits/packs/md5.bit
DEPS_33 += bits/packs/nanossl.bit
DEPS_33 += bits/packs/openssl.bit
DEPS_33 += bits/packs/pcre.bit
DEPS_33 += bits/packs/pmaker.bit
DEPS_33 += bits/packs/ranlib.bit
DEPS_33 += bits/packs/rc.bit
DEPS_33 += bits/packs/sqlite.bit
DEPS_33 += bits/packs/ssl.bit
DEPS_33 += bits/packs/strip.bit
DEPS_33 += bits/packs/tidy.bit
DEPS_33 += bits/packs/utest.bit
DEPS_33 += bits/packs/vxworks.bit
DEPS_33 += bits/packs/winsdk.bit
DEPS_33 += bits/packs/zip.bit
DEPS_33 += bits/packs/zlib.bit
DEPS_33 += bits/sample-main.bit
DEPS_33 += bits/sample-start.bit
DEPS_33 += bits/simple.bit
DEPS_33 += bits/standard.bit
DEPS_33 += bits/vstudio.es
DEPS_33 += bits/xcode.es

$(CONFIG)/bin/bits: $(DEPS_33)
	@echo '      [Copy] $(CONFIG)/bin/bits'
	mkdir -p "$(CONFIG)/bin/bits"
	cp "bits/bit.es" "$(CONFIG)/bin/bits/bit.es"
	cp "bits/configure.es" "$(CONFIG)/bin/bits/configure.es"
	cp "bits/embedthis-manifest.bit" "$(CONFIG)/bin/bits/embedthis-manifest.bit"
	cp "bits/embedthis.bit" "$(CONFIG)/bin/bits/embedthis.bit"
	cp "bits/embedthis.es" "$(CONFIG)/bin/bits/embedthis.es"
	cp "bits/gendoc.es" "$(CONFIG)/bin/bits/gendoc.es"
	cp "bits/generate.es" "$(CONFIG)/bin/bits/generate.es"
	mkdir -p "$(CONFIG)/bin/bits/os"
	cp "bits/os/freebsd.bit" "$(CONFIG)/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "$(CONFIG)/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "$(CONFIG)/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "$(CONFIG)/bin/bits/os/macosx.bit"
	cp "bits/os/solaris.bit" "$(CONFIG)/bin/bits/os/solaris.bit"
	cp "bits/os/unix.bit" "$(CONFIG)/bin/bits/os/unix.bit"
	cp "bits/os/vxworks.bit" "$(CONFIG)/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "$(CONFIG)/bin/bits/os/windows.bit"
	mkdir -p "$(CONFIG)/bin/bits/os"
	cp "bits/os/freebsd.bit" "$(CONFIG)/bin/bits/os/freebsd.bit"
	cp "bits/os/gcc.bit" "$(CONFIG)/bin/bits/os/gcc.bit"
	cp "bits/os/linux.bit" "$(CONFIG)/bin/bits/os/linux.bit"
	cp "bits/os/macosx.bit" "$(CONFIG)/bin/bits/os/macosx.bit"
	cp "bits/os/solaris.bit" "$(CONFIG)/bin/bits/os/solaris.bit"
	cp "bits/os/unix.bit" "$(CONFIG)/bin/bits/os/unix.bit"
	cp "bits/os/vxworks.bit" "$(CONFIG)/bin/bits/os/vxworks.bit"
	cp "bits/os/windows.bit" "$(CONFIG)/bin/bits/os/windows.bit"
	mkdir -p "$(CONFIG)/bin/bits/packs"
	cp "bits/packs/compiler.bit" "$(CONFIG)/bin/bits/packs/compiler.bit"
	cp "bits/packs/doxygen.bit" "$(CONFIG)/bin/bits/packs/doxygen.bit"
	cp "bits/packs/dsi.bit" "$(CONFIG)/bin/bits/packs/dsi.bit"
	cp "bits/packs/dumpbin.bit" "$(CONFIG)/bin/bits/packs/dumpbin.bit"
	cp "bits/packs/ejs.bit" "$(CONFIG)/bin/bits/packs/ejs.bit"
	cp "bits/packs/ejscript.bit" "$(CONFIG)/bin/bits/packs/ejscript.bit"
	cp "bits/packs/est.bit" "$(CONFIG)/bin/bits/packs/est.bit"
	cp "bits/packs/http.bit" "$(CONFIG)/bin/bits/packs/http.bit"
	cp "bits/packs/lib.bit" "$(CONFIG)/bin/bits/packs/lib.bit"
	cp "bits/packs/link.bit" "$(CONFIG)/bin/bits/packs/link.bit"
	cp "bits/packs/man.bit" "$(CONFIG)/bin/bits/packs/man.bit"
	cp "bits/packs/man2html.bit" "$(CONFIG)/bin/bits/packs/man2html.bit"
	cp "bits/packs/matrixssl.bit" "$(CONFIG)/bin/bits/packs/matrixssl.bit"
	cp "bits/packs/md5.bit" "$(CONFIG)/bin/bits/packs/md5.bit"
	cp "bits/packs/nanossl.bit" "$(CONFIG)/bin/bits/packs/nanossl.bit"
	cp "bits/packs/openssl.bit" "$(CONFIG)/bin/bits/packs/openssl.bit"
	cp "bits/packs/pcre.bit" "$(CONFIG)/bin/bits/packs/pcre.bit"
	cp "bits/packs/pmaker.bit" "$(CONFIG)/bin/bits/packs/pmaker.bit"
	cp "bits/packs/ranlib.bit" "$(CONFIG)/bin/bits/packs/ranlib.bit"
	cp "bits/packs/rc.bit" "$(CONFIG)/bin/bits/packs/rc.bit"
	cp "bits/packs/sqlite.bit" "$(CONFIG)/bin/bits/packs/sqlite.bit"
	cp "bits/packs/ssl.bit" "$(CONFIG)/bin/bits/packs/ssl.bit"
	cp "bits/packs/strip.bit" "$(CONFIG)/bin/bits/packs/strip.bit"
	cp "bits/packs/tidy.bit" "$(CONFIG)/bin/bits/packs/tidy.bit"
	cp "bits/packs/utest.bit" "$(CONFIG)/bin/bits/packs/utest.bit"
	cp "bits/packs/vxworks.bit" "$(CONFIG)/bin/bits/packs/vxworks.bit"
	cp "bits/packs/winsdk.bit" "$(CONFIG)/bin/bits/packs/winsdk.bit"
	cp "bits/packs/zip.bit" "$(CONFIG)/bin/bits/packs/zip.bit"
	cp "bits/packs/zlib.bit" "$(CONFIG)/bin/bits/packs/zlib.bit"
	mkdir -p "$(CONFIG)/bin/bits/packs"
	cp "bits/packs/compiler.bit" "$(CONFIG)/bin/bits/packs/compiler.bit"
	cp "bits/packs/doxygen.bit" "$(CONFIG)/bin/bits/packs/doxygen.bit"
	cp "bits/packs/dsi.bit" "$(CONFIG)/bin/bits/packs/dsi.bit"
	cp "bits/packs/dumpbin.bit" "$(CONFIG)/bin/bits/packs/dumpbin.bit"
	cp "bits/packs/ejs.bit" "$(CONFIG)/bin/bits/packs/ejs.bit"
	cp "bits/packs/ejscript.bit" "$(CONFIG)/bin/bits/packs/ejscript.bit"
	cp "bits/packs/est.bit" "$(CONFIG)/bin/bits/packs/est.bit"
	cp "bits/packs/http.bit" "$(CONFIG)/bin/bits/packs/http.bit"
	cp "bits/packs/lib.bit" "$(CONFIG)/bin/bits/packs/lib.bit"
	cp "bits/packs/link.bit" "$(CONFIG)/bin/bits/packs/link.bit"
	cp "bits/packs/man.bit" "$(CONFIG)/bin/bits/packs/man.bit"
	cp "bits/packs/man2html.bit" "$(CONFIG)/bin/bits/packs/man2html.bit"
	cp "bits/packs/matrixssl.bit" "$(CONFIG)/bin/bits/packs/matrixssl.bit"
	cp "bits/packs/md5.bit" "$(CONFIG)/bin/bits/packs/md5.bit"
	cp "bits/packs/nanossl.bit" "$(CONFIG)/bin/bits/packs/nanossl.bit"
	cp "bits/packs/openssl.bit" "$(CONFIG)/bin/bits/packs/openssl.bit"
	cp "bits/packs/pcre.bit" "$(CONFIG)/bin/bits/packs/pcre.bit"
	cp "bits/packs/pmaker.bit" "$(CONFIG)/bin/bits/packs/pmaker.bit"
	cp "bits/packs/ranlib.bit" "$(CONFIG)/bin/bits/packs/ranlib.bit"
	cp "bits/packs/rc.bit" "$(CONFIG)/bin/bits/packs/rc.bit"
	cp "bits/packs/sqlite.bit" "$(CONFIG)/bin/bits/packs/sqlite.bit"
	cp "bits/packs/ssl.bit" "$(CONFIG)/bin/bits/packs/ssl.bit"
	cp "bits/packs/strip.bit" "$(CONFIG)/bin/bits/packs/strip.bit"
	cp "bits/packs/tidy.bit" "$(CONFIG)/bin/bits/packs/tidy.bit"
	cp "bits/packs/utest.bit" "$(CONFIG)/bin/bits/packs/utest.bit"
	cp "bits/packs/vxworks.bit" "$(CONFIG)/bin/bits/packs/vxworks.bit"
	cp "bits/packs/winsdk.bit" "$(CONFIG)/bin/bits/packs/winsdk.bit"
	cp "bits/packs/zip.bit" "$(CONFIG)/bin/bits/packs/zip.bit"
	cp "bits/packs/zlib.bit" "$(CONFIG)/bin/bits/packs/zlib.bit"
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
DEPS_35 += $(CONFIG)/inc/bitos.h
DEPS_35 += $(CONFIG)/obj/bit.o

ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_35 += -lejs
endif
LIBS_35 += -lhttp
LIBS_35 += -lmpr
LIBS_35 += -lpcre
ifeq ($(BIT_PACK_EST),1)
    LIBS_35 += -lest
endif

$(CONFIG)/bin/bit: $(DEPS_35)
	@echo '      [Link] bit'
	$(CC) -o $(CONFIG)/bin/bit -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/bit.o $(LIBS_35) $(LIBS_35) $(LIBS) 

#
#   stop
#
stop: $(DEPS_36)

#
#   installBinary
#
DEPS_37 += stop

installBinary: $(DEPS_37)
	mkdir -p "$(BIT_APP_PREFIX)"
	mkdir -p "$(BIT_VAPP_PREFIX)"
	mkdir -p "$(BIT_APP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	ln -s "0.8.5" "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin"
	cp "$(CONFIG)/bin/bit" "$(BIT_VAPP_PREFIX)/bin/bit"
	mkdir -p "$(BIT_BIN_PREFIX)"
	rm -f "$(BIT_BIN_PREFIX)/bit"
	ln -s "$(BIT_VAPP_PREFIX)/bin/bit" "$(BIT_BIN_PREFIX)/bit"
	cp "$(CONFIG)/bin/ca.crt" "$(BIT_VAPP_PREFIX)/bin/ca.crt"
	cp "$(CONFIG)/bin/ejs.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.mod"
	cp "$(CONFIG)/bin/libejs.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.dylib"
	cp "$(CONFIG)/bin/libest.dylib" "$(BIT_VAPP_PREFIX)/bin/libest.dylib"
	cp "$(CONFIG)/bin/libhttp.dylib" "$(BIT_VAPP_PREFIX)/bin/libhttp.dylib"
	cp "$(CONFIG)/bin/libmpr.dylib" "$(BIT_VAPP_PREFIX)/bin/libmpr.dylib"
	cp "$(CONFIG)/bin/libmprssl.dylib" "$(BIT_VAPP_PREFIX)/bin/libmprssl.dylib"
	cp "$(CONFIG)/bin/libpcre.dylib" "$(BIT_VAPP_PREFIX)/bin/libpcre.dylib"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits"
	cp "$(CONFIG)/bin/bits/**" "$(BIT_VAPP_PREFIX)/bin/bits/**"
	mkdir -p "$(BIT_VAPP_PREFIX)/doc/man/man1"
	cp "doc/man/bit.1" "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1"
	mkdir -p "$(BIT_MAN_PREFIX)/man1"
	rm -f "$(BIT_MAN_PREFIX)/man1/bit.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1" "$(BIT_MAN_PREFIX)/man1/bit.1"

#
#   start
#
start: $(DEPS_38)

#
#   install
#
DEPS_39 += stop
DEPS_39 += installBinary
DEPS_39 += start

install: $(DEPS_39)
	

#
#   uninstall
#
DEPS_40 += stop

uninstall: $(DEPS_40)
	rm -fr "$(BIT_VAPP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	rmdir -p "$(BIT_APP_PREFIX)" 2>/dev/null ; true

