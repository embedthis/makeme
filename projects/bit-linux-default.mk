#
#   bit-linux-default.mk -- Makefile to build Embedthis Bit for linux
#

PRODUCT            := bit
VERSION            := 0.9.4
PROFILE            := default
ARCH               := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH            := $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                 := linux
CC                 := gcc
LD                 := link
CONFIG             := $(OS)-$(ARCH)-$(PROFILE)
LBIN               := $(CONFIG)/bin

BIT_PACK_EJSCRIPT  := 1
BIT_PACK_EST       := 1
BIT_PACK_PCRE      := 1
BIT_PACK_SSL       := 1
BIT_PACK_ZLIB      := 1

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

BIT_PACK_COMPILER_PATH    := gcc
BIT_PACK_DSI_PATH         := dsi
BIT_PACK_EJSCRIPT_PATH    := ejscript
BIT_PACK_EST_PATH         := est
BIT_PACK_LIB_PATH         := ar
BIT_PACK_LINK_PATH        := link
BIT_PACK_MAN_PATH         := man
BIT_PACK_MAN2HTML_PATH    := man2html
BIT_PACK_MATRIXSSL_PATH   := /usr/src/matrixssl
BIT_PACK_NANOSSL_PATH     := /usr/src/nanossl
BIT_PACK_OPENSSL_PATH     := /usr/src/openssl
BIT_PACK_PCRE_PATH        := pcre
BIT_PACK_PMAKER_PATH      := pmaker
BIT_PACK_SSL_PATH         := ssl
BIT_PACK_ZIP_PATH         := zip
BIT_PACK_ZLIB_PATH        := zlib

CFLAGS             += -fPIC -w
DFLAGS             += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS))) -DBIT_PACK_EJSCRIPT=$(BIT_PACK_EJSCRIPT) -DBIT_PACK_EST=$(BIT_PACK_EST) -DBIT_PACK_PCRE=$(BIT_PACK_PCRE) -DBIT_PACK_SSL=$(BIT_PACK_SSL) -DBIT_PACK_ZLIB=$(BIT_PACK_ZLIB) 
IFLAGS             += "-I$(CONFIG)/inc"
LDFLAGS            += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS           += -L$(CONFIG)/bin
LIBS               += -lrt -ldl -lpthread -lm

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
TARGETS            += $(CONFIG)/bin/libest.so
endif
TARGETS            += $(CONFIG)/bin/ca.crt
TARGETS            += $(CONFIG)/bin/libmpr.so
TARGETS            += $(CONFIG)/bin/libmprssl.so
TARGETS            += $(CONFIG)/bin/makerom
ifeq ($(BIT_PACK_PCRE),1)
TARGETS            += $(CONFIG)/bin/libpcre.so
endif
ifeq ($(BIT_PACK_ZLIB),1)
TARGETS            += $(CONFIG)/bin/libzlib.so
endif
TARGETS            += $(CONFIG)/bin/libhttp.so
TARGETS            += $(CONFIG)/bin/http
ifeq ($(BIT_PACK_EJSCRIPT),1)
TARGETS            += $(CONFIG)/bin/libejs.so
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
TARGETS            += bower.json

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
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bitos.h src/bitos.h >/dev/null ; then\
		cp src/bitos.h $(CONFIG)/inc/bitos.h  ; \
	fi; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/bit-linux-default-bit.h $(CONFIG)/inc/bit.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/bit-linux-default-bit.h >/dev/null ; then\
		cp projects/bit-linux-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags

clean:
	rm -f "$(CONFIG)/bin/libest.so"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libmpr.so"
	rm -f "$(CONFIG)/bin/libmprssl.so"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libpcre.so"
	rm -f "$(CONFIG)/bin/libzlib.so"
	rm -f "$(CONFIG)/bin/libhttp.so"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libejs.so"
	rm -f "$(CONFIG)/bin/ejs"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "bower.json"
	rm -f "$(CONFIG)/obj/estLib.o"
	rm -f "$(CONFIG)/obj/mprLib.o"
	rm -f "$(CONFIG)/obj/mprSsl.o"
	rm -f "$(CONFIG)/obj/makerom.o"
	rm -f "$(CONFIG)/obj/pcre.o"
	rm -f "$(CONFIG)/obj/zlib.o"
	rm -f "$(CONFIG)/obj/httpLib.o"
	rm -f "$(CONFIG)/obj/http.o"
	rm -f "$(CONFIG)/obj/ejsLib.o"
	rm -f "$(CONFIG)/obj/ejs.o"
	rm -f "$(CONFIG)/obj/ejsc.o"
	rm -f "$(CONFIG)/obj/bit.o"

clobber: clean
	rm -fr ./$(CONFIG)



#
#   version
#
version: $(DEPS_1)
	( \
	cd bits; \
	echo 0.9.4 ; \
	)

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

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
	cp src/bitos.h $(CONFIG)/inc/bitos.h

#
#   estLib.o
#
DEPS_5 += $(CONFIG)/inc/bit.h
DEPS_5 += $(CONFIG)/inc/est.h
DEPS_5 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_5)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(BIT_PACK_EST),1)
#
#   libest
#
DEPS_6 += $(CONFIG)/inc/est.h
DEPS_6 += $(CONFIG)/inc/bit.h
DEPS_6 += $(CONFIG)/inc/bitos.h
DEPS_6 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.so: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libest.so'
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   ca-crt
#
DEPS_7 += src/paks/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp src/paks/est/ca.crt $(CONFIG)/bin/ca.crt

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_8)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h $(CONFIG)/inc/mpr.h

