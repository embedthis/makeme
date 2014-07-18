#
#   me-linux-default.mk -- Makefile to build Embedthis MakeMe for linux
#

NAME                  := me
VERSION               := 0.8.2
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= linux
CC                    ?= gcc
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_EJS            ?= 1
ME_COM_EST            ?= 1
ME_COM_HTTP           ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_PCRE           ?= 1
ME_COM_SQLITE         ?= 0
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_WINSDK         ?= 1
ME_COM_ZLIB           ?= 1

ifeq ($(ME_COM_EST),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_MATRIXSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_NANOSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_EJS),1)
    ME_COM_ZLIB := 1
endif

ME_COM_COMPILER_PATH  ?= gcc
ME_COM_LIB_PATH       ?= ar
ME_COM_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_COM_NANOSSL_PATH   ?= /usr/src/nanossl
ME_COM_OPENSSL_PATH   ?= /usr/src/openssl

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EJS=$(ME_COM_EJS) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-Ibuild/$(CONFIG)/inc"
LDFLAGS               += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS              += -Lbuild/$(CONFIG)/bin
LIBS                  += -lrt -ldl -lpthread -lm

DEBUG                 ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG
LDFLAGS-debug         ?= -g
DFLAGS-release        ?= 
CFLAGS-release        ?= -O2
LDFLAGS-release       ?= 
CFLAGS                += $(CFLAGS-$(DEBUG))
DFLAGS                += $(DFLAGS-$(DEBUG))
LDFLAGS               += $(LDFLAGS-$(DEBUG))

ME_ROOT_PREFIX        ?= 
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local
ME_DATA_PREFIX        ?= $(ME_ROOT_PREFIX)/
ME_STATE_PREFIX       ?= $(ME_ROOT_PREFIX)/var
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)/lib/$(NAME)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)/$(VERSION)
ME_BIN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/bin
ME_INC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/include
ME_LIB_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/lib
ME_MAN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/share/man
ME_SBIN_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local/sbin
ME_ETC_PREFIX         ?= $(ME_ROOT_PREFIX)/etc/$(NAME)
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)-default
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


ifeq ($(ME_COM_EJS),1)
    TARGETS           += build/$(CONFIG)/bin/ejs.mod
endif
TARGETS               += build/$(CONFIG)/bin/ejs.testme.es
ifeq ($(ME_COM_EJS),1)
    TARGETS           += build/$(CONFIG)/bin/ejs
endif
TARGETS               += build/$(CONFIG)/bin/ca.crt
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += build/$(CONFIG)/bin/http
endif
ifeq ($(ME_COM_EST),1)
    TARGETS           += build/$(CONFIG)/bin/libest.so
endif
TARGETS               += build/$(CONFIG)/bin/libmprssl.so
TARGETS               += build/$(CONFIG)/bin/libtestme.so
TARGETS               += build/$(CONFIG)/bin/me
TARGETS               += build/$(CONFIG)/bin/.updated
TARGETS               += build/$(CONFIG)/bin/testme
TARGETS               += build/$(CONFIG)/bin/testme.es

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(BUILD)/bin ] && mkdir -p $(BUILD)/bin; true
	@[ ! -x $(BUILD)/inc ] && mkdir -p $(BUILD)/inc; true
	@[ ! -x $(BUILD)/obj ] && mkdir -p $(BUILD)/obj; true
	@[ ! -f $(BUILD)/inc/osdep.h ] && cp src/paks/osdep/osdep.h $(BUILD)/inc/osdep.h ; true
	@if ! diff $(BUILD)/inc/osdep.h src/paks/osdep/osdep.h >/dev/null ; then\
		cp src/paks/osdep/osdep.h $(BUILD)/inc/osdep.h  ; \
	fi; true
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/me-linux-default-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/me-linux-default-me.h >/dev/null ; then\
		cp projects/me-linux-default-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(BUILD)/.makeflags

clean:
	rm -f "build/$(CONFIG)/obj/ejs.o"
	rm -f "build/$(CONFIG)/obj/ejsLib.o"
	rm -f "build/$(CONFIG)/obj/ejsc.o"
	rm -f "build/$(CONFIG)/obj/estLib.o"
	rm -f "build/$(CONFIG)/obj/http.o"
	rm -f "build/$(CONFIG)/obj/httpLib.o"
	rm -f "build/$(CONFIG)/obj/libtestme.o"
	rm -f "build/$(CONFIG)/obj/me.o"
	rm -f "build/$(CONFIG)/obj/mprLib.o"
	rm -f "build/$(CONFIG)/obj/mprSsl.o"
	rm -f "build/$(CONFIG)/obj/pcre.o"
	rm -f "build/$(CONFIG)/obj/testme.o"
	rm -f "build/$(CONFIG)/obj/zlib.o"
	rm -f "build/$(CONFIG)/bin/ejsc"
	rm -f "build/$(CONFIG)/bin/ejs"
	rm -f "build/$(CONFIG)/bin/ca.crt"
	rm -f "build/$(CONFIG)/bin/http"
	rm -f "build/$(CONFIG)/bin/libejs.so"
	rm -f "build/$(CONFIG)/bin/libest.so"
	rm -f "build/$(CONFIG)/bin/libhttp.so"
	rm -f "build/$(CONFIG)/bin/libmpr.so"
	rm -f "build/$(CONFIG)/bin/libmprssl.so"
	rm -f "build/$(CONFIG)/bin/libpcre.so"
	rm -f "build/$(CONFIG)/bin/libtestme.so"
	rm -f "build/$(CONFIG)/bin/libzlib.so"
	rm -f "build/$(CONFIG)/bin/testme"

