#
#   makeme-linux-default.mk -- Makefile to build Embedthis MakeMe for linux
#

NAME                  := makeme
VERSION               := 0.8.0
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= linux
CC                    ?= gcc
LD                    ?= link
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(CONFIG)/bin
PATH                  := $(LBIN):$(PATH)

ME_EXT_EJS            ?= 1
ME_EXT_EST            ?= 1
ME_EXT_OPENSSL        ?= 0
ME_EXT_PCRE           ?= 1
ME_EXT_SQLITE         ?= 1
ME_EXT_SSL            ?= 1
ME_EXT_ZLIB           ?= 1

ME_EXT_COMPILER_PATH  ?= gcc
ME_EXT_DSI_PATH       ?= dsi
ME_EXT_EJS_PATH       ?= src/paks/ejs
ME_EXT_EST_PATH       ?= src/paks/est/estLib.c
ME_EXT_HTTP_PATH      ?= src/paks/http
ME_EXT_LIB_PATH       ?= ar
ME_EXT_LINK_PATH      ?= link
ME_EXT_MAN_PATH       ?= man
ME_EXT_MAN2HTML_PATH  ?= man2html
ME_EXT_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_EXT_MPR_PATH       ?= src/paks/mpr
ME_EXT_NANOSSL_PATH   ?= /usr/src/nanossl
ME_EXT_OPENSSL_PATH   ?= /usr/src/openssl
ME_EXT_OSDEP_PATH     ?= src/paks/osdep
ME_EXT_PCRE_PATH      ?= src/paks/pcre
ME_EXT_PMAKER_PATH    ?= pmaker
ME_EXT_SSL_PATH       ?= ssl
ME_EXT_VXWORKS_PATH   ?= $(WIND_BASE)
ME_EXT_WINSDK_PATH    ?= winsdk
ME_EXT_ZIP_PATH       ?= zip
ME_EXT_ZLIB_PATH      ?= src/paks/zlib

export WIND_HOME      ?= $(WIND_BASE)/..

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_EXT_EJS=$(ME_EXT_EJS) -DME_EXT_EST=$(ME_EXT_EST) -DME_EXT_OPENSSL=$(ME_EXT_OPENSSL) -DME_EXT_PCRE=$(ME_EXT_PCRE) -DME_EXT_SQLITE=$(ME_EXT_SQLITE) -DME_EXT_SSL=$(ME_EXT_SSL) -DME_EXT_ZLIB=$(ME_EXT_ZLIB) 
IFLAGS                += "-I$(CONFIG)/inc"
LDFLAGS               += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS              += -L$(CONFIG)/bin
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


ifeq ($(ME_EXT_EJS),1)
    TARGETS           += $(CONFIG)/bin/libejs.so
endif
ifeq ($(ME_EXT_EJS),1)
    TARGETS           += $(CONFIG)/bin/ejscmd
endif
ifeq ($(ME_EXT_EJS),1)
    TARGETS           += $(CONFIG)/bin/ejsc
endif
ifeq ($(ME_EXT_EJS),1)
    TARGETS           += $(CONFIG)/bin/ejs.mod
endif
ifeq ($(ME_EXT_EST),1)
    TARGETS           += $(CONFIG)/bin/libest.so