#
#   mprLib.o
#
DEPS_9 += $(CONFIG)/inc/bit.h
DEPS_9 += $(CONFIG)/inc/mpr.h
DEPS_9 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_9)
	@echo '   [Compile] $(CONFIG)/obj/mprLib.o'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_10 += $(CONFIG)/inc/mpr.h
DEPS_10 += $(CONFIG)/inc/bit.h
DEPS_10 += $(CONFIG)/inc/bitos.h
DEPS_10 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.so: $(DEPS_10)
	@echo '      [Link] $(CONFIG)/bin/libmpr.so'
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/mprLib.o" $(LIBS) 

#
#   mprSsl.o
#
DEPS_11 += $(CONFIG)/inc/bit.h
DEPS_11 += $(CONFIG)/inc/mpr.h
DEPS_11 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_11)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/bit.h
DEPS_12 += $(CONFIG)/inc/bitos.h
DEPS_12 += $(CONFIG)/obj/mprLib.o
DEPS_12 += $(CONFIG)/bin/libmpr.so
DEPS_12 += $(CONFIG)/inc/est.h
DEPS_12 += $(CONFIG)/obj/estLib.o
ifeq ($(BIT_PACK_EST),1)
    DEPS_12 += $(CONFIG)/bin/libest.so
endif
DEPS_12 += $(CONFIG)/obj/mprSsl.o

LIBS_12 += -lmpr
ifeq ($(BIT_PACK_EST),1)
    LIBS_12 += -lest
endif

$(CONFIG)/bin/libmprssl.so: $(DEPS_12)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.so'
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_12) $(LIBS_12) $(LIBS_12) $(LIBS) 

#
#   makerom.o
#
DEPS_13 += $(CONFIG)/inc/bit.h
DEPS_13 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/paks/mpr/makerom.c $(DEPS_13)
	@echo '   [Compile] $(CONFIG)/obj/makerom.o'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/makerom.c

#
#   makerom
#
DEPS_14 += $(CONFIG)/inc/mpr.h
DEPS_14 += $(CONFIG)/inc/bit.h
DEPS_14 += $(CONFIG)/inc/bitos.h
DEPS_14 += $(CONFIG)/obj/mprLib.o
DEPS_14 += $(CONFIG)/bin/libmpr.so
DEPS_14 += $(CONFIG)/obj/makerom.o

LIBS_14 += -lmpr

$(CONFIG)/bin/makerom: $(DEPS_14)
	@echo '      [Link] $(CONFIG)/bin/makerom'
	$(CC) -o $(CONFIG)/bin/makerom $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/makerom.o" $(LIBPATHS_14) $(LIBS_14) $(LIBS_14) $(LIBS) $(LIBS) 

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_15)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h $(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_16 += $(CONFIG)/inc/bit.h
DEPS_16 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_16)
	@echo '   [Compile] $(CONFIG)/obj/pcre.o'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(BIT_PACK_PCRE),1)
#
#   libpcre
#
DEPS_17 += $(CONFIG)/inc/pcre.h
DEPS_17 += $(CONFIG)/inc/bit.h
DEPS_17 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.so: $(DEPS_17)
	@echo '      [Link] $(CONFIG)/bin/libpcre.so'
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/pcre.o" $(LIBS) 
endif

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_18)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h $(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_19 += $(CONFIG)/inc/bit.h
DEPS_19 += $(CONFIG)/inc/zlib.h
DEPS_19 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_19)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(BIT_PACK_ZLIB),1)
#
#   libzlib
#
DEPS_20 += $(CONFIG)/inc/zlib.h
DEPS_20 += $(CONFIG)/inc/bit.h
DEPS_20 += $(CONFIG)/inc/bitos.h
DEPS_20 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.so: $(DEPS_20)
	@echo '      [Link] $(CONFIG)/bin/libzlib.so'
	$(CC) -shared -o $(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_21)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/http/http.h $(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_22 += $(CONFIG)/inc/bit.h
DEPS_22 += $(CONFIG)/inc/http.h
DEPS_22 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_22)
	@echo '   [Compile] $(CONFIG)/obj/httpLib.o'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

#
#   libhttp
#
DEPS_23 += $(CONFIG)/inc/mpr.h
DEPS_23 += $(CONFIG)/inc/bit.h
DEPS_23 += $(CONFIG)/inc/bitos.h
DEPS_23 += $(CONFIG)/obj/mprLib.o
DEPS_23 += $(CONFIG)/bin/libmpr.so
DEPS_23 += $(CONFIG)/inc/pcre.h
DEPS_23 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_23 += $(CONFIG)/bin/libpcre.so
endif
DEPS_23 += $(CONFIG)/inc/http.h
DEPS_23 += $(CONFIG)/obj/httpLib.o

LIBS_23 += -lmpr
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_23 += -lpcre
endif

$(CONFIG)/bin/libhttp.so: $(DEPS_23)
	@echo '      [Link] $(CONFIG)/bin/libhttp.so'
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/httpLib.o" $(LIBPATHS_23) $(LIBS_23) $(LIBS_23) $(LIBS) 

