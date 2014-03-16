#
#   makeme-macosx-default.mk -- Makefile to build Embedthis MakeMe for macosx
#

NAME                  := makeme
VERSION               := 0.8.0
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= macosx
CC                    ?= clang
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

ME_EXT_COMPILER_PATH  ?= clang
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
ME_EXT_SQLITE_PATH    ?= src/paks/sqlite
ME_EXT_SSL_PATH       ?= ssl
ME_EXT_VXWORKS_PATH   ?= $(WIND_BASE)
ME_EXT_WINSDK_PATH    ?= winsdk
ME_EXT_ZIP_PATH       ?= zip
ME_EXT_ZLIB_PATH      ?= src/paks/zlib

export WIND_HOME      ?= $(WIND_BASE)/..

CFLAGS                += -w
DFLAGS                +=  $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_EXT_EJS=$(ME_EXT_EJS) -DME_EXT_EST=$(ME_EXT_EST) -DME_EXT_OPENSSL=$(ME_EXT_OPENSSL) -DME_EXT_PCRE=$(ME_EXT_PCRE) -DME_EXT_SQLITE=$(ME_EXT_SQLITE) -DME_EXT_SSL=$(ME_EXT_SSL) -DME_EXT_ZLIB=$(ME_EXT_ZLIB) 
IFLAGS                += "-I$(CONFIG)/inc"
LDFLAGS               += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS              += -L$(CONFIG)/bin
LIBS                  += -ldl -lpthread -lm

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
    TARGETS           += $(CONFIG)/bin/libejs.dylib
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
    TARGETS           += $(CONFIG)/bin/libest.dylib
endif
TARGETS               += $(CONFIG)/bin/ca.crt
TARGETS               += $(CONFIG)/bin/http
TARGETS               += $(CONFIG)/bin/libmprssl.dylib
ifeq ($(ME_EXT_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/sqlite
endif
TARGETS               += $(CONFIG)/bin/me
TARGETS               += $(CONFIG)/bin

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
	@[ ! -f $(CONFIG)/inc/me.h ] && cp projects/makeme-macosx-default-me.h $(CONFIG)/inc/me.h ; true
	@if ! diff $(CONFIG)/inc/me.h projects/makeme-macosx-default-me.h >/dev/null ; then\
		cp projects/makeme-macosx-default-me.h $(CONFIG)/inc/me.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags

clean:
	rm -f "$(CONFIG)/bin/libejs.dylib"
	rm -f "$(CONFIG)/bin/ejscmd"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/libest.dylib"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libhttp.dylib"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libmpr.dylib"
	rm -f "$(CONFIG)/bin/libmprssl.dylib"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libpcre.dylib"
	rm -f "$(CONFIG)/bin/libsql.dylib"
	rm -f "$(CONFIG)/bin/sqlite"
	rm -f "$(CONFIG)/bin/libzlib.dylib"
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
	cd macosx-x64-release/bin; \
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
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/mprLib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/osdep.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.dylib: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libmpr.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/mprLib.o" $(LIBS) 

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
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/pcre.o -arch $(CC_ARCH) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_EXT_PCRE),1)
#
#   libpcre
#
DEPS_9 += $(CONFIG)/inc/pcre.h
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.dylib: $(DEPS_9)
	@echo '      [Link] $(CONFIG)/bin/libpcre.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib -arch $(CC_ARCH) $(LDFLAGS) -compatibility_version 0.8.0 -current_version 0.8.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/pcre.o" $(LIBS) 
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
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/httpLib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/http/httpLib.c

#
#   libhttp
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/osdep.h
DEPS_12 += $(CONFIG)/obj/mprLib.o
DEPS_12 += $(CONFIG)/bin/libmpr.dylib
DEPS_12 += $(CONFIG)/inc/pcre.h
DEPS_12 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_12 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_12 += $(CONFIG)/inc/http.h
DEPS_12 += $(CONFIG)/obj/httpLib.o

LIBS_12 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_12 += -lpcre
endif