endif
TARGETS               += $(CONFIG)/bin/ca.crt
TARGETS               += $(CONFIG)/bin/httpcmd
TARGETS               += $(CONFIG)/bin/libmprssl.so
ifeq ($(ME_EXT_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/libsql.so
endif
ifeq ($(ME_EXT_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/sqlite
endif
TARGETS               += $(CONFIG)/bin/me
TARGETS               += $(CONFIG)/bin/makeme
TARGETS               += bower.json

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
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/osdep.h ] && cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h ; true
	@if ! diff $(CONFIG)/inc/osdep.h src/paks/osdep/osdep.h >/dev/null ; then\
		cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h  ; \
	fi; true
	@[ ! -f $(CONFIG)/inc/me.h ] && cp projects/makeme-linux-default-me.h $(CONFIG)/inc/me.h ; true
	@if ! diff $(CONFIG)/inc/me.h projects/makeme-linux-default-me.h >/dev/null ; then\
		cp projects/makeme-linux-default-me.h $(CONFIG)/inc/me.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags

clean:
	rm -f "$(CONFIG)/bin/libejs.so"
	rm -f "$(CONFIG)/bin/ejscmd"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/libest.so"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libhttp.so"
	rm -f "$(CONFIG)/bin/httpcmd"
	rm -f "$(CONFIG)/bin/libmpr.so"
	rm -f "$(CONFIG)/bin/libmprssl.so"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libpcre.so"
	rm -f "$(CONFIG)/bin/libsql.so"
	rm -f "$(CONFIG)/bin/sqlite"
	rm -f "$(CONFIG)/bin/libzlib.so"
	rm -f "bower.json"
	rm -f "$(CONFIG)/obj/ejsLib.o"
	rm -f "$(CONFIG)/obj/ejs.o"
	rm -f "$(CONFIG)/obj/ejsc.o"
	rm -f "$(CONFIG)/obj/estLib.o"
	rm -f "$(CONFIG)/obj/httpLib.o"
	rm -f "$(CONFIG)/obj/http.o"
	rm -f "$(CONFIG)/obj/mprLib.o"
	rm -f "$(CONFIG)/obj/mprSsl.o"
	rm -f "$(CONFIG)/obj/makerom.o"
	rm -f "$(CONFIG)/obj/pcre.o"
	rm -f "$(CONFIG)/obj/sqlite3.o"
	rm -f "$(CONFIG)/obj/sqlite.o"
	rm -f "$(CONFIG)/obj/zlib.o"
	rm -f "$(CONFIG)/obj/me.o"

clobber: clean
	rm -fr ./$(CONFIG)



#
#   version
#
version: $(DEPS_1)
	( \
	cd macosx-x64-release/bin/makeme; \
	echo 0.8.0 ; \
	)

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h $(CONFIG)/inc/mpr.h

#
#   me.h
#
$(CONFIG)/inc/me.h: $(DEPS_3)
	@echo '      [Copy] $(CONFIG)/inc/me.h'

#
#   osdep.h
#
$(CONFIG)/inc/osdep.h: $(DEPS_4)
	@echo '      [Copy] $(CONFIG)/inc/osdep.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h

#
#   mprLib.o
#
DEPS_5 += $(CONFIG)/inc/me.h
DEPS_5 += $(CONFIG)/inc/mpr.h
DEPS_5 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_5)
	@echo '   [Compile] $(CONFIG)/obj/mprLib.o'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/osdep.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.so: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libmpr.so'
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/mprLib.o" $(LIBS) 

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h $(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_8 += $(CONFIG)/inc/me.h
DEPS_8 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_8)
	@echo '   [Compile] $(CONFIG)/obj/pcre.o'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_EXT_PCRE),1)
#
#   libpcre
#
DEPS_9 += $(CONFIG)/inc/pcre.h
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.so: $(DEPS_9)
	@echo '      [Link] $(CONFIG)/bin/libpcre.so'
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/pcre.o" $(LIBS) 
endif

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_10)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/http/http.h $(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_11 += $(CONFIG)/inc/me.h
DEPS_11 += $(CONFIG)/inc/http.h
DEPS_11 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_11)
	@echo '   [Compile] $(CONFIG)/obj/httpLib.o'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

#
#   libhttp
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/osdep.h
DEPS_12 += $(CONFIG)/obj/mprLib.o
DEPS_12 += $(CONFIG)/bin/libmpr.so
DEPS_12 += $(CONFIG)/inc/pcre.h
DEPS_12 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_12 += $(CONFIG)/bin/libpcre.so
endif
DEPS_12 += $(CONFIG)/inc/http.h
DEPS_12 += $(CONFIG)/obj/httpLib.o

LIBS_12 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_12 += -lpcre
endif

$(CONFIG)/bin/libhttp.so: $(DEPS_12)
	@echo '      [Link] $(CONFIG)/bin/libhttp.so'
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/httpLib.o" $(LIBPATHS_12) $(LIBS_12) $(LIBS_12) $(LIBS) 

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_13)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h $(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_14 += $(CONFIG)/inc/me.h
DEPS_14 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_14)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_EXT_ZLIB),1)
#
#   libzlib
#
DEPS_15 += $(CONFIG)/inc/zlib.h
DEPS_15 += $(CONFIG)/inc/me.h
DEPS_15 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.so: $(DEPS_15)
	@echo '      [Link] $(CONFIG)/bin/libzlib.so'
	$(CC) -shared -o $(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   ejs.h
#
$(CONFIG)/inc/ejs.h: $(DEPS_16)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.h $(CONFIG)/inc/ejs.h

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_17)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_18)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejsByteGoto.h $(CONFIG)/inc/ejsByteGoto.h