clobber: clean
	rm -fr ./$(BUILD)


#
#   mpr.h
#
build/$(CONFIG)/inc/mpr.h: $(DEPS_1)
	@echo '      [Copy] build/$(CONFIG)/inc/mpr.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h build/$(CONFIG)/inc/mpr.h

#
#   me.h
#
build/$(CONFIG)/inc/me.h: $(DEPS_2)
	@echo '      [Copy] build/$(CONFIG)/inc/me.h'

#
#   osdep.h
#
DEPS_3 += build/$(CONFIG)/inc/me.h

build/$(CONFIG)/inc/osdep.h: $(DEPS_3)
	@echo '      [Copy] build/$(CONFIG)/inc/osdep.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/osdep/osdep.h build/$(CONFIG)/inc/osdep.h

#
#   mprLib.o
#
DEPS_4 += build/$(CONFIG)/inc/me.h
DEPS_4 += build/$(CONFIG)/inc/mpr.h
DEPS_4 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_4)
	@echo '   [Compile] build/$(CONFIG)/obj/mprLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_5 += build/$(CONFIG)/inc/mpr.h
DEPS_5 += build/$(CONFIG)/inc/me.h
DEPS_5 += build/$(CONFIG)/inc/osdep.h
DEPS_5 += build/$(CONFIG)/obj/mprLib.o

build/$(CONFIG)/bin/libmpr.so: $(DEPS_5)
	@echo '      [Link] build/$(CONFIG)/bin/libmpr.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/mprLib.o" $(LIBS) 

#
#   pcre.h
#
build/$(CONFIG)/inc/pcre.h: $(DEPS_6)
	@echo '      [Copy] build/$(CONFIG)/inc/pcre.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h build/$(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_7 += build/$(CONFIG)/inc/me.h
DEPS_7 += build/$(CONFIG)/inc/pcre.h

build/$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_7)
	@echo '   [Compile] build/$(CONFIG)/obj/pcre.o'
	$(CC) -c -o build/$(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_8 += build/$(CONFIG)/inc/pcre.h
DEPS_8 += build/$(CONFIG)/inc/me.h
DEPS_8 += build/$(CONFIG)/obj/pcre.o

build/$(CONFIG)/bin/libpcre.so: $(DEPS_8)
	@echo '      [Link] build/$(CONFIG)/bin/libpcre.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/pcre.o" $(LIBS) 
endif

#
#   http.h
#
build/$(CONFIG)/inc/http.h: $(DEPS_9)
	@echo '      [Copy] build/$(CONFIG)/inc/http.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/http/http.h build/$(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_10 += build/$(CONFIG)/inc/me.h
DEPS_10 += build/$(CONFIG)/inc/http.h
DEPS_10 += build/$(CONFIG)/inc/mpr.h

build/$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_10)
	@echo '   [Compile] build/$(CONFIG)/obj/httpLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_11 += build/$(CONFIG)/inc/mpr.h
DEPS_11 += build/$(CONFIG)/inc/me.h
DEPS_11 += build/$(CONFIG)/inc/osdep.h
DEPS_11 += build/$(CONFIG)/obj/mprLib.o
DEPS_11 += build/$(CONFIG)/bin/libmpr.so
DEPS_11 += build/$(CONFIG)/inc/pcre.h
DEPS_11 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_11 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_11 += build/$(CONFIG)/inc/http.h
DEPS_11 += build/$(CONFIG)/obj/httpLib.o

LIBS_11 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_11 += -lpcre
endif

build/$(CONFIG)/bin/libhttp.so: $(DEPS_11)
	@echo '      [Link] build/$(CONFIG)/bin/libhttp.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/httpLib.o" $(LIBPATHS_11) $(LIBS_11) $(LIBS_11) $(LIBS) 
endif

#
#   zlib.h
#
build/$(CONFIG)/inc/zlib.h: $(DEPS_12)
	@echo '      [Copy] build/$(CONFIG)/inc/zlib.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h build/$(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_13 += build/$(CONFIG)/inc/me.h
DEPS_13 += build/$(CONFIG)/inc/zlib.h

build/$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_13)
	@echo '   [Compile] build/$(CONFIG)/obj/zlib.o'
	$(CC) -c -o build/$(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_14 += build/$(CONFIG)/inc/zlib.h
DEPS_14 += build/$(CONFIG)/inc/me.h
DEPS_14 += build/$(CONFIG)/obj/zlib.o

build/$(CONFIG)/bin/libzlib.so: $(DEPS_14)
	@echo '      [Link] build/$(CONFIG)/bin/libzlib.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   ejs.h
#
build/$(CONFIG)/inc/ejs.h: $(DEPS_15)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejs.h build/$(CONFIG)/inc/ejs.h

#
#   ejs.slots.h
#
build/$(CONFIG)/inc/ejs.slots.h: $(DEPS_16)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejs.slots.h build/$(CONFIG)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
build/$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_17)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejsByteGoto.h build/$(CONFIG)/inc/ejsByteGoto.h