$(CONFIG)/bin/libhttp.dylib: $(DEPS_12)
	@echo '      [Link] $(CONFIG)/bin/libhttp.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/httpLib.o" $(LIBPATHS_12) $(LIBS_12) $(LIBS_12) $(LIBS) 

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
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/zlib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_EXT_ZLIB),1)
#
#   libzlib
#
DEPS_15 += $(CONFIG)/inc/zlib.h
DEPS_15 += $(CONFIG)/inc/me.h
DEPS_15 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.dylib: $(DEPS_15)
	@echo '      [Link] $(CONFIG)/bin/libzlib.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libzlib.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libzlib.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_16)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_17 += $(CONFIG)/inc/me.h
DEPS_17 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_17)
	@echo '   [Compile] $(CONFIG)/obj/sqlite3.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/sqlite3.o -arch $(CC_ARCH) $(IFLAGS) src/paks/sqlite/sqlite3.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   libsql
#
DEPS_18 += $(CONFIG)/inc/sqlite3.h
DEPS_18 += $(CONFIG)/inc/me.h
DEPS_18 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsql.dylib: $(DEPS_18)
	@echo '      [Link] $(CONFIG)/bin/libsql.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libsql.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libsql.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/sqlite3.o" $(LIBS) 
endif

#
#   ejs.h
#
$(CONFIG)/inc/ejs.h: $(DEPS_19)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.h $(CONFIG)/inc/ejs.h

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_20)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_21)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/ejs/ejsByteGoto.h $(CONFIG)/inc/ejsByteGoto.h

#
#   ejsLib.o
#
DEPS_22 += $(CONFIG)/inc/me.h
DEPS_22 += $(CONFIG)/inc/ejs.h
DEPS_22 += $(CONFIG)/inc/mpr.h
DEPS_22 += $(CONFIG)/inc/pcre.h
DEPS_22 += $(CONFIG)/inc/osdep.h
DEPS_22 += $(CONFIG)/inc/http.h
DEPS_22 += $(CONFIG)/inc/ejs.slots.h
DEPS_22 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_22)
	@echo '   [Compile] $(CONFIG)/obj/ejsLib.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/ejsLib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(ME_EXT_EJS),1)
#
#   libejs
#
DEPS_23 += $(CONFIG)/inc/mpr.h
DEPS_23 += $(CONFIG)/inc/me.h
DEPS_23 += $(CONFIG)/inc/osdep.h
DEPS_23 += $(CONFIG)/obj/mprLib.o
DEPS_23 += $(CONFIG)/bin/libmpr.dylib
DEPS_23 += $(CONFIG)/inc/pcre.h
DEPS_23 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_23 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_23 += $(CONFIG)/inc/http.h
DEPS_23 += $(CONFIG)/obj/httpLib.o
DEPS_23 += $(CONFIG)/bin/libhttp.dylib
DEPS_23 += $(CONFIG)/inc/zlib.h
DEPS_23 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_23 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_23 += $(CONFIG)/inc/sqlite3.h
DEPS_23 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_23 += $(CONFIG)/bin/libsql.dylib
endif
DEPS_23 += $(CONFIG)/inc/ejs.h
DEPS_23 += $(CONFIG)/inc/ejs.slots.h
DEPS_23 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_23 += $(CONFIG)/obj/ejsLib.o

LIBS_23 += -lhttp
LIBS_23 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_23 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_23 += -lzlib
endif
ifeq ($(ME_EXT_SQLITE),1)
    LIBS_23 += -lsql
endif

$(CONFIG)/bin/libejs.dylib: $(DEPS_23)
	@echo '      [Link] $(CONFIG)/bin/libejs.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libejs.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_23) $(LIBS_23) $(LIBS_23) $(LIBS) 
endif