#
#   ejsLib.o
#
DEPS_19 += $(CONFIG)/inc/me.h
DEPS_19 += $(CONFIG)/inc/ejs.h
DEPS_19 += $(CONFIG)/inc/mpr.h
DEPS_19 += $(CONFIG)/inc/pcre.h
DEPS_19 += $(CONFIG)/inc/osdep.h
DEPS_19 += $(CONFIG)/inc/http.h
DEPS_19 += $(CONFIG)/inc/ejs.slots.h
DEPS_19 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_19)
	@echo '   [Compile] $(CONFIG)/obj/ejsLib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(ME_EXT_EJS),1)
#
#   libejs
#
DEPS_20 += $(CONFIG)/inc/mpr.h
DEPS_20 += $(CONFIG)/inc/me.h
DEPS_20 += $(CONFIG)/inc/osdep.h
DEPS_20 += $(CONFIG)/obj/mprLib.o
DEPS_20 += $(CONFIG)/bin/libmpr.so
DEPS_20 += $(CONFIG)/inc/pcre.h
DEPS_20 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_20 += $(CONFIG)/bin/libpcre.so
endif
DEPS_20 += $(CONFIG)/inc/http.h
DEPS_20 += $(CONFIG)/obj/httpLib.o
DEPS_20 += $(CONFIG)/bin/libhttp.so
DEPS_20 += $(CONFIG)/inc/zlib.h
DEPS_20 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_20 += $(CONFIG)/bin/libzlib.so
endif
DEPS_20 += $(CONFIG)/inc/ejs.h
DEPS_20 += $(CONFIG)/inc/ejs.slots.h
DEPS_20 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_20 += $(CONFIG)/obj/ejsLib.o

LIBS_20 += -lhttp
LIBS_20 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_20 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_20 += -lzlib
endif

$(CONFIG)/bin/libejs.so: $(DEPS_20)
	@echo '      [Link] $(CONFIG)/bin/libejs.so'
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_20) $(LIBS_20) $(LIBS_20) $(LIBS) 
endif

#
#   ejs.o
#
DEPS_21 += $(CONFIG)/inc/me.h
DEPS_21 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_21)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejscmd
#
DEPS_22 += $(CONFIG)/inc/mpr.h
DEPS_22 += $(CONFIG)/inc/me.h
DEPS_22 += $(CONFIG)/inc/osdep.h
DEPS_22 += $(CONFIG)/obj/mprLib.o
DEPS_22 += $(CONFIG)/bin/libmpr.so
DEPS_22 += $(CONFIG)/inc/pcre.h
DEPS_22 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_22 += $(CONFIG)/bin/libpcre.so
endif
DEPS_22 += $(CONFIG)/inc/http.h
DEPS_22 += $(CONFIG)/obj/httpLib.o
DEPS_22 += $(CONFIG)/bin/libhttp.so
DEPS_22 += $(CONFIG)/inc/zlib.h
DEPS_22 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_22 += $(CONFIG)/bin/libzlib.so
endif
DEPS_22 += $(CONFIG)/inc/ejs.h
DEPS_22 += $(CONFIG)/inc/ejs.slots.h
DEPS_22 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_22 += $(CONFIG)/obj/ejsLib.o
DEPS_22 += $(CONFIG)/bin/libejs.so
DEPS_22 += $(CONFIG)/obj/ejs.o

LIBS_22 += -lejs
LIBS_22 += -lhttp
LIBS_22 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_22 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_22 += -lzlib
endif

$(CONFIG)/bin/ejscmd: $(DEPS_22)
	@echo '      [Link] $(CONFIG)/bin/ejscmd'
	$(CC) -o $(CONFIG)/bin/ejscmd $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_22) $(LIBS_22) $(LIBS_22) $(LIBS) $(LIBS) 
endif