#
#   ejsLib.o
#
DEPS_18 += build/$(CONFIG)/inc/me.h
DEPS_18 += build/$(CONFIG)/inc/ejs.h
DEPS_18 += build/$(CONFIG)/inc/mpr.h
DEPS_18 += build/$(CONFIG)/inc/pcre.h
DEPS_18 += build/$(CONFIG)/inc/osdep.h
DEPS_18 += build/$(CONFIG)/inc/http.h
DEPS_18 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_18 += build/$(CONFIG)/inc/zlib.h

build/$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_18)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(ME_COM_EJS),1)
#
#   libejs
#
DEPS_19 += build/$(CONFIG)/inc/mpr.h
DEPS_19 += build/$(CONFIG)/inc/me.h
DEPS_19 += build/$(CONFIG)/inc/osdep.h
DEPS_19 += build/$(CONFIG)/obj/mprLib.o
DEPS_19 += build/$(CONFIG)/bin/libmpr.so
DEPS_19 += build/$(CONFIG)/inc/pcre.h
DEPS_19 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_19 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_19 += build/$(CONFIG)/inc/http.h
DEPS_19 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_19 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_19 += build/$(CONFIG)/inc/zlib.h
DEPS_19 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_19 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_19 += build/$(CONFIG)/inc/ejs.h
DEPS_19 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_19 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_19 += build/$(CONFIG)/obj/ejsLib.o

ifeq ($(ME_COM_HTTP),1)
    LIBS_19 += -lhttp
endif
LIBS_19 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_19 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_19 += -lzlib
endif

build/$(CONFIG)/bin/libejs.so: $(DEPS_19)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_19) $(LIBS_19) $(LIBS_19) $(LIBS) 
endif

#
#   ejsc.o
#
DEPS_20 += build/$(CONFIG)/inc/me.h
DEPS_20 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_20)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsc.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(ME_COM_EJS),1)
#
#   ejsc
#
DEPS_21 += build/$(CONFIG)/inc/mpr.h
DEPS_21 += build/$(CONFIG)/inc/me.h
DEPS_21 += build/$(CONFIG)/inc/osdep.h
DEPS_21 += build/$(CONFIG)/obj/mprLib.o
DEPS_21 += build/$(CONFIG)/bin/libmpr.so
DEPS_21 += build/$(CONFIG)/inc/pcre.h
DEPS_21 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_21 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_21 += build/$(CONFIG)/inc/http.h
DEPS_21 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_21 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_21 += build/$(CONFIG)/inc/zlib.h
DEPS_21 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_21 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_21 += build/$(CONFIG)/inc/ejs.h
DEPS_21 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_21 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_21 += build/$(CONFIG)/obj/ejsLib.o
DEPS_21 += build/$(CONFIG)/bin/libejs.so
DEPS_21 += build/$(CONFIG)/obj/ejsc.o

LIBS_21 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_21 += -lhttp
endif
LIBS_21 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_21 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_21 += -lzlib
endif