#
#   ejs.o
#
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_24)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/ejs.o -arch $(CC_ARCH) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejscmd
#
DEPS_25 += $(CONFIG)/inc/mpr.h
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/inc/osdep.h
DEPS_25 += $(CONFIG)/obj/mprLib.o
DEPS_25 += $(CONFIG)/bin/libmpr.dylib
DEPS_25 += $(CONFIG)/inc/pcre.h
DEPS_25 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_25 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_25 += $(CONFIG)/inc/http.h
DEPS_25 += $(CONFIG)/obj/httpLib.o
DEPS_25 += $(CONFIG)/bin/libhttp.dylib
DEPS_25 += $(CONFIG)/inc/zlib.h
DEPS_25 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_25 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_25 += $(CONFIG)/inc/sqlite3.h
DEPS_25 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_25 += $(CONFIG)/bin/libsql.dylib
endif
DEPS_25 += $(CONFIG)/inc/ejs.h
DEPS_25 += $(CONFIG)/inc/ejs.slots.h
DEPS_25 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_25 += $(CONFIG)/obj/ejsLib.o
DEPS_25 += $(CONFIG)/bin/libejs.dylib
DEPS_25 += $(CONFIG)/obj/ejs.o

LIBS_25 += -lejs
LIBS_25 += -lhttp
LIBS_25 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_25 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_25 += -lzlib
endif
ifeq ($(ME_EXT_SQLITE),1)
    LIBS_25 += -lsql
endif

$(CONFIG)/bin/ejscmd: $(DEPS_25)
	@echo '      [Link] $(CONFIG)/bin/ejscmd'
	$(CC) -o $(CONFIG)/bin/ejscmd -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_25) $(LIBS_25) $(LIBS_25) $(LIBS) -ledit 
endif

#
#   ejsc.o
#
DEPS_26 += $(CONFIG)/inc/me.h
DEPS_26 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_26)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/ejsc.o -arch $(CC_ARCH) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejsc
#
DEPS_27 += $(CONFIG)/inc/mpr.h
DEPS_27 += $(CONFIG)/inc/me.h
DEPS_27 += $(CONFIG)/inc/osdep.h
DEPS_27 += $(CONFIG)/obj/mprLib.o
DEPS_27 += $(CONFIG)/bin/libmpr.dylib
DEPS_27 += $(CONFIG)/inc/pcre.h
DEPS_27 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_27 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_27 += $(CONFIG)/inc/http.h
DEPS_27 += $(CONFIG)/obj/httpLib.o
DEPS_27 += $(CONFIG)/bin/libhttp.dylib
DEPS_27 += $(CONFIG)/inc/zlib.h
DEPS_27 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_27 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_27 += $(CONFIG)/inc/sqlite3.h
DEPS_27 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_27 += $(CONFIG)/bin/libsql.dylib
endif
DEPS_27 += $(CONFIG)/inc/ejs.h
DEPS_27 += $(CONFIG)/inc/ejs.slots.h
DEPS_27 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_27 += $(CONFIG)/obj/ejsLib.o
DEPS_27 += $(CONFIG)/bin/libejs.dylib
DEPS_27 += $(CONFIG)/obj/ejsc.o

LIBS_27 += -lejs
LIBS_27 += -lhttp
LIBS_27 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_27 += -lpcre
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_27 += -lzlib
endif
ifeq ($(ME_EXT_SQLITE),1)
    LIBS_27 += -lsql
endif

$(CONFIG)/bin/ejsc: $(DEPS_27)
	@echo '      [Link] $(CONFIG)/bin/ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_27) $(LIBS_27) $(LIBS_27) $(LIBS) 
endif

ifeq ($(ME_EXT_EJS),1)
#
#   ejs.mod
#
DEPS_28 += src/paks/ejs/ejs.es
DEPS_28 += $(CONFIG)/inc/mpr.h
DEPS_28 += $(CONFIG)/inc/me.h
DEPS_28 += $(CONFIG)/inc/osdep.h
DEPS_28 += $(CONFIG)/obj/mprLib.o
DEPS_28 += $(CONFIG)/bin/libmpr.dylib
DEPS_28 += $(CONFIG)/inc/pcre.h
DEPS_28 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_28 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_28 += $(CONFIG)/inc/http.h
DEPS_28 += $(CONFIG)/obj/httpLib.o
DEPS_28 += $(CONFIG)/bin/libhttp.dylib
DEPS_28 += $(CONFIG)/inc/zlib.h
DEPS_28 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_28 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_28 += $(CONFIG)/inc/sqlite3.h
DEPS_28 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_28 += $(CONFIG)/bin/libsql.dylib
endif
DEPS_28 += $(CONFIG)/inc/ejs.h
DEPS_28 += $(CONFIG)/inc/ejs.slots.h
DEPS_28 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_28 += $(CONFIG)/obj/ejsLib.o
DEPS_28 += $(CONFIG)/bin/libejs.dylib
DEPS_28 += $(CONFIG)/obj/ejsc.o
DEPS_28 += $(CONFIG)/bin/ejsc