#
#   http.o
#
DEPS_24 += $(CONFIG)/inc/bit.h
DEPS_24 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_24)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   http
#
DEPS_25 += $(CONFIG)/inc/mpr.h
DEPS_25 += $(CONFIG)/inc/bit.h
DEPS_25 += $(CONFIG)/inc/bitos.h
DEPS_25 += $(CONFIG)/obj/mprLib.o
DEPS_25 += $(CONFIG)/bin/libmpr.so
DEPS_25 += $(CONFIG)/inc/pcre.h
DEPS_25 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_25 += $(CONFIG)/bin/libpcre.so
endif
DEPS_25 += $(CONFIG)/inc/http.h
DEPS_25 += $(CONFIG)/obj/httpLib.o
DEPS_25 += $(CONFIG)/bin/libhttp.so
DEPS_25 += $(CONFIG)/obj/http.o

LIBS_25 += -lhttp
LIBS_25 += -lmpr
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_25 += -lpcre
endif

$(CONFIG)/bin/http: $(DEPS_25)
	@echo '      [Link] $(CONFIG)/bin/http'
	$(CC) -o $(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_25) $(LIBS_25) $(LIBS_25) $(LIBS) $(LIBS) 

#
#   ejs.h
#
$(CONFIG)/inc/ejs.h: $(DEPS_26)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.h $(CONFIG)/inc/ejs.h

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_27)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_28)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejsByteGoto.h $(CONFIG)/inc/ejsByteGoto.h

#
#   ejsLib.o
#
DEPS_29 += $(CONFIG)/inc/bit.h
DEPS_29 += $(CONFIG)/inc/ejs.h
DEPS_29 += $(CONFIG)/inc/mpr.h
DEPS_29 += $(CONFIG)/inc/pcre.h
DEPS_29 += $(CONFIG)/inc/bitos.h
DEPS_29 += $(CONFIG)/inc/http.h
DEPS_29 += $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_29)
	@echo '   [Compile] $(CONFIG)/obj/ejsLib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   libejs
#
DEPS_30 += $(CONFIG)/inc/mpr.h
DEPS_30 += $(CONFIG)/inc/bit.h
DEPS_30 += $(CONFIG)/inc/bitos.h
DEPS_30 += $(CONFIG)/obj/mprLib.o
DEPS_30 += $(CONFIG)/bin/libmpr.so
DEPS_30 += $(CONFIG)/inc/pcre.h
DEPS_30 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_30 += $(CONFIG)/bin/libpcre.so
endif
DEPS_30 += $(CONFIG)/inc/http.h
DEPS_30 += $(CONFIG)/obj/httpLib.o
DEPS_30 += $(CONFIG)/bin/libhttp.so
DEPS_30 += $(CONFIG)/inc/zlib.h
DEPS_30 += $(CONFIG)/obj/zlib.o
ifeq ($(BIT_PACK_ZLIB),1)
    DEPS_30 += $(CONFIG)/bin/libzlib.so
endif
DEPS_30 += $(CONFIG)/inc/ejs.h
DEPS_30 += $(CONFIG)/inc/ejs.slots.h
DEPS_30 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_30 += $(CONFIG)/obj/ejsLib.o

LIBS_30 += -lhttp
LIBS_30 += -lmpr
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_30 += -lpcre
endif
ifeq ($(BIT_PACK_ZLIB),1)
    LIBS_30 += -lzlib
endif

$(CONFIG)/bin/libejs.so: $(DEPS_30)
	@echo '      [Link] $(CONFIG)/bin/libejs.so'
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_30) $(LIBS_30) $(LIBS_30) $(LIBS) 
endif

#
#   ejs.o
#
DEPS_31 += $(CONFIG)/inc/bit.h
DEPS_31 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_31)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs
#
DEPS_32 += $(CONFIG)/inc/mpr.h
DEPS_32 += $(CONFIG)/inc/bit.h
DEPS_32 += $(CONFIG)/inc/bitos.h
DEPS_32 += $(CONFIG)/obj/mprLib.o
DEPS_32 += $(CONFIG)/bin/libmpr.so
DEPS_32 += $(CONFIG)/inc/pcre.h
DEPS_32 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_32 += $(CONFIG)/bin/libpcre.so
endif
DEPS_32 += $(CONFIG)/inc/http.h
DEPS_32 += $(CONFIG)/obj/httpLib.o
DEPS_32 += $(CONFIG)/bin/libhttp.so
DEPS_32 += $(CONFIG)/inc/zlib.h
DEPS_32 += $(CONFIG)/obj/zlib.o
ifeq ($(BIT_PACK_ZLIB),1)
    DEPS_32 += $(CONFIG)/bin/libzlib.so
endif
DEPS_32 += $(CONFIG)/inc/ejs.h
DEPS_32 += $(CONFIG)/inc/ejs.slots.h
DEPS_32 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_32 += $(CONFIG)/obj/ejsLib.o
DEPS_32 += $(CONFIG)/bin/libejs.so
DEPS_32 += $(CONFIG)/obj/ejs.o

LIBS_32 += -lejs
LIBS_32 += -lhttp
LIBS_32 += -lmpr
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_32 += -lpcre
endif
ifeq ($(BIT_PACK_ZLIB),1)
    LIBS_32 += -lzlib
endif

$(CONFIG)/bin/ejs: $(DEPS_32)
	@echo '      [Link] $(CONFIG)/bin/ejs'
	$(CC) -o $(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_32) $(LIBS_32) $(LIBS_32) $(LIBS) $(LIBS) 
endif