build/$(CONFIG)/bin/ejsc: $(DEPS_21)
	@echo '      [Link] build/$(CONFIG)/bin/ejsc'
	$(CC) -o build/$(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsc.o" $(LIBPATHS_21) $(LIBS_21) $(LIBS_21) $(LIBS) $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   ejs.mod
#
DEPS_22 += src/paks/ejs/ejs.es
DEPS_22 += build/$(CONFIG)/inc/mpr.h
DEPS_22 += build/$(CONFIG)/inc/me.h
DEPS_22 += build/$(CONFIG)/inc/osdep.h
DEPS_22 += build/$(CONFIG)/obj/mprLib.o
DEPS_22 += build/$(CONFIG)/bin/libmpr.so
DEPS_22 += build/$(CONFIG)/inc/pcre.h
DEPS_22 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_22 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_22 += build/$(CONFIG)/inc/http.h
DEPS_22 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_22 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_22 += build/$(CONFIG)/inc/zlib.h
DEPS_22 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_22 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_22 += build/$(CONFIG)/inc/ejs.h
DEPS_22 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_22 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_22 += build/$(CONFIG)/obj/ejsLib.o
DEPS_22 += build/$(CONFIG)/bin/libejs.so
DEPS_22 += build/$(CONFIG)/obj/ejsc.o
DEPS_22 += build/$(CONFIG)/bin/ejsc

build/$(CONFIG)/bin/ejs.mod: $(DEPS_22)
	( \
	cd src/paks/ejs; \
	../../../$(LBIN)/ejsc --out ../../../build/$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.es
#
DEPS_23 += src/tm/ejs.testme.es

build/$(CONFIG)/bin/ejs.testme.es: $(DEPS_23)
	( \
	cd src/tm; \
	cp ejs.testme.es ../../build/$(CONFIG)/bin ; \
	)

#
#   ejs.testme.mod
#
DEPS_24 += build/$(CONFIG)/inc/mpr.h
DEPS_24 += build/$(CONFIG)/inc/me.h
DEPS_24 += build/$(CONFIG)/inc/osdep.h
DEPS_24 += build/$(CONFIG)/obj/mprLib.o
DEPS_24 += build/$(CONFIG)/bin/libmpr.so
DEPS_24 += build/$(CONFIG)/inc/pcre.h
DEPS_24 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_24 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_24 += build/$(CONFIG)/inc/http.h
DEPS_24 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_24 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_24 += build/$(CONFIG)/inc/zlib.h
DEPS_24 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_24 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_24 += build/$(CONFIG)/inc/ejs.h
DEPS_24 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_24 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_24 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_24 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_24 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_24 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/ejs.testme.mod: $(DEPS_24)
	( \
	cd src/tm; \
	../../$(LBIN)/ejsc --out ../../build/$(CONFIG)/bin/ejs.testme.mod --optimize 9 ../../src/tm/ejs.testme.es ; \
	)
#
#   ejs.testme.mod
#
DEPS_25 += build/$(CONFIG)/inc/mpr.h
DEPS_25 += build/$(CONFIG)/inc/me.h
DEPS_25 += build/$(CONFIG)/inc/osdep.h
DEPS_25 += build/$(CONFIG)/obj/mprLib.o
DEPS_25 += build/$(CONFIG)/bin/libmpr.so
DEPS_25 += build/$(CONFIG)/inc/pcre.h
DEPS_25 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_25 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_25 += build/$(CONFIG)/inc/http.h
DEPS_25 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_25 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_25 += build/$(CONFIG)/inc/zlib.h
DEPS_25 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_25 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_25 += build/$(CONFIG)/inc/ejs.h
DEPS_25 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_25 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_25 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_25 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_25 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_25 += build/$(CONFIG)/bin/ejsc
endif

ejs.testme.mod: $(DEPS_25)
	cd src/tm; /Users/mob/git/me/build/linux-x86-default/bin/ejsc --debug --out /Users/mob/git/me/build/linux-x86-default/bin/ejs.testme.mod --optimize 9 /Users/mob/git/me/src/tm/ejs.testme.es ; cd ../..
#
#   ejs.o
#
DEPS_26 += build/$(CONFIG)/inc/me.h
DEPS_26 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_26)
	@echo '   [Compile] build/$(CONFIG)/obj/ejs.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(ME_COM_EJS),1)
#
#   ejscmd
#
DEPS_27 += build/$(CONFIG)/inc/mpr.h
DEPS_27 += build/$(CONFIG)/inc/me.h
DEPS_27 += build/$(CONFIG)/inc/osdep.h
DEPS_27 += build/$(CONFIG)/obj/mprLib.o
DEPS_27 += build/$(CONFIG)/bin/libmpr.so
DEPS_27 += build/$(CONFIG)/inc/pcre.h
DEPS_27 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_27 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_27 += build/$(CONFIG)/inc/http.h
DEPS_27 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_27 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_27 += build/$(CONFIG)/inc/zlib.h
DEPS_27 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_27 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_27 += build/$(CONFIG)/inc/ejs.h
DEPS_27 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_27 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_27 += build/$(CONFIG)/obj/ejsLib.o
DEPS_27 += build/$(CONFIG)/bin/libejs.so
DEPS_27 += build/$(CONFIG)/obj/ejs.o

LIBS_27 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_27 += -lhttp
endif
LIBS_27 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_27 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_27 += -lzlib
endif

build/$(CONFIG)/bin/ejs: $(DEPS_27)
	@echo '      [Link] build/$(CONFIG)/bin/ejs'
	$(CC) -o build/$(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejs.o" $(LIBPATHS_27) $(LIBS_27) $(LIBS_27) $(LIBS) $(LIBS) 
endif


#
#   http-ca-crt
#
DEPS_28 += src/paks/http/ca.crt

build/$(CONFIG)/bin/ca.crt: $(DEPS_28)
	@echo '      [Copy] build/$(CONFIG)/bin/ca.crt'
	mkdir -p "build/$(CONFIG)/bin"
	cp src/paks/http/ca.crt build/$(CONFIG)/bin/ca.crt

#
#   http.o
#
DEPS_29 += build/$(CONFIG)/inc/me.h
DEPS_29 += build/$(CONFIG)/inc/http.h

build/$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_29)
	@echo '   [Compile] build/$(CONFIG)/obj/http.o'
	$(CC) -c -o build/$(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_30 += build/$(CONFIG)/inc/mpr.h
DEPS_30 += build/$(CONFIG)/inc/me.h
DEPS_30 += build/$(CONFIG)/inc/osdep.h
DEPS_30 += build/$(CONFIG)/obj/mprLib.o
DEPS_30 += build/$(CONFIG)/bin/libmpr.so
DEPS_30 += build/$(CONFIG)/inc/pcre.h
DEPS_30 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_30 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_30 += build/$(CONFIG)/inc/http.h
DEPS_30 += build/$(CONFIG)/obj/httpLib.o
DEPS_30 += build/$(CONFIG)/bin/libhttp.so
DEPS_30 += build/$(CONFIG)/obj/http.o

LIBS_30 += -lhttp
LIBS_30 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_30 += -lpcre
endif

build/$(CONFIG)/bin/http: $(DEPS_30)
	@echo '      [Link] build/$(CONFIG)/bin/http'
	$(CC) -o build/$(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/http.o" $(LIBPATHS_30) $(LIBS_30) $(LIBS_30) $(LIBS) $(LIBS) 
endif

#
#   est.h
#
build/$(CONFIG)/inc/est.h: $(DEPS_31)
	@echo '      [Copy] build/$(CONFIG)/inc/est.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/est/est.h build/$(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_32 += build/$(CONFIG)/inc/me.h
DEPS_32 += build/$(CONFIG)/inc/est.h
DEPS_32 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_32)
	@echo '   [Compile] build/$(CONFIG)/obj/estLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_33 += build/$(CONFIG)/inc/est.h
DEPS_33 += build/$(CONFIG)/inc/me.h
DEPS_33 += build/$(CONFIG)/inc/osdep.h
DEPS_33 += build/$(CONFIG)/obj/estLib.o

build/$(CONFIG)/bin/libest.so: $(DEPS_33)
	@echo '      [Link] build/$(CONFIG)/bin/libest.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   mprSsl.o
#
DEPS_34 += build/$(CONFIG)/inc/me.h
DEPS_34 += build/$(CONFIG)/inc/mpr.h
DEPS_34 += build/$(CONFIG)/inc/est.h

build/$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_34)
	@echo '   [Compile] build/$(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o build/$(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MATRIXSSL_PATH)" "-I$(ME_COM_MATRIXSSL_PATH)/matrixssl" "-I$(ME_COM_NANOSSL_PATH)/src" src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_35 += build/$(CONFIG)/inc/mpr.h
DEPS_35 += build/$(CONFIG)/inc/me.h
DEPS_35 += build/$(CONFIG)/inc/osdep.h
DEPS_35 += build/$(CONFIG)/obj/mprLib.o
DEPS_35 += build/$(CONFIG)/bin/libmpr.so
DEPS_35 += build/$(CONFIG)/inc/est.h
DEPS_35 += build/$(CONFIG)/obj/estLib.o
ifeq ($(ME_COM_EST),1)
    DEPS_35 += build/$(CONFIG)/bin/libest.so
endif
DEPS_35 += build/$(CONFIG)/obj/mprSsl.o

LIBS_35 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_35 += -lssl
    LIBPATHS_35 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_35 += -lcrypto
    LIBPATHS_35 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_EST),1)
    LIBS_35 += -lest