#
#   ejsc.o
#
DEPS_23 += $(CONFIG)/inc/me.h
DEPS_23 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_23)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejsc
#
DEPS_24 += $(CONFIG)/inc/mpr.h
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/osdep.h
DEPS_24 += $(CONFIG)/obj/mprLib.o
DEPS_24 += $(CONFIG)/bin/libmpr.so
DEPS_24 += $(CONFIG)/inc/pcre.h
DEPS_24 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_24 += $(CONFIG)/bin/libpcre.so
endif
DEPS_24 += $(CONFIG)/inc/http.h
DEPS_24 += $(CONFIG)/obj/httpLib.o
DEPS_24 += $(CONFIG)/bin/libhttp.so
DEPS_24 += $(CONFIG)/inc/zlib.h
DEPS_24 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_24 += $(CONFIG)/bin/libzlib.so
endif
DEPS_24 += $(CONFIG)/inc/ejs.h
DEPS_24 += $(CONFIG)/inc/ejs.slots.h
DEPS_24 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_24 += $(CONFIG)/obj/ejsLib.o
DEPS_24 += $(CONFIG)/bin/libejs.so
DEPS_24 += $(CONFIG)/obj/ejsc.o

LIBS_24 += -lejs
LIBS_24 += -lhttp
LIBS_24 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_24 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_24 += -lzlib
endif

$(CONFIG)/bin/ejsc: $(DEPS_24)
	@echo '      [Link] $(CONFIG)/bin/ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_24) $(LIBS_24) $(LIBS_24) $(LIBS) $(LIBS) 
endif

ifeq ($(ME_EXT_EJS),1)
#
#   ejs.mod
#
DEPS_25 += src/paks/ejs/ejs.es
DEPS_25 += $(CONFIG)/inc/mpr.h
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/inc/osdep.h
DEPS_25 += $(CONFIG)/obj/mprLib.o
DEPS_25 += $(CONFIG)/bin/libmpr.so
DEPS_25 += $(CONFIG)/inc/pcre.h
DEPS_25 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_25 += $(CONFIG)/bin/libpcre.so
endif
DEPS_25 += $(CONFIG)/inc/http.h
DEPS_25 += $(CONFIG)/obj/httpLib.o
DEPS_25 += $(CONFIG)/bin/libhttp.so
DEPS_25 += $(CONFIG)/inc/zlib.h
DEPS_25 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_25 += $(CONFIG)/bin/libzlib.so
endif
DEPS_25 += $(CONFIG)/inc/ejs.h
DEPS_25 += $(CONFIG)/inc/ejs.slots.h
DEPS_25 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_25 += $(CONFIG)/obj/ejsLib.o
DEPS_25 += $(CONFIG)/bin/libejs.so
DEPS_25 += $(CONFIG)/obj/ejsc.o
DEPS_25 += $(CONFIG)/bin/ejsc

$(CONFIG)/bin/ejs.mod: $(DEPS_25)
	( \
	cd src/paks/ejs; \
	../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_26)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_27 += $(CONFIG)/inc/me.h
DEPS_27 += $(CONFIG)/inc/est.h
DEPS_27 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_27)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_EXT_EST),1)
#
#   libest
#
DEPS_28 += $(CONFIG)/inc/est.h
DEPS_28 += $(CONFIG)/inc/me.h
DEPS_28 += $(CONFIG)/inc/osdep.h
DEPS_28 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.so: $(DEPS_28)
	@echo '      [Link] $(CONFIG)/bin/libest.so'
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   ca-crt
#
DEPS_29 += src/paks/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_29)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp src/paks/est/ca.crt $(CONFIG)/bin/ca.crt

#
#   http.o
#
DEPS_30 += $(CONFIG)/inc/me.h
DEPS_30 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_30)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   httpcmd
#
DEPS_31 += $(CONFIG)/inc/mpr.h
DEPS_31 += $(CONFIG)/inc/me.h
DEPS_31 += $(CONFIG)/inc/osdep.h
DEPS_31 += $(CONFIG)/obj/mprLib.o
DEPS_31 += $(CONFIG)/bin/libmpr.so
DEPS_31 += $(CONFIG)/inc/pcre.h
DEPS_31 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_31 += $(CONFIG)/bin/libpcre.so
endif
DEPS_31 += $(CONFIG)/inc/http.h
DEPS_31 += $(CONFIG)/obj/httpLib.o
DEPS_31 += $(CONFIG)/bin/libhttp.so
DEPS_31 += $(CONFIG)/obj/http.o

LIBS_31 += -lhttp
LIBS_31 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_31 += -lpcre
endif

$(CONFIG)/bin/httpcmd: $(DEPS_31)
	@echo '      [Link] $(CONFIG)/bin/httpcmd'
	$(CC) -o $(CONFIG)/bin/httpcmd $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_31) $(LIBS_31) $(LIBS_31) $(LIBS) $(LIBS) 