#
#   ejsc.o
#
DEPS_33 += $(CONFIG)/inc/bit.h
DEPS_33 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_33)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejsc
#
DEPS_34 += $(CONFIG)/inc/mpr.h
DEPS_34 += $(CONFIG)/inc/bit.h
DEPS_34 += $(CONFIG)/inc/bitos.h
DEPS_34 += $(CONFIG)/obj/mprLib.o
DEPS_34 += $(CONFIG)/bin/libmpr.so
DEPS_34 += $(CONFIG)/inc/pcre.h
DEPS_34 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_34 += $(CONFIG)/bin/libpcre.so
endif
DEPS_34 += $(CONFIG)/inc/http.h
DEPS_34 += $(CONFIG)/obj/httpLib.o
DEPS_34 += $(CONFIG)/bin/libhttp.so
DEPS_34 += $(CONFIG)/inc/zlib.h
DEPS_34 += $(CONFIG)/obj/zlib.o
ifeq ($(BIT_PACK_ZLIB),1)
    DEPS_34 += $(CONFIG)/bin/libzlib.so
endif
DEPS_34 += $(CONFIG)/inc/ejs.h
DEPS_34 += $(CONFIG)/inc/ejs.slots.h
DEPS_34 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_34 += $(CONFIG)/obj/ejsLib.o
DEPS_34 += $(CONFIG)/bin/libejs.so
DEPS_34 += $(CONFIG)/obj/ejsc.o

LIBS_34 += -lejs
LIBS_34 += -lhttp
LIBS_34 += -lmpr
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_34 += -lpcre
endif
ifeq ($(BIT_PACK_ZLIB),1)
    LIBS_34 += -lzlib
endif

$(CONFIG)/bin/ejsc: $(DEPS_34)
	@echo '      [Link] $(CONFIG)/bin/ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_34) $(LIBS_34) $(LIBS_34) $(LIBS) $(LIBS) 
endif

ifeq ($(BIT_PACK_EJSCRIPT),1)
#
#   ejs.mod
#
DEPS_35 += src/paks/ejs/ejs.es
DEPS_35 += $(CONFIG)/inc/mpr.h
DEPS_35 += $(CONFIG)/inc/bit.h
DEPS_35 += $(CONFIG)/inc/bitos.h
DEPS_35 += $(CONFIG)/obj/mprLib.o
DEPS_35 += $(CONFIG)/bin/libmpr.so
DEPS_35 += $(CONFIG)/inc/pcre.h
DEPS_35 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_35 += $(CONFIG)/bin/libpcre.so
endif
DEPS_35 += $(CONFIG)/inc/http.h
DEPS_35 += $(CONFIG)/obj/httpLib.o
DEPS_35 += $(CONFIG)/bin/libhttp.so
DEPS_35 += $(CONFIG)/inc/zlib.h
DEPS_35 += $(CONFIG)/obj/zlib.o
ifeq ($(BIT_PACK_ZLIB),1)
    DEPS_35 += $(CONFIG)/bin/libzlib.so
endif
DEPS_35 += $(CONFIG)/inc/ejs.h
DEPS_35 += $(CONFIG)/inc/ejs.slots.h
DEPS_35 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_35 += $(CONFIG)/obj/ejsLib.o
DEPS_35 += $(CONFIG)/bin/libejs.so
DEPS_35 += $(CONFIG)/obj/ejsc.o
DEPS_35 += $(CONFIG)/bin/ejsc