endif
ifeq ($(ME_COM_MATRIXSSL),1)
    LIBS_35 += -lmatrixssl
    LIBPATHS_35 += -L$(ME_COM_MATRIXSSL_PATH)
endif
ifeq ($(ME_COM_NANOSSL),1)
    LIBS_35 += -lssls
    LIBPATHS_35 += -L$(ME_COM_NANOSSL_PATH)/bin
endif

build/$(CONFIG)/bin/libmprssl.so: $(DEPS_35)
	@echo '      [Link] build/$(CONFIG)/bin/libmprssl.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS)    "build/$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_35) $(LIBS_35) $(LIBS_35) $(LIBS) 

#
#   testme.h
#
build/$(CONFIG)/inc/testme.h: $(DEPS_36)
	@echo '      [Copy] build/$(CONFIG)/inc/testme.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/tm/testme.h build/$(CONFIG)/inc/testme.h

#
#   libtestme.o
#
DEPS_37 += build/$(CONFIG)/inc/me.h
DEPS_37 += build/$(CONFIG)/inc/testme.h
DEPS_37 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/obj/libtestme.o: \
    src/tm/libtestme.c $(DEPS_37)
	@echo '   [Compile] build/$(CONFIG)/obj/libtestme.o'
	$(CC) -c -o build/$(CONFIG)/obj/libtestme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/libtestme.c

#
#   libtestme
#
DEPS_38 += build/$(CONFIG)/inc/testme.h
DEPS_38 += build/$(CONFIG)/inc/me.h
DEPS_38 += build/$(CONFIG)/inc/osdep.h
DEPS_38 += build/$(CONFIG)/obj/libtestme.o

build/$(CONFIG)/bin/libtestme.so: $(DEPS_38)
	@echo '      [Link] build/$(CONFIG)/bin/libtestme.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libtestme.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/libtestme.o" $(LIBS) 

#
#   me.mod
#
DEPS_39 += build/$(CONFIG)/inc/mpr.h
DEPS_39 += build/$(CONFIG)/inc/me.h
DEPS_39 += build/$(CONFIG)/inc/osdep.h
DEPS_39 += build/$(CONFIG)/obj/mprLib.o
DEPS_39 += build/$(CONFIG)/bin/libmpr.so
DEPS_39 += build/$(CONFIG)/inc/pcre.h
DEPS_39 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_39 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_39 += build/$(CONFIG)/inc/http.h
DEPS_39 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_39 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_39 += build/$(CONFIG)/inc/zlib.h
DEPS_39 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_39 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_39 += build/$(CONFIG)/inc/ejs.h
DEPS_39 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_39 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_39 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_39 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_39 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_39 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/me.mod: $(DEPS_39)
	( \
	cd .; \
	$(LBIN)/ejsc --out ./build/$(CONFIG)/bin/me.mod --optimize 9 ./src/me.es ./src/paks/ejs-version/Version.es ; \
	)
#
#   me.mod
#
DEPS_40 += build/$(CONFIG)/inc/mpr.h
DEPS_40 += build/$(CONFIG)/inc/me.h
DEPS_40 += build/$(CONFIG)/inc/osdep.h
DEPS_40 += build/$(CONFIG)/obj/mprLib.o
DEPS_40 += build/$(CONFIG)/bin/libmpr.so
DEPS_40 += build/$(CONFIG)/inc/pcre.h
DEPS_40 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_40 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_40 += build/$(CONFIG)/inc/http.h
DEPS_40 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_40 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_40 += build/$(CONFIG)/inc/zlib.h
DEPS_40 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_40 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_40 += build/$(CONFIG)/inc/ejs.h
DEPS_40 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_40 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_40 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_40 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_40 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_40 += build/$(CONFIG)/bin/ejsc
endif