#
#   mprSsl.o
#
DEPS_32 += $(CONFIG)/inc/me.h
DEPS_32 += $(CONFIG)/inc/mpr.h
DEPS_32 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_32)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_33 += $(CONFIG)/inc/mpr.h
DEPS_33 += $(CONFIG)/inc/me.h
DEPS_33 += $(CONFIG)/inc/osdep.h
DEPS_33 += $(CONFIG)/obj/mprLib.o
DEPS_33 += $(CONFIG)/bin/libmpr.so
DEPS_33 += $(CONFIG)/inc/est.h
DEPS_33 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_EXT_EST),1)
    DEPS_33 += $(CONFIG)/bin/libest.so
endif
DEPS_33 += $(CONFIG)/obj/mprSsl.o

LIBS_33 += -lmpr
ifeq ($(ME_EXT_EST),1)
    LIBS_33 += -lest
endif

$(CONFIG)/bin/libmprssl.so: $(DEPS_33)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.so'
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_33) $(LIBS_33) $(LIBS_33) $(LIBS) 

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_34)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_35 += $(CONFIG)/inc/me.h
DEPS_35 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_35)
	@echo '   [Compile] $(CONFIG)/obj/sqlite3.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite3.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   libsql
#
DEPS_36 += $(CONFIG)/inc/sqlite3.h
DEPS_36 += $(CONFIG)/inc/me.h
DEPS_36 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsql.so: $(DEPS_36)
	@echo '      [Link] $(CONFIG)/bin/libsql.so'
	$(CC) -shared -o $(CONFIG)/bin/libsql.so $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/sqlite3.o" $(LIBS) 
endif

#
#   sqlite.o
#
DEPS_37 += $(CONFIG)/inc/me.h
DEPS_37 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_37)
	@echo '   [Compile] $(CONFIG)/obj/sqlite.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   sqliteshell
#
DEPS_38 += $(CONFIG)/inc/sqlite3.h
DEPS_38 += $(CONFIG)/inc/me.h
DEPS_38 += $(CONFIG)/obj/sqlite3.o
DEPS_38 += $(CONFIG)/bin/libsql.so
DEPS_38 += $(CONFIG)/obj/sqlite.o

LIBS_38 += -lsql

$(CONFIG)/bin/sqlite: $(DEPS_38)
	@echo '      [Link] $(CONFIG)/bin/sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/sqlite.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) $(LIBS) 
endif

#
#   me.o
#
DEPS_39 += $(CONFIG)/inc/me.h
DEPS_39 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/me.o: \
    src/me.c $(DEPS_39)
	@echo '   [Compile] $(CONFIG)/obj/me.o'
	$(CC) -c -o $(CONFIG)/obj/me.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/me.c

#
#   me
#
DEPS_40 += $(CONFIG)/inc/mpr.h
DEPS_40 += $(CONFIG)/inc/me.h
DEPS_40 += $(CONFIG)/inc/osdep.h
DEPS_40 += $(CONFIG)/obj/mprLib.o
DEPS_40 += $(CONFIG)/bin/libmpr.so
DEPS_40 += $(CONFIG)/inc/pcre.h
DEPS_40 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_40 += $(CONFIG)/bin/libpcre.so
endif
DEPS_40 += $(CONFIG)/inc/http.h
DEPS_40 += $(CONFIG)/obj/httpLib.o
DEPS_40 += $(CONFIG)/bin/libhttp.so
DEPS_40 += $(CONFIG)/inc/zlib.h
DEPS_40 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_40 += $(CONFIG)/bin/libzlib.so
endif
DEPS_40 += $(CONFIG)/inc/ejs.h
DEPS_40 += $(CONFIG)/inc/ejs.slots.h
DEPS_40 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_40 += $(CONFIG)/obj/ejsLib.o
ifeq ($(ME_EXT_EJS),1)
    DEPS_40 += $(CONFIG)/bin/libejs.so
endif
DEPS_40 += $(CONFIG)/obj/me.o

LIBS_40 += -lmpr
LIBS_40 += -lhttp
ifeq ($(ME_EXT_PCRE),1)
    LIBS_40 += -lpcre
endif
ifeq ($(ME_EXT_EJS),1)
    LIBS_40 += -lejs
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_40 += -lzlib
endif

$(CONFIG)/bin/me: $(DEPS_40)
	@echo '      [Link] $(CONFIG)/bin/me'
	$(CC) -o $(CONFIG)/bin/me $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/me.o" $(LIBPATHS_40) $(LIBS_40) $(LIBS_40) $(LIBS) $(LIBS) 