$(CONFIG)/bin/ejs.mod: $(DEPS_28)
	( \
	cd src/paks/ejs; \
	../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_29)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_30 += $(CONFIG)/inc/me.h
DEPS_30 += $(CONFIG)/inc/est.h
DEPS_30 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_30)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/estLib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_EXT_EST),1)
#
#   libest
#
DEPS_31 += $(CONFIG)/inc/est.h
DEPS_31 += $(CONFIG)/inc/me.h
DEPS_31 += $(CONFIG)/inc/osdep.h
DEPS_31 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.dylib: $(DEPS_31)
	@echo '      [Link] $(CONFIG)/bin/libest.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libest.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   ca-crt
#
DEPS_32 += src/paks/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_32)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp src/paks/est/ca.crt $(CONFIG)/bin/ca.crt

#
#   http.o
#
DEPS_33 += $(CONFIG)/inc/me.h
DEPS_33 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_33)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/http.o -arch $(CC_ARCH) $(IFLAGS) src/paks/http/http.c

#
#   httpcmd
#
DEPS_34 += $(CONFIG)/inc/mpr.h
DEPS_34 += $(CONFIG)/inc/me.h
DEPS_34 += $(CONFIG)/inc/osdep.h
DEPS_34 += $(CONFIG)/obj/mprLib.o
DEPS_34 += $(CONFIG)/bin/libmpr.dylib
DEPS_34 += $(CONFIG)/inc/pcre.h
DEPS_34 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_34 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_34 += $(CONFIG)/inc/http.h
DEPS_34 += $(CONFIG)/obj/httpLib.o
DEPS_34 += $(CONFIG)/bin/libhttp.dylib
DEPS_34 += $(CONFIG)/obj/http.o

LIBS_34 += -lhttp
LIBS_34 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_34 += -lpcre
endif

$(CONFIG)/bin/http: $(DEPS_34)
	@echo '      [Link] $(CONFIG)/bin/http'
	$(CC) -o $(CONFIG)/bin/http -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_34) $(LIBS_34) $(LIBS_34) $(LIBS) 

#
#   mprSsl.o
#
DEPS_35 += $(CONFIG)/inc/me.h
DEPS_35 += $(CONFIG)/inc/mpr.h
DEPS_35 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_35)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/mprSsl.o -arch $(CC_ARCH) $(IFLAGS) src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_36 += $(CONFIG)/inc/mpr.h
DEPS_36 += $(CONFIG)/inc/me.h
DEPS_36 += $(CONFIG)/inc/osdep.h
DEPS_36 += $(CONFIG)/obj/mprLib.o
DEPS_36 += $(CONFIG)/bin/libmpr.dylib
DEPS_36 += $(CONFIG)/inc/est.h
DEPS_36 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_EXT_EST),1)
    DEPS_36 += $(CONFIG)/bin/libest.dylib
endif
DEPS_36 += $(CONFIG)/obj/mprSsl.o

LIBS_36 += -lmpr
ifeq ($(ME_EXT_EST),1)
    LIBS_36 += -lest
endif

$(CONFIG)/bin/libmprssl.dylib: $(DEPS_36)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmprssl.dylib -compatibility_version 0.8.0 -current_version 0.8.0 "$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_36) $(LIBS_36) $(LIBS_36) $(LIBS) 

#
#   sqlite.o
#
DEPS_37 += $(CONFIG)/inc/me.h
DEPS_37 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_37)
	@echo '   [Compile] $(CONFIG)/obj/sqlite.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/sqlite.o -arch $(CC_ARCH) $(IFLAGS) src/paks/sqlite/sqlite.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   sqliteshell