me.mod: $(DEPS_40)
	/Users/mob/git/me/build/linux-x86-default/bin/ejsc --out /Users/mob/git/me/build/linux-x86-default/bin/me.mod --optimize 9 /Users/mob/git/me/src/me.es /Users/mob/git/me/src/paks/ejs-version/Version.es
#
#   me.o
#
DEPS_41 += build/$(CONFIG)/inc/me.h
DEPS_41 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/me.o: \
    src/me.c $(DEPS_41)
	@echo '   [Compile] build/$(CONFIG)/obj/me.o'
	$(CC) -c -o build/$(CONFIG)/obj/me.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/me.c

#
#   me
#
DEPS_42 += build/$(CONFIG)/inc/mpr.h
DEPS_42 += build/$(CONFIG)/inc/me.h
DEPS_42 += build/$(CONFIG)/inc/osdep.h
DEPS_42 += build/$(CONFIG)/obj/mprLib.o
DEPS_42 += build/$(CONFIG)/bin/libmpr.so
DEPS_42 += build/$(CONFIG)/inc/pcre.h
DEPS_42 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_42 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_42 += build/$(CONFIG)/inc/http.h
DEPS_42 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_42 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_42 += build/$(CONFIG)/inc/zlib.h
DEPS_42 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_42 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_42 += build/$(CONFIG)/inc/ejs.h
DEPS_42 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_42 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_42 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_42 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_42 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_42 += build/$(CONFIG)/bin/ejsc
endif
DEPS_42 += build/$(CONFIG)/bin/me.mod
DEPS_42 += build/$(CONFIG)/obj/me.o

LIBS_42 += -lmpr
ifeq ($(ME_COM_HTTP),1)
    LIBS_42 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_42 += -lpcre
endif
ifeq ($(ME_COM_EJS),1)
    LIBS_42 += -lejs
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_42 += -lzlib
endif

build/$(CONFIG)/bin/me: $(DEPS_42)
	@echo '      [Link] build/$(CONFIG)/bin/me'
	$(CC) -o build/$(CONFIG)/bin/me $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/me.o" $(LIBPATHS_42) $(LIBS_42) $(LIBS_42) $(LIBS) $(LIBS) 

#
#   me-core
#
DEPS_43 += src/configure/appweb.me
DEPS_43 += src/configure/compiler.me
DEPS_43 += src/configure/lib.me
DEPS_43 += src/configure/link.me
DEPS_43 += src/configure/rc.me
DEPS_43 += src/configure/testme.me
DEPS_43 += src/configure/vxworks.me
DEPS_43 += src/configure/winsdk.me
DEPS_43 += src/configure.es
DEPS_43 += src/generate.es
DEPS_43 += src/master-main.me
DEPS_43 += src/master-start.me
DEPS_43 += src/me.es
DEPS_43 += src/os/freebsd.me
DEPS_43 += src/os/gcc.me
DEPS_43 += src/os/linux.me
DEPS_43 += src/os/macosx.me
DEPS_43 += src/os/solaris.me
DEPS_43 += src/os/unix.me
DEPS_43 += src/os/vxworks.me
DEPS_43 += src/os/windows.me
DEPS_43 += src/simple.me
DEPS_43 += src/standard.me
DEPS_43 += src/vstudio.es
DEPS_43 += src/xcode.es

build/$(CONFIG)/bin/.updated: $(DEPS_43)
	@echo '      [Copy] build/$(CONFIG)/bin'
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/appweb.me build/$(CONFIG)/bin/configure/appweb.me
	cp src/configure/compiler.me build/$(CONFIG)/bin/configure/compiler.me
	cp src/configure/lib.me build/$(CONFIG)/bin/configure/lib.me
	cp src/configure/link.me build/$(CONFIG)/bin/configure/link.me
	cp src/configure/rc.me build/$(CONFIG)/bin/configure/rc.me
	cp src/configure/testme.me build/$(CONFIG)/bin/configure/testme.me
	cp src/configure/vxworks.me build/$(CONFIG)/bin/configure/vxworks.me
	cp src/configure/winsdk.me build/$(CONFIG)/bin/configure/winsdk.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/configure.es build/$(CONFIG)/bin/configure.es
	cp src/generate.es build/$(CONFIG)/bin/generate.es
	cp src/master-main.me build/$(CONFIG)/bin/master-main.me
	cp src/master-start.me build/$(CONFIG)/bin/master-start.me
	cp src/me.es build/$(CONFIG)/bin/me.es
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/freebsd.me build/$(CONFIG)/bin/os/freebsd.me
	cp src/os/gcc.me build/$(CONFIG)/bin/os/gcc.me
	cp src/os/linux.me build/$(CONFIG)/bin/os/linux.me
	cp src/os/macosx.me build/$(CONFIG)/bin/os/macosx.me
	cp src/os/solaris.me build/$(CONFIG)/bin/os/solaris.me
	cp src/os/unix.me build/$(CONFIG)/bin/os/unix.me
	cp src/os/vxworks.me build/$(CONFIG)/bin/os/vxworks.me
	cp src/os/windows.me build/$(CONFIG)/bin/os/windows.me
	cp src/simple.me build/$(CONFIG)/bin/simple.me
	cp src/standard.me build/$(CONFIG)/bin/standard.me
	cp src/vstudio.es build/$(CONFIG)/bin/vstudio.es
	cp src/xcode.es build/$(CONFIG)/bin/xcode.es
	rm -fr "build/$(CONFIG)/bin/.updated"
	mkdir -p "build/$(CONFIG)/bin/.updated"