$(CONFIG)/bin/ejs.mod: $(DEPS_35)
	( \
	cd src/paks/ejs; \
	../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   bits
#
DEPS_36 += bits/bit.es
DEPS_36 += bits/configure.es
DEPS_36 += bits/embedthis-manifest.bit
DEPS_36 += bits/embedthis.bit
DEPS_36 += bits/embedthis.es
DEPS_36 += bits/gendoc.es
DEPS_36 += bits/generate.es
DEPS_36 += bits/os
DEPS_36 += bits/os/freebsd.bit
DEPS_36 += bits/os/gcc.bit
DEPS_36 += bits/os/linux.bit
DEPS_36 += bits/os/macosx.bit
DEPS_36 += bits/os/solaris.bit
DEPS_36 += bits/os/unix.bit
DEPS_36 += bits/os/vxworks.bit
DEPS_36 += bits/os/windows.bit
DEPS_36 += bits/packs
DEPS_36 += bits/packs/appweb-embed.bit
DEPS_36 += bits/packs/appweb.bit
DEPS_36 += bits/packs/compiler.bit
DEPS_36 += bits/packs/doxygen.bit
DEPS_36 += bits/packs/dsi.bit
DEPS_36 += bits/packs/dumpbin.bit
DEPS_36 += bits/packs/ejs.bit
DEPS_36 += bits/packs/ejscript.bit
DEPS_36 += bits/packs/est.bit
DEPS_36 += bits/packs/gzip.bit
DEPS_36 += bits/packs/htmlmin.bit
DEPS_36 += bits/packs/http.bit
DEPS_36 += bits/packs/lib.bit
DEPS_36 += bits/packs/link.bit
DEPS_36 += bits/packs/man.bit
DEPS_36 += bits/packs/man2html.bit
DEPS_36 += bits/packs/matrixssl.bit
DEPS_36 += bits/packs/md5.bit
DEPS_36 += bits/packs/nanossl.bit
DEPS_36 += bits/packs/ngmin.bit
DEPS_36 += bits/packs/openssl.bit
DEPS_36 += bits/packs/pak.bit
DEPS_36 += bits/packs/pcre.bit
DEPS_36 += bits/packs/pmaker.bit
DEPS_36 += bits/packs/ranlib.bit
DEPS_36 += bits/packs/rc.bit
DEPS_36 += bits/packs/recess.bit
DEPS_36 += bits/packs/sqlite.bit
DEPS_36 += bits/packs/ssl.bit
DEPS_36 += bits/packs/strip.bit
DEPS_36 += bits/packs/tidy.bit
DEPS_36 += bits/packs/uglifyjs.bit
DEPS_36 += bits/packs/utest.bit
DEPS_36 += bits/packs/vxworks.bit
DEPS_36 += bits/packs/winsdk.bit
DEPS_36 += bits/packs/zip.bit
DEPS_36 += bits/packs/zlib.bit
DEPS_36 += bits/sample-main.bit
DEPS_36 += bits/sample-start.bit
DEPS_36 += bits/simple.bit
DEPS_36 += bits/standard.bit
DEPS_36 += bits/vstudio.es
DEPS_36 += bits/xcode.es

$(CONFIG)/bin/bits: $(DEPS_36)
	@echo '      [Copy] $(CONFIG)/bin/bits'
	mkdir -p "$(CONFIG)/bin/bits"
	cp bits/bit.es $(CONFIG)/bin/bits/bit.es
	cp bits/configure.es $(CONFIG)/bin/bits/configure.es
	cp bits/embedthis-manifest.bit $(CONFIG)/bin/bits/embedthis-manifest.bit
	cp bits/embedthis.bit $(CONFIG)/bin/bits/embedthis.bit
	cp bits/embedthis.es $(CONFIG)/bin/bits/embedthis.es
	cp bits/gendoc.es $(CONFIG)/bin/bits/gendoc.es
	cp bits/generate.es $(CONFIG)/bin/bits/generate.es
	mkdir -p "$(CONFIG)/bin/bits/os"
	cp bits/os/freebsd.bit $(CONFIG)/bin/bits/os/freebsd.bit
	cp bits/os/gcc.bit $(CONFIG)/bin/bits/os/gcc.bit
	cp bits/os/linux.bit $(CONFIG)/bin/bits/os/linux.bit
	cp bits/os/macosx.bit $(CONFIG)/bin/bits/os/macosx.bit
	cp bits/os/solaris.bit $(CONFIG)/bin/bits/os/solaris.bit
	cp bits/os/unix.bit $(CONFIG)/bin/bits/os/unix.bit
	cp bits/os/vxworks.bit $(CONFIG)/bin/bits/os/vxworks.bit
	cp bits/os/windows.bit $(CONFIG)/bin/bits/os/windows.bit
	mkdir -p "$(CONFIG)/bin/bits/os"
	cp bits/os/freebsd.bit $(CONFIG)/bin/bits/os/freebsd.bit
	cp bits/os/gcc.bit $(CONFIG)/bin/bits/os/gcc.bit
	cp bits/os/linux.bit $(CONFIG)/bin/bits/os/linux.bit
	cp bits/os/macosx.bit $(CONFIG)/bin/bits/os/macosx.bit
	cp bits/os/solaris.bit $(CONFIG)/bin/bits/os/solaris.bit
	cp bits/os/unix.bit $(CONFIG)/bin/bits/os/unix.bit
	cp bits/os/vxworks.bit $(CONFIG)/bin/bits/os/vxworks.bit
	cp bits/os/windows.bit $(CONFIG)/bin/bits/os/windows.bit
	mkdir -p "$(CONFIG)/bin/bits/packs"
	cp bits/packs/appweb-embed.bit $(CONFIG)/bin/bits/packs/appweb-embed.bit
	cp bits/packs/appweb.bit $(CONFIG)/bin/bits/packs/appweb.bit
	cp bits/packs/compiler.bit $(CONFIG)/bin/bits/packs/compiler.bit
	cp bits/packs/doxygen.bit $(CONFIG)/bin/bits/packs/doxygen.bit
	cp bits/packs/dsi.bit $(CONFIG)/bin/bits/packs/dsi.bit
	cp bits/packs/dumpbin.bit $(CONFIG)/bin/bits/packs/dumpbin.bit
	cp bits/packs/ejs.bit $(CONFIG)/bin/bits/packs/ejs.bit
	cp bits/packs/ejscript.bit $(CONFIG)/bin/bits/packs/ejscript.bit
	cp bits/packs/est.bit $(CONFIG)/bin/bits/packs/est.bit
	cp bits/packs/gzip.bit $(CONFIG)/bin/bits/packs/gzip.bit
	cp bits/packs/htmlmin.bit $(CONFIG)/bin/bits/packs/htmlmin.bit
	cp bits/packs/http.bit $(CONFIG)/bin/bits/packs/http.bit
	cp bits/packs/lib.bit $(CONFIG)/bin/bits/packs/lib.bit
	cp bits/packs/link.bit $(CONFIG)/bin/bits/packs/link.bit
	cp bits/packs/man.bit $(CONFIG)/bin/bits/packs/man.bit
	cp bits/packs/man2html.bit $(CONFIG)/bin/bits/packs/man2html.bit
	cp bits/packs/matrixssl.bit $(CONFIG)/bin/bits/packs/matrixssl.bit
	cp bits/packs/md5.bit $(CONFIG)/bin/bits/packs/md5.bit
	cp bits/packs/nanossl.bit $(CONFIG)/bin/bits/packs/nanossl.bit
	cp bits/packs/ngmin.bit $(CONFIG)/bin/bits/packs/ngmin.bit
	cp bits/packs/openssl.bit $(CONFIG)/bin/bits/packs/openssl.bit
	cp bits/packs/pak.bit $(CONFIG)/bin/bits/packs/pak.bit
	cp bits/packs/pcre.bit $(CONFIG)/bin/bits/packs/pcre.bit
	cp bits/packs/pmaker.bit $(CONFIG)/bin/bits/packs/pmaker.bit
	cp bits/packs/ranlib.bit $(CONFIG)/bin/bits/packs/ranlib.bit
	cp bits/packs/rc.bit $(CONFIG)/bin/bits/packs/rc.bit
	cp bits/packs/recess.bit $(CONFIG)/bin/bits/packs/recess.bit
	cp bits/packs/sqlite.bit $(CONFIG)/bin/bits/packs/sqlite.bit
	cp bits/packs/ssl.bit $(CONFIG)/bin/bits/packs/ssl.bit
	cp bits/packs/strip.bit $(CONFIG)/bin/bits/packs/strip.bit
	cp bits/packs/tidy.bit $(CONFIG)/bin/bits/packs/tidy.bit
	cp bits/packs/uglifyjs.bit $(CONFIG)/bin/bits/packs/uglifyjs.bit
	cp bits/packs/utest.bit $(CONFIG)/bin/bits/packs/utest.bit
	cp bits/packs/vxworks.bit $(CONFIG)/bin/bits/packs/vxworks.bit
	cp bits/packs/winsdk.bit $(CONFIG)/bin/bits/packs/winsdk.bit
	cp bits/packs/zip.bit $(CONFIG)/bin/bits/packs/zip.bit
	cp bits/packs/zlib.bit $(CONFIG)/bin/bits/packs/zlib.bit
	mkdir -p "$(CONFIG)/bin/bits/packs"
	cp bits/packs/appweb-embed.bit $(CONFIG)/bin/bits/packs/appweb-embed.bit
	cp bits/packs/appweb.bit $(CONFIG)/bin/bits/packs/appweb.bit
	cp bits/packs/compiler.bit $(CONFIG)/bin/bits/packs/compiler.bit
	cp bits/packs/doxygen.bit $(CONFIG)/bin/bits/packs/doxygen.bit
	cp bits/packs/dsi.bit $(CONFIG)/bin/bits/packs/dsi.bit
	cp bits/packs/dumpbin.bit $(CONFIG)/bin/bits/packs/dumpbin.bit
	cp bits/packs/ejs.bit $(CONFIG)/bin/bits/packs/ejs.bit
	cp bits/packs/ejscript.bit $(CONFIG)/bin/bits/packs/ejscript.bit
	cp bits/packs/est.bit $(CONFIG)/bin/bits/packs/est.bit
	cp bits/packs/gzip.bit $(CONFIG)/bin/bits/packs/gzip.bit
	cp bits/packs/htmlmin.bit $(CONFIG)/bin/bits/packs/htmlmin.bit
	cp bits/packs/http.bit $(CONFIG)/bin/bits/packs/http.bit
	cp bits/packs/lib.bit $(CONFIG)/bin/bits/packs/lib.bit
	cp bits/packs/link.bit $(CONFIG)/bin/bits/packs/link.bit
	cp bits/packs/man.bit $(CONFIG)/bin/bits/packs/man.bit
	cp bits/packs/man2html.bit $(CONFIG)/bin/bits/packs/man2html.bit
	cp bits/packs/matrixssl.bit $(CONFIG)/bin/bits/packs/matrixssl.bit
	cp bits/packs/md5.bit $(CONFIG)/bin/bits/packs/md5.bit
	cp bits/packs/nanossl.bit $(CONFIG)/bin/bits/packs/nanossl.bit
	cp bits/packs/ngmin.bit $(CONFIG)/bin/bits/packs/ngmin.bit
	cp bits/packs/openssl.bit $(CONFIG)/bin/bits/packs/openssl.bit
	cp bits/packs/pak.bit $(CONFIG)/bin/bits/packs/pak.bit
	cp bits/packs/pcre.bit $(CONFIG)/bin/bits/packs/pcre.bit
	cp bits/packs/pmaker.bit $(CONFIG)/bin/bits/packs/pmaker.bit
	cp bits/packs/ranlib.bit $(CONFIG)/bin/bits/packs/ranlib.bit
	cp bits/packs/rc.bit $(CONFIG)/bin/bits/packs/rc.bit
	cp bits/packs/recess.bit $(CONFIG)/bin/bits/packs/recess.bit
	cp bits/packs/sqlite.bit $(CONFIG)/bin/bits/packs/sqlite.bit
	cp bits/packs/ssl.bit $(CONFIG)/bin/bits/packs/ssl.bit
	cp bits/packs/strip.bit $(CONFIG)/bin/bits/packs/strip.bit
	cp bits/packs/tidy.bit $(CONFIG)/bin/bits/packs/tidy.bit
	cp bits/packs/uglifyjs.bit $(CONFIG)/bin/bits/packs/uglifyjs.bit
	cp bits/packs/utest.bit $(CONFIG)/bin/bits/packs/utest.bit
	cp bits/packs/vxworks.bit $(CONFIG)/bin/bits/packs/vxworks.bit
	cp bits/packs/winsdk.bit $(CONFIG)/bin/bits/packs/winsdk.bit
	cp bits/packs/zip.bit $(CONFIG)/bin/bits/packs/zip.bit
	cp bits/packs/zlib.bit $(CONFIG)/bin/bits/packs/zlib.bit
	cp bits/sample-main.bit $(CONFIG)/bin/bits/sample-main.bit
	cp bits/sample-start.bit $(CONFIG)/bin/bits/sample-start.bit
	cp bits/simple.bit $(CONFIG)/bin/bits/simple.bit
	cp bits/standard.bit $(CONFIG)/bin/bits/standard.bit
	cp bits/vstudio.es $(CONFIG)/bin/bits/vstudio.es
	cp bits/xcode.es $(CONFIG)/bin/bits/xcode.es

#
#   bit.o
#
DEPS_37 += $(CONFIG)/inc/bit.h
DEPS_37 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/bit.o: \
    src/bit.c $(DEPS_37)
	@echo '   [Compile] $(CONFIG)/obj/bit.o'
	$(CC) -c -o $(CONFIG)/obj/bit.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/bit.c

#
#   bit
#
DEPS_38 += $(CONFIG)/inc/mpr.h
DEPS_38 += $(CONFIG)/inc/bit.h
DEPS_38 += $(CONFIG)/inc/bitos.h
DEPS_38 += $(CONFIG)/obj/mprLib.o
DEPS_38 += $(CONFIG)/bin/libmpr.so
DEPS_38 += $(CONFIG)/inc/pcre.h
DEPS_38 += $(CONFIG)/obj/pcre.o
ifeq ($(BIT_PACK_PCRE),1)
    DEPS_38 += $(CONFIG)/bin/libpcre.so
endif
DEPS_38 += $(CONFIG)/inc/http.h
DEPS_38 += $(CONFIG)/obj/httpLib.o
DEPS_38 += $(CONFIG)/bin/libhttp.so
DEPS_38 += $(CONFIG)/inc/zlib.h
DEPS_38 += $(CONFIG)/obj/zlib.o
ifeq ($(BIT_PACK_ZLIB),1)
    DEPS_38 += $(CONFIG)/bin/libzlib.so
endif
DEPS_38 += $(CONFIG)/inc/ejs.h
DEPS_38 += $(CONFIG)/inc/ejs.slots.h
DEPS_38 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_38 += $(CONFIG)/obj/ejsLib.o
ifeq ($(BIT_PACK_EJSCRIPT),1)
    DEPS_38 += $(CONFIG)/bin/libejs.so
endif
DEPS_38 += $(CONFIG)/bin/bits
DEPS_38 += $(CONFIG)/obj/bit.o

LIBS_38 += -lmpr
LIBS_38 += -lhttp
ifeq ($(BIT_PACK_PCRE),1)
    LIBS_38 += -lpcre
endif
ifeq ($(BIT_PACK_EJSCRIPT),1)
    LIBS_38 += -lejs
endif
ifeq ($(BIT_PACK_ZLIB),1)
    LIBS_38 += -lzlib
endif

$(CONFIG)/bin/bit: $(DEPS_38)
	@echo '      [Link] $(CONFIG)/bin/bit'
	$(CC) -o $(CONFIG)/bin/bit $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/bit.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) $(LIBS) 