#
#   makeme
#
DEPS_41 += src/configure.es
DEPS_41 += src/gendoc.es
DEPS_41 += src/generate.es
DEPS_41 += src/me.es
DEPS_41 += src/os/freebsd.me
DEPS_41 += src/os/gcc.me
DEPS_41 += src/os/linux.me
DEPS_41 += src/os/macosx.me
DEPS_41 += src/os/solaris.me
DEPS_41 += src/os/unix.me
DEPS_41 += src/os/vxworks.me
DEPS_41 += src/os/windows.me
DEPS_41 += src/probe/appweb.me
DEPS_41 += src/probe/appwebcmd.me
DEPS_41 += src/probe/compiler.me
DEPS_41 += src/probe/doxygen.me
DEPS_41 += src/probe/dsi.me
DEPS_41 += src/probe/dumpbin.me
DEPS_41 += src/probe/ejscmd.me
DEPS_41 += src/probe/est.me
DEPS_41 += src/probe/gzip.me
DEPS_41 += src/probe/htmlmin.me
DEPS_41 += src/probe/httpcmd.me
DEPS_41 += src/probe/lib.me
DEPS_41 += src/probe/link.me
DEPS_41 += src/probe/man.me
DEPS_41 += src/probe/man2html.me
DEPS_41 += src/probe/matrixssl.me
DEPS_41 += src/probe/md5.me
DEPS_41 += src/probe/nanossl.me
DEPS_41 += src/probe/ngmin.me
DEPS_41 += src/probe/openssl.me
DEPS_41 += src/probe/pak.me
DEPS_41 += src/probe/pmaker.me
DEPS_41 += src/probe/ranlib.me
DEPS_41 += src/probe/rc.me
DEPS_41 += src/probe/recess.me
DEPS_41 += src/probe/sqlite.me
DEPS_41 += src/probe/ssl.me
DEPS_41 += src/probe/strip.me
DEPS_41 += src/probe/tidy.me
DEPS_41 += src/probe/uglifyjs.me
DEPS_41 += src/probe/utest.me
DEPS_41 += src/probe/vxworks.me
DEPS_41 += src/probe/winsdk.me
DEPS_41 += src/probe/zip.me
DEPS_41 += src/simple.me
DEPS_41 += src/standard.me
DEPS_41 += src/vstudio.es
DEPS_41 += src/xcode.es