#
#   testme.mod
#
DEPS_44 += build/$(CONFIG)/inc/mpr.h
DEPS_44 += build/$(CONFIG)/inc/me.h
DEPS_44 += build/$(CONFIG)/inc/osdep.h
DEPS_44 += build/$(CONFIG)/obj/mprLib.o
DEPS_44 += build/$(CONFIG)/bin/libmpr.so
DEPS_44 += build/$(CONFIG)/inc/pcre.h
DEPS_44 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_44 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_44 += build/$(CONFIG)/inc/http.h
DEPS_44 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_44 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_44 += build/$(CONFIG)/inc/zlib.h
DEPS_44 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_44 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_44 += build/$(CONFIG)/inc/ejs.h
DEPS_44 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_44 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_44 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_44 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_44 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_44 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/testme.mod: $(DEPS_44)
	( \
	cd src/tm; \
	../../$(LBIN)/ejsc --out ../../build/$(CONFIG)/bin/testme.mod --optimize 9 ../../src/tm/testme.es ; \
	)
#
#   testme.mod
#
DEPS_45 += build/$(CONFIG)/inc/mpr.h
DEPS_45 += build/$(CONFIG)/inc/me.h
DEPS_45 += build/$(CONFIG)/inc/osdep.h
DEPS_45 += build/$(CONFIG)/obj/mprLib.o
DEPS_45 += build/$(CONFIG)/bin/libmpr.so
DEPS_45 += build/$(CONFIG)/inc/pcre.h
DEPS_45 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_45 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_45 += build/$(CONFIG)/inc/http.h
DEPS_45 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_45 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_45 += build/$(CONFIG)/inc/zlib.h
DEPS_45 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_45 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_45 += build/$(CONFIG)/inc/ejs.h
DEPS_45 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_45 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_45 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_45 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_45 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_45 += build/$(CONFIG)/bin/ejsc
endif

testme.mod: $(DEPS_45)
	cd src/tm; /Users/mob/git/me/build/linux-x86-default/bin/ejsc --debug --out /Users/mob/git/me/build/linux-x86-default/bin/testme.mod --optimize 9 /Users/mob/git/me/src/tm/testme.es ; cd ../..
#
#   testme.o
#
DEPS_46 += build/$(CONFIG)/inc/me.h
DEPS_46 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/testme.o: \
    src/tm/testme.c $(DEPS_46)
	@echo '   [Compile] build/$(CONFIG)/obj/testme.o'
	$(CC) -c -o build/$(CONFIG)/obj/testme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/testme.c

#
#   testme
#
DEPS_47 += build/$(CONFIG)/inc/mpr.h
DEPS_47 += build/$(CONFIG)/inc/me.h
DEPS_47 += build/$(CONFIG)/inc/osdep.h
DEPS_47 += build/$(CONFIG)/obj/mprLib.o
DEPS_47 += build/$(CONFIG)/bin/libmpr.so
DEPS_47 += build/$(CONFIG)/inc/pcre.h
DEPS_47 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_47 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_47 += build/$(CONFIG)/inc/http.h
DEPS_47 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_47 += build/$(CONFIG)/bin/libhttp.so
endif
DEPS_47 += build/$(CONFIG)/inc/zlib.h
DEPS_47 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_47 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_47 += build/$(CONFIG)/inc/ejs.h
DEPS_47 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_47 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_47 += build/$(CONFIG)/obj/ejsLib.o
ifeq ($(ME_COM_EJS),1)
    DEPS_47 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_47 += build/$(CONFIG)/obj/ejsc.o
ifeq ($(ME_COM_EJS),1)
    DEPS_47 += build/$(CONFIG)/bin/ejsc
endif
DEPS_47 += build/$(CONFIG)/bin/testme.mod
DEPS_47 += build/$(CONFIG)/bin/ejs.testme.mod
DEPS_47 += build/$(CONFIG)/obj/testme.o

ifeq ($(ME_COM_EJS),1)
    LIBS_47 += -lejs
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_47 += -lhttp
endif
LIBS_47 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_47 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_47 += -lzlib
endif

build/$(CONFIG)/bin/testme: $(DEPS_47)
	@echo '      [Link] build/$(CONFIG)/bin/testme'
	$(CC) -o build/$(CONFIG)/bin/testme $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/testme.o" $(LIBPATHS_47) $(LIBS_47) $(LIBS_47) $(LIBS) $(LIBS) 

#
#   testme.es
#
DEPS_48 += src/tm/testme.es

build/$(CONFIG)/bin/testme.es: $(DEPS_48)
	( \
	cd src/tm; \
	cp testme.es ../../build/$(CONFIG)/bin ; \
	)

#
#   stop
#
stop: $(DEPS_49)