#
#   bower.json
#
DEPS_39 += package.json

bower.json: $(DEPS_39)
	@echo '      [Copy] bower.json'
	mkdir -p "."
	cp package.json bower.json

#
#   stop
#
stop: $(DEPS_40)

#
#   installBinary
#
installBinary: $(DEPS_41)
	( \
	cd .; \
	mkdir -p "$(BIT_APP_PREFIX)" ; \
	rm -f "$(BIT_APP_PREFIX)/latest" ; \
	ln -s "0.9.4" "$(BIT_APP_PREFIX)/latest" ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/bin" ; \
	cp $(CONFIG)/bin/bit $(BIT_VAPP_PREFIX)/bin/bit ; \
	mkdir -p "$(BIT_BIN_PREFIX)" ; \
	rm -f "$(BIT_BIN_PREFIX)/bit" ; \
	ln -s "$(BIT_VAPP_PREFIX)/bin/bit" "$(BIT_BIN_PREFIX)/bit" ; \
	cp $(CONFIG)/bin/ejs $(BIT_VAPP_PREFIX)/bin/ejs ; \
	cp $(CONFIG)/bin/libejs.so $(BIT_VAPP_PREFIX)/bin/libejs.so ; \
	cp $(CONFIG)/bin/libest.so $(BIT_VAPP_PREFIX)/bin/libest.so ; \
	cp $(CONFIG)/bin/libhttp.so $(BIT_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(CONFIG)/bin/libmpr.so $(BIT_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(CONFIG)/bin/libmprssl.so $(BIT_VAPP_PREFIX)/bin/libmprssl.so ; \
	cp $(CONFIG)/bin/libpcre.so $(BIT_VAPP_PREFIX)/bin/libpcre.so ; \
	cp $(CONFIG)/bin/ca.crt $(BIT_VAPP_PREFIX)/bin/ca.crt ; \
	cp $(CONFIG)/bin/ejs.mod $(BIT_VAPP_PREFIX)/bin/ejs.mod ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits" ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits/bits" ; \
	cp bits/bit.es $(BIT_VAPP_PREFIX)/bin/bits/bit.es ; \
	cp bits/configure.es $(BIT_VAPP_PREFIX)/bin/bits/configure.es ; \
	cp bits/embedthis-manifest.bit $(BIT_VAPP_PREFIX)/bin/bits/embedthis-manifest.bit ; \
	cp bits/embedthis.bit $(BIT_VAPP_PREFIX)/bin/bits/embedthis.bit ; \
	cp bits/embedthis.es $(BIT_VAPP_PREFIX)/bin/bits/embedthis.es ; \
	cp bits/gendoc.es $(BIT_VAPP_PREFIX)/bin/bits/gendoc.es ; \
	cp bits/generate.es $(BIT_VAPP_PREFIX)/bin/bits/generate.es ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits/os" ; \
	cp bits/os/freebsd.bit $(BIT_VAPP_PREFIX)/bin/bits/os/freebsd.bit ; \
	cp bits/os/gcc.bit $(BIT_VAPP_PREFIX)/bin/bits/os/gcc.bit ; \
	cp bits/os/linux.bit $(BIT_VAPP_PREFIX)/bin/bits/os/linux.bit ; \
	cp bits/os/macosx.bit $(BIT_VAPP_PREFIX)/bin/bits/os/macosx.bit ; \
	cp bits/os/solaris.bit $(BIT_VAPP_PREFIX)/bin/bits/os/solaris.bit ; \
	cp bits/os/unix.bit $(BIT_VAPP_PREFIX)/bin/bits/os/unix.bit ; \
	cp bits/os/vxworks.bit $(BIT_VAPP_PREFIX)/bin/bits/os/vxworks.bit ; \
	cp bits/os/windows.bit $(BIT_VAPP_PREFIX)/bin/bits/os/windows.bit ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/bits/packs" ; \
	cp bits/packs/appweb-embed.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/appweb-embed.bit ; \
	cp bits/packs/appweb.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/appweb.bit ; \
	cp bits/packs/compiler.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/compiler.bit ; \
	cp bits/packs/doxygen.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/doxygen.bit ; \
	cp bits/packs/dsi.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/dsi.bit ; \
	cp bits/packs/dumpbin.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/dumpbin.bit ; \
	cp bits/packs/ejs.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/ejs.bit ; \
	cp bits/packs/ejscript.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/ejscript.bit ; \
	cp bits/packs/est.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/est.bit ; \
	cp bits/packs/gzip.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/gzip.bit ; \
	cp bits/packs/htmlmin.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/htmlmin.bit ; \
	cp bits/packs/http.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/http.bit ; \
	cp bits/packs/lib.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/lib.bit ; \
	cp bits/packs/link.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/link.bit ; \
	cp bits/packs/man.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/man.bit ; \
	cp bits/packs/man2html.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/man2html.bit ; \
	cp bits/packs/matrixssl.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/matrixssl.bit ; \
	cp bits/packs/md5.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/md5.bit ; \
	cp bits/packs/nanossl.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/nanossl.bit ; \
	cp bits/packs/ngmin.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/ngmin.bit ; \
	cp bits/packs/openssl.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/openssl.bit ; \
	cp bits/packs/pak.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/pak.bit ; \
	cp bits/packs/pcre.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/pcre.bit ; \
	cp bits/packs/pmaker.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/pmaker.bit ; \
	cp bits/packs/ranlib.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/ranlib.bit ; \
	cp bits/packs/rc.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/rc.bit ; \
	cp bits/packs/recess.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/recess.bit ; \
	cp bits/packs/sqlite.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/sqlite.bit ; \
	cp bits/packs/ssl.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/ssl.bit ; \
	cp bits/packs/strip.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/strip.bit ; \
	cp bits/packs/tidy.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/tidy.bit ; \
	cp bits/packs/uglifyjs.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/uglifyjs.bit ; \
	cp bits/packs/utest.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/utest.bit ; \
	cp bits/packs/vxworks.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/vxworks.bit ; \
	cp bits/packs/winsdk.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/winsdk.bit ; \
	cp bits/packs/zip.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/zip.bit ; \
	cp bits/packs/zlib.bit $(BIT_VAPP_PREFIX)/bin/bits/packs/zlib.bit ; \
	cp bits/sample-main.bit $(BIT_VAPP_PREFIX)/bin/bits/sample-main.bit ; \
	cp bits/sample-start.bit $(BIT_VAPP_PREFIX)/bin/bits/sample-start.bit ; \
	cp bits/simple.bit $(BIT_VAPP_PREFIX)/bin/bits/simple.bit ; \
	cp bits/standard.bit $(BIT_VAPP_PREFIX)/bin/bits/standard.bit ; \
	cp bits/vstudio.es $(BIT_VAPP_PREFIX)/bin/bits/vstudio.es ; \
	cp bits/xcode.es $(BIT_VAPP_PREFIX)/bin/bits/xcode.es ; \
	mkdir -p "$(BIT_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/man/bit.1 $(BIT_VAPP_PREFIX)/doc/man/man1/bit.1 ; \
	mkdir -p "$(BIT_MAN_PREFIX)/man1" ; \
	rm -f "$(BIT_MAN_PREFIX)/man1/bit.1" ; \
	ln -s "$(BIT_VAPP_PREFIX)/doc/man/man1/bit.1" "$(BIT_MAN_PREFIX)/man1/bit.1" ; \
	)

#
#   start
#
start: $(DEPS_42)

#
#   install
#
DEPS_43 += stop
DEPS_43 += installBinary
DEPS_43 += start

install: $(DEPS_43)

#
#   uninstall
#
DEPS_44 += stop

uninstall: $(DEPS_44)
	( \
	cd .; \
	rm -fr "$(BIT_VAPP_PREFIX)" ; \
	rm -f "$(BIT_APP_PREFIX)/latest" ; \
	rmdir -p "$(BIT_APP_PREFIX)" 2>/dev/null ; true ; \
	)