$(CONFIG)/bin/makeme: $(DEPS_41)
	@echo '      [Copy] $(CONFIG)/bin/makeme'
	mkdir -p "$(CONFIG)/bin/makeme"
	cp src/configure.es $(CONFIG)/bin/makeme/configure.es
	cp src/gendoc.es $(CONFIG)/bin/makeme/gendoc.es
	cp src/generate.es $(CONFIG)/bin/makeme/generate.es
	cp src/me.es $(CONFIG)/bin/makeme/me.es
	mkdir -p "$(CONFIG)/bin/makeme/os"
	cp src/os/freebsd.me $(CONFIG)/bin/makeme/os/freebsd.me
	cp src/os/gcc.me $(CONFIG)/bin/makeme/os/gcc.me
	cp src/os/linux.me $(CONFIG)/bin/makeme/os/linux.me
	cp src/os/macosx.me $(CONFIG)/bin/makeme/os/macosx.me
	cp src/os/solaris.me $(CONFIG)/bin/makeme/os/solaris.me
	cp src/os/unix.me $(CONFIG)/bin/makeme/os/unix.me
	cp src/os/vxworks.me $(CONFIG)/bin/makeme/os/vxworks.me
	cp src/os/windows.me $(CONFIG)/bin/makeme/os/windows.me
	mkdir -p "$(CONFIG)/bin/makeme/probe"
	cp src/probe/appweb.me $(CONFIG)/bin/makeme/probe/appweb.me
	cp src/probe/appwebcmd.me $(CONFIG)/bin/makeme/probe/appwebcmd.me
	cp src/probe/compiler.me $(CONFIG)/bin/makeme/probe/compiler.me
	cp src/probe/doxygen.me $(CONFIG)/bin/makeme/probe/doxygen.me
	cp src/probe/dsi.me $(CONFIG)/bin/makeme/probe/dsi.me
	cp src/probe/dumpbin.me $(CONFIG)/bin/makeme/probe/dumpbin.me
	cp src/probe/ejscmd.me $(CONFIG)/bin/makeme/probe/ejscmd.me
	cp src/probe/est.me $(CONFIG)/bin/makeme/probe/est.me
	cp src/probe/gzip.me $(CONFIG)/bin/makeme/probe/gzip.me
	cp src/probe/htmlmin.me $(CONFIG)/bin/makeme/probe/htmlmin.me
	cp src/probe/httpcmd.me $(CONFIG)/bin/makeme/probe/httpcmd.me
	cp src/probe/lib.me $(CONFIG)/bin/makeme/probe/lib.me
	cp src/probe/link.me $(CONFIG)/bin/makeme/probe/link.me
	cp src/probe/man.me $(CONFIG)/bin/makeme/probe/man.me
	cp src/probe/man2html.me $(CONFIG)/bin/makeme/probe/man2html.me
	cp src/probe/matrixssl.me $(CONFIG)/bin/makeme/probe/matrixssl.me
	cp src/probe/md5.me $(CONFIG)/bin/makeme/probe/md5.me
	cp src/probe/nanossl.me $(CONFIG)/bin/makeme/probe/nanossl.me
	cp src/probe/ngmin.me $(CONFIG)/bin/makeme/probe/ngmin.me
	cp src/probe/openssl.me $(CONFIG)/bin/makeme/probe/openssl.me
	cp src/probe/pak.me $(CONFIG)/bin/makeme/probe/pak.me
	cp src/probe/pmaker.me $(CONFIG)/bin/makeme/probe/pmaker.me
	cp src/probe/ranlib.me $(CONFIG)/bin/makeme/probe/ranlib.me
	cp src/probe/rc.me $(CONFIG)/bin/makeme/probe/rc.me
	cp src/probe/recess.me $(CONFIG)/bin/makeme/probe/recess.me
	cp src/probe/sqlite.me $(CONFIG)/bin/makeme/probe/sqlite.me
	cp src/probe/ssl.me $(CONFIG)/bin/makeme/probe/ssl.me
	cp src/probe/strip.me $(CONFIG)/bin/makeme/probe/strip.me
	cp src/probe/tidy.me $(CONFIG)/bin/makeme/probe/tidy.me
	cp src/probe/uglifyjs.me $(CONFIG)/bin/makeme/probe/uglifyjs.me
	cp src/probe/utest.me $(CONFIG)/bin/makeme/probe/utest.me
	cp src/probe/vxworks.me $(CONFIG)/bin/makeme/probe/vxworks.me
	cp src/probe/winsdk.me $(CONFIG)/bin/makeme/probe/winsdk.me
	cp src/probe/zip.me $(CONFIG)/bin/makeme/probe/zip.me
	cp src/simple.me $(CONFIG)/bin/makeme/simple.me
	cp src/standard.me $(CONFIG)/bin/makeme/standard.me
	cp src/vstudio.es $(CONFIG)/bin/makeme/vstudio.es
	cp src/xcode.es $(CONFIG)/bin/makeme/xcode.es

#
#   bower.json
#
DEPS_42 += package.json

bower.json: $(DEPS_42)
	@echo '      [Copy] bower.json'
	mkdir -p "."
	cp package.json bower.json

#
#   stop
#
stop: $(DEPS_43)

#
#   installBinary
#
installBinary: $(DEPS_44)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.8.0" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(CONFIG)/bin/me $(ME_VAPP_PREFIX)/bin/me ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/me" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/me" "$(ME_BIN_PREFIX)/me" ; \
	cp $(CONFIG)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
	cp $(CONFIG)/bin/libest.so $(ME_VAPP_PREFIX)/bin/libest.so ; \
	cp $(CONFIG)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(CONFIG)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(CONFIG)/bin/libmprssl.so $(ME_VAPP_PREFIX)/bin/libmprssl.so ; \
	cp $(CONFIG)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	cp $(CONFIG)/bin/libzlib.so $(ME_VAPP_PREFIX)/bin/libzlib.so ; \
	cp $(CONFIG)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	cp $(CONFIG)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/makeme" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/makeme/makeme" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	)

#
#   start
#
start: $(DEPS_45)

#
#   install
#
DEPS_46 += stop
DEPS_46 += installBinary
DEPS_46 += start

install: $(DEPS_46)

#
#   uninstall
#
DEPS_47 += stop

uninstall: $(DEPS_47)
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