#
#   installBinary
#
installBinary: $(DEPS_50)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.8.2" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp build/$(CONFIG)/bin/me $(ME_VAPP_PREFIX)/bin/me ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/me" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/me" "$(ME_BIN_PREFIX)/me" ; \
	cp build/$(CONFIG)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
	rm -f "$(ME_BIN_PREFIX)/ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejs" "$(ME_BIN_PREFIX)/ejs" ; \
	cp build/$(CONFIG)/bin/testme $(ME_VAPP_PREFIX)/bin/testme ; \
	rm -f "$(ME_BIN_PREFIX)/testme" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/testme" "$(ME_BIN_PREFIX)/testme" ; \
	cp build/$(CONFIG)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
	cp build/$(CONFIG)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp build/$(CONFIG)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp build/$(CONFIG)/bin/libmprssl.so $(ME_VAPP_PREFIX)/bin/libmprssl.so ; \
	cp build/$(CONFIG)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	cp build/$(CONFIG)/bin/libzlib.so $(ME_VAPP_PREFIX)/bin/libzlib.so ; \
	cp build/$(CONFIG)/bin/libtestme.so $(ME_VAPP_PREFIX)/bin/libtestme.so ; \
	if [ "$(ME_COM_EST)" = 1 ]; then true ; \
	cp build/$(CONFIG)/bin/libest.so $(ME_VAPP_PREFIX)/bin/libest.so ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	cp build/$(CONFIG)/bin/libssl*.so* $(ME_VAPP_PREFIX)/bin/libssl*.so* ; \
	cp build/$(CONFIG)/bin/libcrypto*.so* $(ME_VAPP_PREFIX)/bin/libcrypto*.so* ; \
	fi ; \
	cp build/$(CONFIG)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	cp build/$(CONFIG)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp build/$(CONFIG)/bin/me.mod $(ME_VAPP_PREFIX)/bin/me.mod ; \
	cp build/$(CONFIG)/bin/testme.mod $(ME_VAPP_PREFIX)/bin/testme.mod ; \
	cp build/$(CONFIG)/bin/ejs.testme.mod $(ME_VAPP_PREFIX)/bin/ejs.testme.mod ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/configure" ; \
	cp src/configure/appweb.me $(ME_VAPP_PREFIX)/bin/configure/appweb.me ; \
	cp src/configure/compiler.me $(ME_VAPP_PREFIX)/bin/configure/compiler.me ; \
	cp src/configure/lib.me $(ME_VAPP_PREFIX)/bin/configure/lib.me ; \
	cp src/configure/link.me $(ME_VAPP_PREFIX)/bin/configure/link.me ; \
	cp src/configure/rc.me $(ME_VAPP_PREFIX)/bin/configure/rc.me ; \
	cp src/configure/testme.me $(ME_VAPP_PREFIX)/bin/configure/testme.me ; \
	cp src/configure/vxworks.me $(ME_VAPP_PREFIX)/bin/configure/vxworks.me ; \
	cp src/configure/winsdk.me $(ME_VAPP_PREFIX)/bin/configure/winsdk.me ; \
	cp src/configure.es $(ME_VAPP_PREFIX)/bin/configure.es ; \
	cp src/generate.es $(ME_VAPP_PREFIX)/bin/generate.es ; \
	cp src/master-main.me $(ME_VAPP_PREFIX)/bin/master-main.me ; \
	cp src/master-start.me $(ME_VAPP_PREFIX)/bin/master-start.me ; \
	cp src/me.es $(ME_VAPP_PREFIX)/bin/me.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/os" ; \
	cp src/os/freebsd.me $(ME_VAPP_PREFIX)/bin/os/freebsd.me ; \
	cp src/os/gcc.me $(ME_VAPP_PREFIX)/bin/os/gcc.me ; \
	cp src/os/linux.me $(ME_VAPP_PREFIX)/bin/os/linux.me ; \
	cp src/os/macosx.me $(ME_VAPP_PREFIX)/bin/os/macosx.me ; \
	cp src/os/solaris.me $(ME_VAPP_PREFIX)/bin/os/solaris.me ; \
	cp src/os/unix.me $(ME_VAPP_PREFIX)/bin/os/unix.me ; \
	cp src/os/vxworks.me $(ME_VAPP_PREFIX)/bin/os/vxworks.me ; \
	cp src/os/windows.me $(ME_VAPP_PREFIX)/bin/os/windows.me ; \
	cp src/simple.me $(ME_VAPP_PREFIX)/bin/simple.me ; \
	cp src/standard.me $(ME_VAPP_PREFIX)/bin/standard.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/tm" ; \
	cp src/tm/ejs.testme.es $(ME_VAPP_PREFIX)/bin/tm/ejs.testme.es ; \
	cp src/tm/libtestme.c $(ME_VAPP_PREFIX)/bin/tm/libtestme.c ; \
	cp src/tm/sample.ct $(ME_VAPP_PREFIX)/bin/tm/sample.ct ; \
	cp src/tm/testme.c $(ME_VAPP_PREFIX)/bin/tm/testme.c ; \
	cp src/tm/testme.es $(ME_VAPP_PREFIX)/bin/tm/testme.es ; \
	cp src/tm/testme.h $(ME_VAPP_PREFIX)/bin/tm/testme.h ; \
	cp src/tm/testme.me $(ME_VAPP_PREFIX)/bin/tm/testme.me ; \
	cp src/vstudio.es $(ME_VAPP_PREFIX)/bin/vstudio.es ; \
	cp src/xcode.es $(ME_VAPP_PREFIX)/bin/xcode.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	cp doc/man/testme.1 $(ME_VAPP_PREFIX)/doc/man/man1/testme.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/testme.1" "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	)

#
#   start
#
start: $(DEPS_51)

#
#   install
#
DEPS_52 += stop
DEPS_52 += installBinary
DEPS_52 += start

install: $(DEPS_52)

#
#   uninstall
#
DEPS_53 += stop

uninstall: $(DEPS_53)
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

#
#   version
#
version: $(DEPS_54)
	( \
	cd build/macosx-x64-release/bin; \
	echo 0.8.2 ; \
	)