#
DEPS_38 += $(CONFIG)/inc/sqlite3.h
DEPS_38 += $(CONFIG)/inc/me.h
DEPS_38 += $(CONFIG)/obj/sqlite3.o
DEPS_38 += $(CONFIG)/bin/libsql.dylib
DEPS_38 += $(CONFIG)/obj/sqlite.o

LIBS_38 += -lsql

$(CONFIG)/bin/sqlite: $(DEPS_38)
	@echo '      [Link] $(CONFIG)/bin/sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/sqlite.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) 
endif

#
#   me.o
#
DEPS_39 += $(CONFIG)/inc/me.h
DEPS_39 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/me.o: \
    src/me.c $(DEPS_39)
	@echo '   [Compile] $(CONFIG)/obj/me.o'
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $(CONFIG)/obj/me.o -arch $(CC_ARCH) $(IFLAGS) src/me.c

#
#   me
#
DEPS_40 += $(CONFIG)/inc/mpr.h
DEPS_40 += $(CONFIG)/inc/me.h
DEPS_40 += $(CONFIG)/inc/osdep.h
DEPS_40 += $(CONFIG)/obj/mprLib.o
DEPS_40 += $(CONFIG)/bin/libmpr.dylib
DEPS_40 += $(CONFIG)/inc/pcre.h
DEPS_40 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_40 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_40 += $(CONFIG)/inc/http.h
DEPS_40 += $(CONFIG)/obj/httpLib.o
DEPS_40 += $(CONFIG)/bin/libhttp.dylib
DEPS_40 += $(CONFIG)/inc/zlib.h
DEPS_40 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_40 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_40 += $(CONFIG)/inc/sqlite3.h
DEPS_40 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_40 += $(CONFIG)/bin/libsql.dylib
endif
DEPS_40 += $(CONFIG)/inc/ejs.h
DEPS_40 += $(CONFIG)/inc/ejs.slots.h
DEPS_40 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_40 += $(CONFIG)/obj/ejsLib.o
ifeq ($(ME_EXT_EJS),1)
    DEPS_40 += $(CONFIG)/bin/libejs.dylib
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
ifeq ($(ME_EXT_SQLITE),1)
    LIBS_40 += -lsql
endif

$(CONFIG)/bin/me: $(DEPS_40)
	@echo '      [Link] $(CONFIG)/bin/me'
	$(CC) -o $(CONFIG)/bin/me -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/me.o" $(LIBPATHS_40) $(LIBS_40) $(LIBS_40) $(LIBS) 

#
#   me-core
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

$(CONFIG)/bin: $(DEPS_41)
	@echo '      [Copy] $(CONFIG)/bin'
	mkdir -p "$(CONFIG)/bin"
	cp src/configure.es $(CONFIG)/bin/configure.es
	cp src/gendoc.es $(CONFIG)/bin/gendoc.es
	cp src/generate.es $(CONFIG)/bin/generate.es
	cp src/me.es $(CONFIG)/bin/me.es
	mkdir -p "$(CONFIG)/bin/os"
	cp src/os/freebsd.me $(CONFIG)/bin/os/freebsd.me
	cp src/os/gcc.me $(CONFIG)/bin/os/gcc.me
	cp src/os/linux.me $(CONFIG)/bin/os/linux.me
	cp src/os/macosx.me $(CONFIG)/bin/os/macosx.me
	cp src/os/solaris.me $(CONFIG)/bin/os/solaris.me
	cp src/os/unix.me $(CONFIG)/bin/os/unix.me
	cp src/os/vxworks.me $(CONFIG)/bin/os/vxworks.me
	cp src/os/windows.me $(CONFIG)/bin/os/windows.me
	mkdir -p "$(CONFIG)/bin/probe"
	cp src/probe/appweb.me $(CONFIG)/bin/probe/appweb.me
	cp src/probe/appwebcmd.me $(CONFIG)/bin/probe/appwebcmd.me
	cp src/probe/compiler.me $(CONFIG)/bin/probe/compiler.me
	cp src/probe/doxygen.me $(CONFIG)/bin/probe/doxygen.me
	cp src/probe/dsi.me $(CONFIG)/bin/probe/dsi.me
	cp src/probe/dumpbin.me $(CONFIG)/bin/probe/dumpbin.me
	cp src/probe/ejscmd.me $(CONFIG)/bin/probe/ejscmd.me
	cp src/probe/est.me $(CONFIG)/bin/probe/est.me
	cp src/probe/gzip.me $(CONFIG)/bin/probe/gzip.me
	cp src/probe/htmlmin.me $(CONFIG)/bin/probe/htmlmin.me
	cp src/probe/httpcmd.me $(CONFIG)/bin/probe/httpcmd.me
	cp src/probe/lib.me $(CONFIG)/bin/probe/lib.me
	cp src/probe/link.me $(CONFIG)/bin/probe/link.me
	cp src/probe/man.me $(CONFIG)/bin/probe/man.me
	cp src/probe/man2html.me $(CONFIG)/bin/probe/man2html.me
	cp src/probe/matrixssl.me $(CONFIG)/bin/probe/matrixssl.me
	cp src/probe/md5.me $(CONFIG)/bin/probe/md5.me
	cp src/probe/nanossl.me $(CONFIG)/bin/probe/nanossl.me
	cp src/probe/ngmin.me $(CONFIG)/bin/probe/ngmin.me
	cp src/probe/openssl.me $(CONFIG)/bin/probe/openssl.me
	cp src/probe/pak.me $(CONFIG)/bin/probe/pak.me
	cp src/probe/pmaker.me $(CONFIG)/bin/probe/pmaker.me
	cp src/probe/ranlib.me $(CONFIG)/bin/probe/ranlib.me
	cp src/probe/rc.me $(CONFIG)/bin/probe/rc.me
	cp src/probe/recess.me $(CONFIG)/bin/probe/recess.me
	cp src/probe/sqlite.me $(CONFIG)/bin/probe/sqlite.me
	cp src/probe/ssl.me $(CONFIG)/bin/probe/ssl.me
	cp src/probe/strip.me $(CONFIG)/bin/probe/strip.me
	cp src/probe/tidy.me $(CONFIG)/bin/probe/tidy.me
	cp src/probe/uglifyjs.me $(CONFIG)/bin/probe/uglifyjs.me
	cp src/probe/utest.me $(CONFIG)/bin/probe/utest.me
	cp src/probe/vxworks.me $(CONFIG)/bin/probe/vxworks.me
	cp src/probe/winsdk.me $(CONFIG)/bin/probe/winsdk.me
	cp src/probe/zip.me $(CONFIG)/bin/probe/zip.me
	cp src/simple.me $(CONFIG)/bin/simple.me
	cp src/standard.me $(CONFIG)/bin/standard.me
	cp src/vstudio.es $(CONFIG)/bin/vstudio.es
	cp src/xcode.es $(CONFIG)/bin/xcode.es

#
#   stop
#
stop: $(DEPS_42)

#
#   installBinary
#
installBinary: $(DEPS_43)
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
	cp $(CONFIG)/bin/libejs.dylib $(ME_VAPP_PREFIX)/bin/libejs.dylib ; \
	cp $(CONFIG)/bin/libest.dylib $(ME_VAPP_PREFIX)/bin/libest.dylib ; \
	cp $(CONFIG)/bin/libhttp.dylib $(ME_VAPP_PREFIX)/bin/libhttp.dylib ; \
	cp $(CONFIG)/bin/libmpr.dylib $(ME_VAPP_PREFIX)/bin/libmpr.dylib ; \
	cp $(CONFIG)/bin/libmprssl.dylib $(ME_VAPP_PREFIX)/bin/libmprssl.dylib ; \
	cp $(CONFIG)/bin/libpcre.dylib $(ME_VAPP_PREFIX)/bin/libpcre.dylib ; \
	cp $(CONFIG)/bin/libzlib.dylib $(ME_VAPP_PREFIX)/bin/libzlib.dylib ; \
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
start: $(DEPS_44)

#
#   install
#
DEPS_45 += stop
DEPS_45 += installBinary
DEPS_45 += start

install: $(DEPS_45)

#
#   uninstall
#
DEPS_46 += stop

uninstall: $(DEPS_46)
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

