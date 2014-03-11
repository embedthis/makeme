#
#   makeme-macosx-default.mk -- Makefile to build MakeMe for macosx
#

PRODUCT            := makeme
VERSION            := 0.9.4
PROFILE            := default
ARCH               := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH            := $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                 := macosx
CC                 := clang
LD                 := link
CONFIG             := $(OS)-$(ARCH)-$(PROFILE)
LBIN               := $(CONFIG)/bin

ME_EXT_EJS       := 1
ME_EXT_EST       := 1
ME_EXT_LIBEST    := 0
ME_EXT_MATRIXSSL := 0
ME_EXT_NANOSSL   := 0
ME_EXT_OPENSSL   := 0
ME_EXT_PCRE      := 1
ME_EXT_SSL       := 1
ME_EXT_ZLIB      := 1

ifeq ($(ME_EXT_EST),1)
    ME_EXT_SSL := 1
endif
ifeq ($(ME_EXT_LIB),1)
    ME_EXT_COMPILER := 1
endif
ifeq ($(ME_EXT_MATRIXSSL),1)
    ME_EXT_SSL := 1
endif
ifeq ($(ME_EXT_NANOSSL),1)
    ME_EXT_SSL := 1
endif
ifeq ($(ME_EXT_OPENSSL),1)
    ME_EXT_SSL := 1
endif

ME_EXT_ME_PATH         := me
ME_EXT_BITOS_PATH       := bitos
ME_EXT_COMPILER_PATH    := clang
ME_EXT_DSI_PATH         := dsi
ME_EXT_EJS_PATH         := ejs
ME_EXT_ESP_PATH         := esp
ME_EXT_EST_PATH         := est
ME_EXT_HTTP_PATH        := http
ME_EXT_LIB_PATH         := ar
ME_EXT_LINK_PATH        := link
ME_EXT_MAN_PATH         := man
ME_EXT_MAN2HTML_PATH    := man2html
ME_EXT_MATRIXSSL_PATH   := /usr/src/matrixssl
ME_EXT_MPR_PATH         := mpr
ME_EXT_NANOSSL_PATH     := /usr/src/nanossl
ME_EXT_OPENSSL_PATH     := /usr/src/openssl
ME_EXT_PCRE_PATH        := pcre
ME_EXT_PMAKER_PATH      := pmaker
ME_EXT_SQLITE_PATH      := sqlite
ME_EXT_SSL_PATH         := ssl
ME_EXT_ZIP_PATH         := zip
ME_EXT_ZLIB_PATH        := zlib

CFLAGS             += -Wunreachable-code -w
DFLAGS             +=  $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_EXT_EJS=$(ME_EXT_EJS) -DME_EXT_EST=$(ME_EXT_EST) -DME_EXT_LIBEST=$(ME_EXT_LIBEST) -DME_EXT_MATRIXSSL=$(ME_EXT_MATRIXSSL) -DME_EXT_NANOSSL=$(ME_EXT_NANOSSL) -DME_EXT_OPENSSL=$(ME_EXT_OPENSSL) -DME_EXT_PCRE=$(ME_EXT_PCRE) -DME_EXT_SSL=$(ME_EXT_SSL) -DME_EXT_ZLIB=$(ME_EXT_ZLIB) 
IFLAGS             += "-I$(CONFIG)/inc"
LDFLAGS            += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS           += -L$(CONFIG)/bin
LIBS               += -ldl -lpthread -lm

DEBUG              := debug
CFLAGS-debug       := -g
DFLAGS-debug       := -DME_DEBUG
LDFLAGS-debug      := -g
DFLAGS-release     := 
CFLAGS-release     := -O2
LDFLAGS-release    := 
CFLAGS             += $(CFLAGS-$(DEBUG))
DFLAGS             += $(DFLAGS-$(DEBUG))
LDFLAGS            += $(LDFLAGS-$(DEBUG))

ME_ROOT_PREFIX    := 
ME_BASE_PREFIX    := $(ME_ROOT_PREFIX)/usr/local
ME_DATA_PREFIX    := $(ME_ROOT_PREFIX)/
ME_STATE_PREFIX   := $(ME_ROOT_PREFIX)/var
ME_APP_PREFIX     := $(ME_BASE_PREFIX)/lib/$(PRODUCT)
ME_VAPP_PREFIX    := $(ME_APP_PREFIX)/$(VERSION)
ME_BIN_PREFIX     := $(ME_ROOT_PREFIX)/usr/local/bin
ME_INC_PREFIX     := $(ME_ROOT_PREFIX)/usr/local/include
ME_LIB_PREFIX     := $(ME_ROOT_PREFIX)/usr/local/lib
ME_MAN_PREFIX     := $(ME_ROOT_PREFIX)/usr/local/share/man
ME_SBIN_PREFIX    := $(ME_ROOT_PREFIX)/usr/local/sbin
ME_ETC_PREFIX     := $(ME_ROOT_PREFIX)/etc/$(PRODUCT)
ME_WEB_PREFIX     := $(ME_ROOT_PREFIX)/var/www/$(PRODUCT)-default
ME_LOG_PREFIX     := $(ME_ROOT_PREFIX)/var/log/$(PRODUCT)
ME_SPOOL_PREFIX   := $(ME_ROOT_PREFIX)/var/spool/$(PRODUCT)
ME_CACHE_PREFIX   := $(ME_ROOT_PREFIX)/var/spool/$(PRODUCT)/cache
ME_SRC_PREFIX     := $(ME_ROOT_PREFIX)$(PRODUCT)-$(VERSION)


ifeq ($(ME_EXT_EJS),1)
TARGETS            += $(CONFIG)/bin/libejs.dylib
endif
ifeq ($(ME_EXT_EJS),1)
TARGETS            += $(CONFIG)/bin/ejs
endif
ifeq ($(ME_EXT_EJS),1)
TARGETS            += $(CONFIG)/bin/ejsc
endif
ifeq ($(ME_EXT_EJS),1)
TARGETS            += $(CONFIG)/bin/ejs.mod
endif
ifeq ($(ME_EXT_EST),1)
TARGETS            += $(CONFIG)/bin/libest.dylib
endif
TARGETS            += $(CONFIG)/bin/ca.crt
TARGETS            += $(CONFIG)/bin/http
TARGETS            += $(CONFIG)/bin/libmprssl.dylib
TARGETS            += $(CONFIG)/bin/me
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
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/paks/bitos/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bitos.h src/paks/bitos/bitos.h >/dev/null ; then\
		cp src/paks/bitos/bitos.h $(CONFIG)/inc/bitos.h  ; \
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
	rm -f "$(CONFIG)/bin/ejs"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/libest.dylib"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libhttp.dylib"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libmpr.dylib"
	rm -f "$(CONFIG)/bin/libmprssl.dylib"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libpcre.dylib"
	rm -f "$(CONFIG)/bin/libzlib.dylib"
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
	rm -f "$(CONFIG)/obj/zlib.o"
	rm -f "$(CONFIG)/obj/me.o"

clobber: clean
	rm -fr ./$(CONFIG)



#
#   version
#
version: $(DEPS_1)
	( \
	cd me; \
	echo 0.9.4 ; \
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
#   bitos.h
#
$(CONFIG)/inc/bitos.h: $(DEPS_4)
	@echo '      [Copy] $(CONFIG)/inc/bitos.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/bitos/bitos.h $(CONFIG)/inc/bitos.h

#
#   mprLib.o
#
DEPS_5 += $(CONFIG)/inc/me.h
DEPS_5 += $(CONFIG)/inc/mpr.h
DEPS_5 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_5)
	@echo '   [Compile] $(CONFIG)/obj/mprLib.o'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/bitos.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.dylib: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libmpr.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/mprLib.o" $(LIBS) 

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
	$(CC) -c -o $(CONFIG)/obj/pcre.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_EXT_PCRE),1)
#
#   libpcre
#
DEPS_9 += $(CONFIG)/inc/pcre.h
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.dylib: $(DEPS_9)
	@echo '      [Link] $(CONFIG)/bin/libpcre.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib -arch $(CC_ARCH) $(LDFLAGS) -compatibility_version 0.9.4 -current_version 0.9.4 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/pcre.o" $(LIBS) 
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
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

#
#   libhttp
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/bitos.h
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
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/httpLib.o" $(LIBPATHS_12) $(LIBS_12) $(LIBS_12) $(LIBS) 

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
DEPS_14 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_14)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o -arch $(CC_ARCH) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_EXT_ZLIB),1)
#
#   libzlib
#
DEPS_15 += $(CONFIG)/inc/zlib.h
DEPS_15 += $(CONFIG)/inc/me.h
DEPS_15 += $(CONFIG)/inc/bitos.h
DEPS_15 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.dylib: $(DEPS_15)
	@echo '      [Link] $(CONFIG)/bin/libzlib.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libzlib.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libzlib.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/zlib.o" $(LIBS) 
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
DEPS_19 += $(CONFIG)/inc/bitos.h
DEPS_19 += $(CONFIG)/inc/http.h
DEPS_19 += $(CONFIG)/inc/ejs.slots.h
DEPS_19 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_19)
	@echo '   [Compile] $(CONFIG)/obj/ejsLib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(ME_EXT_EJS),1)
#
#   libejs
#
DEPS_20 += $(CONFIG)/inc/mpr.h
DEPS_20 += $(CONFIG)/inc/me.h
DEPS_20 += $(CONFIG)/inc/bitos.h
DEPS_20 += $(CONFIG)/obj/mprLib.o
DEPS_20 += $(CONFIG)/bin/libmpr.dylib
DEPS_20 += $(CONFIG)/inc/pcre.h
DEPS_20 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_20 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_20 += $(CONFIG)/inc/http.h
DEPS_20 += $(CONFIG)/obj/httpLib.o
DEPS_20 += $(CONFIG)/bin/libhttp.dylib
DEPS_20 += $(CONFIG)/inc/zlib.h
DEPS_20 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_20 += $(CONFIG)/bin/libzlib.dylib
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

$(CONFIG)/bin/libejs.dylib: $(DEPS_20)
	@echo '      [Link] $(CONFIG)/bin/libejs.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libejs.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_20) $(LIBS_20) $(LIBS_20) $(LIBS) 
endif

#
#   ejs.o
#
DEPS_21 += $(CONFIG)/inc/me.h
DEPS_21 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_21)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejs
#
DEPS_22 += $(CONFIG)/inc/mpr.h
DEPS_22 += $(CONFIG)/inc/me.h
DEPS_22 += $(CONFIG)/inc/bitos.h
DEPS_22 += $(CONFIG)/obj/mprLib.o
DEPS_22 += $(CONFIG)/bin/libmpr.dylib
DEPS_22 += $(CONFIG)/inc/pcre.h
DEPS_22 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_22 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_22 += $(CONFIG)/inc/http.h
DEPS_22 += $(CONFIG)/obj/httpLib.o
DEPS_22 += $(CONFIG)/bin/libhttp.dylib
DEPS_22 += $(CONFIG)/inc/zlib.h
DEPS_22 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_22 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_22 += $(CONFIG)/inc/ejs.h
DEPS_22 += $(CONFIG)/inc/ejs.slots.h
DEPS_22 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_22 += $(CONFIG)/obj/ejsLib.o
DEPS_22 += $(CONFIG)/bin/libejs.dylib
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

$(CONFIG)/bin/ejs: $(DEPS_22)
	@echo '      [Link] $(CONFIG)/bin/ejs'
	$(CC) -o $(CONFIG)/bin/ejs -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_22) $(LIBS_22) $(LIBS_22) $(LIBS) -ledit 
endif

#
#   ejsc.o
#
DEPS_23 += $(CONFIG)/inc/me.h
DEPS_23 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_23)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(ME_EXT_EJS),1)
#
#   ejsc
#
DEPS_24 += $(CONFIG)/inc/mpr.h
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/bitos.h
DEPS_24 += $(CONFIG)/obj/mprLib.o
DEPS_24 += $(CONFIG)/bin/libmpr.dylib
DEPS_24 += $(CONFIG)/inc/pcre.h
DEPS_24 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_24 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_24 += $(CONFIG)/inc/http.h
DEPS_24 += $(CONFIG)/obj/httpLib.o
DEPS_24 += $(CONFIG)/bin/libhttp.dylib
DEPS_24 += $(CONFIG)/inc/zlib.h
DEPS_24 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_24 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_24 += $(CONFIG)/inc/ejs.h
DEPS_24 += $(CONFIG)/inc/ejs.slots.h
DEPS_24 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_24 += $(CONFIG)/obj/ejsLib.o
DEPS_24 += $(CONFIG)/bin/libejs.dylib
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
	$(CC) -o $(CONFIG)/bin/ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_24) $(LIBS_24) $(LIBS_24) $(LIBS) 
endif

ifeq ($(ME_EXT_EJS),1)
#
#   ejs.mod
#
DEPS_25 += src/paks/ejs/ejs.es
DEPS_25 += $(CONFIG)/inc/mpr.h
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/inc/bitos.h
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
DEPS_25 += $(CONFIG)/inc/ejs.h
DEPS_25 += $(CONFIG)/inc/ejs.slots.h
DEPS_25 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_25 += $(CONFIG)/obj/ejsLib.o
DEPS_25 += $(CONFIG)/bin/libejs.dylib
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
DEPS_27 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_27)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o -arch $(CC_ARCH) -Wunreachable-code $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_EXT_EST),1)
#
#   libest
#
DEPS_28 += $(CONFIG)/inc/est.h
DEPS_28 += $(CONFIG)/inc/me.h
DEPS_28 += $(CONFIG)/inc/bitos.h
DEPS_28 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.dylib: $(DEPS_28)
	@echo '      [Link] $(CONFIG)/bin/libest.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libest.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/estLib.o" $(LIBS) 
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
	$(CC) -c -o $(CONFIG)/obj/http.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   http
#
DEPS_31 += $(CONFIG)/inc/mpr.h
DEPS_31 += $(CONFIG)/inc/me.h
DEPS_31 += $(CONFIG)/inc/bitos.h
DEPS_31 += $(CONFIG)/obj/mprLib.o
DEPS_31 += $(CONFIG)/bin/libmpr.dylib
DEPS_31 += $(CONFIG)/inc/pcre.h
DEPS_31 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_31 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_31 += $(CONFIG)/inc/http.h
DEPS_31 += $(CONFIG)/obj/httpLib.o
DEPS_31 += $(CONFIG)/bin/libhttp.dylib
DEPS_31 += $(CONFIG)/obj/http.o

LIBS_31 += -lhttp
LIBS_31 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_31 += -lpcre
endif

$(CONFIG)/bin/http: $(DEPS_31)
	@echo '      [Link] $(CONFIG)/bin/http'
	$(CC) -o $(CONFIG)/bin/http -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_31) $(LIBS_31) $(LIBS_31) $(LIBS) 

#
#   mprSsl.o
#
DEPS_32 += $(CONFIG)/inc/me.h
DEPS_32 += $(CONFIG)/inc/mpr.h
DEPS_32 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_32)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_33 += $(CONFIG)/inc/mpr.h
DEPS_33 += $(CONFIG)/inc/me.h
DEPS_33 += $(CONFIG)/inc/bitos.h
DEPS_33 += $(CONFIG)/obj/mprLib.o
DEPS_33 += $(CONFIG)/bin/libmpr.dylib
DEPS_33 += $(CONFIG)/inc/est.h
DEPS_33 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_EXT_EST),1)
    DEPS_33 += $(CONFIG)/bin/libest.dylib
endif
DEPS_33 += $(CONFIG)/obj/mprSsl.o

LIBS_33 += -lmpr
ifeq ($(ME_EXT_EST),1)
    LIBS_33 += -lest
endif

$(CONFIG)/bin/libmprssl.dylib: $(DEPS_33)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.dylib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmprssl.dylib -compatibility_version 0.9.4 -current_version 0.9.4 "$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_33) $(LIBS_33) $(LIBS_33) $(LIBS) 

#
#   me
#
DEPS_34 += src/me.es
DEPS_34 += src/configure.es
DEPS_34 += src/embedthis-manifest.me
DEPS_34 += src/embedthis.me
DEPS_34 += src/embedthis.es
DEPS_34 += src/gendoc.es
DEPS_34 += src/generate.es
DEPS_34 += src/os
DEPS_34 += src/os/freebsd.me
DEPS_34 += src/os/gcc.me
DEPS_34 += src/os/linux.me
DEPS_34 += src/os/macosx.me
DEPS_34 += src/os/solaris.me
DEPS_34 += src/os/unix.me
DEPS_34 += src/os/vxworks.me
DEPS_34 += src/os/windows.me
DEPS_34 += src/probes
DEPS_34 += src/probes/appwebcmd.me
DEPS_34 += src/probes/compiler.me
DEPS_34 += src/probes/doxygen.me
DEPS_34 += src/probes/dsi.me
DEPS_34 += src/probes/dumpbin.me
DEPS_34 += src/probes/ejscmd.me
DEPS_34 += src/probes/est.me
DEPS_34 += src/probes/gzip.me
DEPS_34 += src/probes/htmlmin.me
DEPS_34 += src/probes/httpcmd.me
DEPS_34 += src/probes/lib.me
DEPS_34 += src/probes/link.me
DEPS_34 += src/probes/man.me
DEPS_34 += src/probes/man2html.me
DEPS_34 += src/probes/matrixssl.me
DEPS_34 += src/probes/md5.me
DEPS_34 += src/probes/nanossl.me
DEPS_34 += src/probes/ngmin.me
DEPS_34 += src/probes/openssl.me
DEPS_34 += src/probes/pak.me
DEPS_34 += src/probes/pmaker.me
DEPS_34 += src/probes/ranlib.me
DEPS_34 += src/probes/rc.me
DEPS_34 += src/probes/recess.me
DEPS_34 += src/probes/ssl.me
DEPS_34 += src/probes/strip.me
DEPS_34 += src/probes/tidy.me
DEPS_34 += src/probes/uglifyjs.me
DEPS_34 += src/probes/utest.me
DEPS_34 += src/probes/vxworks.me
DEPS_34 += src/probes/winsdk.me
DEPS_34 += src/probes/zip.me
DEPS_34 += src/sample-main.me
DEPS_34 += src/sample-start.me
DEPS_34 += src/simple.me
DEPS_34 += src/standard.me
DEPS_34 += src/vstudio.es
DEPS_34 += src/xcode.es

$(CONFIG)/bin/makeme: $(DEPS_34)
	@echo '      [Copy] $(CONFIG)/bin/makeme'
	mkdir -p "$(CONFIG)/bin/makeme"
	cp src/me.es $(CONFIG)/bin/makeme/me.es
	cp src/configure.es $(CONFIG)/bin/makeme/configure.es
	cp src/embedthis-manifest.me $(CONFIG)/bin/makeme/embedthis-manifest.me
	cp src/embedthis.me $(CONFIG)/bin/makeme/embedthis.me
	cp src/embedthis.es $(CONFIG)/bin/makeme/embedthis.es
	cp src/gendoc.es $(CONFIG)/bin/makeme/gendoc.es
	cp src/generate.es $(CONFIG)/bin/makeme/generate.es
	mkdir -p "$(CONFIG)/bin/makeme/os"
	cp src/os/freebsd.me $(CONFIG)/bin/makeme/os/freebsd.me
	cp src/os/gcc.me $(CONFIG)/bin/makeme/os/gcc.me
	cp src/os/linux.me $(CONFIG)/bin/makeme/os/linux.me
	cp src/os/macosx.me $(CONFIG)/bin/makeme/os/macosx.me
	cp src/os/solaris.me $(CONFIG)/bin/makeme/os/solaris.me
	cp src/os/unix.me $(CONFIG)/bin/makeme/os/unix.me
	cp src/os/vxworks.me $(CONFIG)/bin/makeme/os/vxworks.me
	cp src/os/windows.me $(CONFIG)/bin/makeme/os/windows.me
	mkdir -p "$(CONFIG)/bin/makeme/os"
	cp src/os/freebsd.me $(CONFIG)/bin/makeme/os/freebsd.me
	cp src/os/gcc.me $(CONFIG)/bin/makeme/os/gcc.me
	cp src/os/linux.me $(CONFIG)/bin/makeme/os/linux.me
	cp src/os/macosx.me $(CONFIG)/bin/makeme/os/macosx.me
	cp src/os/solaris.me $(CONFIG)/bin/makeme/os/solaris.me
	cp src/os/unix.me $(CONFIG)/bin/makeme/os/unix.me
	cp src/os/vxworks.me $(CONFIG)/bin/makeme/os/vxworks.me
	cp src/os/windows.me $(CONFIG)/bin/makeme/os/windows.me
	mkdir -p "$(CONFIG)/bin/makeme/probes"
	cp src/probes/appwebcmd.me $(CONFIG)/bin/makeme/probes/appwebcmd.me
	cp src/probes/compiler.me $(CONFIG)/bin/makeme/probes/compiler.me
	cp src/probes/doxygen.me $(CONFIG)/bin/makeme/probes/doxygen.me
	cp src/probes/dsi.me $(CONFIG)/bin/makeme/probes/dsi.me
	cp src/probes/dumpbin.me $(CONFIG)/bin/makeme/probes/dumpbin.me
	cp src/probes/ejscmd.me $(CONFIG)/bin/makeme/probes/ejscmd.me
	cp src/probes/est.me $(CONFIG)/bin/makeme/probes/est.me
	cp src/probes/gzip.me $(CONFIG)/bin/makeme/probes/gzip.me
	cp src/probes/htmlmin.me $(CONFIG)/bin/makeme/probes/htmlmin.me
	cp src/probes/httpcmd.me $(CONFIG)/bin/makeme/probes/httpcmd.me
	cp src/probes/lib.me $(CONFIG)/bin/makeme/probes/lib.me
	cp src/probes/link.me $(CONFIG)/bin/makeme/probes/link.me
	cp src/probes/man.me $(CONFIG)/bin/makeme/probes/man.me
	cp src/probes/man2html.me $(CONFIG)/bin/makeme/probes/man2html.me
	cp src/probes/matrixssl.me $(CONFIG)/bin/makeme/probes/matrixssl.me
	cp src/probes/md5.me $(CONFIG)/bin/makeme/probes/md5.me
	cp src/probes/nanossl.me $(CONFIG)/bin/makeme/probes/nanossl.me
	cp src/probes/ngmin.me $(CONFIG)/bin/makeme/probes/ngmin.me
	cp src/probes/openssl.me $(CONFIG)/bin/makeme/probes/openssl.me
	cp src/probes/pak.me $(CONFIG)/bin/makeme/probes/pak.me
	cp src/probes/pmaker.me $(CONFIG)/bin/makeme/probes/pmaker.me
	cp src/probes/ranlib.me $(CONFIG)/bin/makeme/probes/ranlib.me
	cp src/probes/rc.me $(CONFIG)/bin/makeme/probes/rc.me
	cp src/probes/recess.me $(CONFIG)/bin/makeme/probes/recess.me
	cp src/probes/ssl.me $(CONFIG)/bin/makeme/probes/ssl.me
	cp src/probes/strip.me $(CONFIG)/bin/makeme/probes/strip.me
	cp src/probes/tidy.me $(CONFIG)/bin/makeme/probes/tidy.me
	cp src/probes/uglifyjs.me $(CONFIG)/bin/makeme/probes/uglifyjs.me
	cp src/probes/utest.me $(CONFIG)/bin/makeme/probes/utest.me
	cp src/probes/vxworks.me $(CONFIG)/bin/makeme/probes/vxworks.me
	cp src/probes/winsdk.me $(CONFIG)/bin/makeme/probes/winsdk.me
	cp src/probes/zip.me $(CONFIG)/bin/makeme/probes/zip.me
	mkdir -p "$(CONFIG)/bin/makeme/probes"
	cp src/probes/appwebcmd.me $(CONFIG)/bin/makeme/probes/appwebcmd.me
	cp src/probes/compiler.me $(CONFIG)/bin/makeme/probes/compiler.me
	cp src/probes/doxygen.me $(CONFIG)/bin/makeme/probes/doxygen.me
	cp src/probes/dsi.me $(CONFIG)/bin/makeme/probes/dsi.me
	cp src/probes/dumpbin.me $(CONFIG)/bin/makeme/probes/dumpbin.me
	cp src/probes/ejscmd.me $(CONFIG)/bin/makeme/probes/ejscmd.me
	cp src/probes/est.me $(CONFIG)/bin/makeme/probes/est.me
	cp src/probes/gzip.me $(CONFIG)/bin/makeme/probes/gzip.me
	cp src/probes/htmlmin.me $(CONFIG)/bin/makeme/probes/htmlmin.me
	cp src/probes/httpcmd.me $(CONFIG)/bin/makeme/probes/httpcmd.me
	cp src/probes/lib.me $(CONFIG)/bin/makeme/probes/lib.me
	cp src/probes/link.me $(CONFIG)/bin/makeme/probes/link.me
	cp src/probes/man.me $(CONFIG)/bin/makeme/probes/man.me
	cp src/probes/man2html.me $(CONFIG)/bin/makeme/probes/man2html.me
	cp src/probes/matrixssl.me $(CONFIG)/bin/makeme/probes/matrixssl.me
	cp src/probes/md5.me $(CONFIG)/bin/makeme/probes/md5.me
	cp src/probes/nanossl.me $(CONFIG)/bin/makeme/probes/nanossl.me
	cp src/probes/ngmin.me $(CONFIG)/bin/makeme/probes/ngmin.me
	cp src/probes/openssl.me $(CONFIG)/bin/makeme/probes/openssl.me
	cp src/probes/pak.me $(CONFIG)/bin/makeme/probes/pak.me
	cp src/probes/pmaker.me $(CONFIG)/bin/makeme/probes/pmaker.me
	cp src/probes/ranlib.me $(CONFIG)/bin/makeme/probes/ranlib.me
	cp src/probes/rc.me $(CONFIG)/bin/makeme/probes/rc.me
	cp src/probes/recess.me $(CONFIG)/bin/makeme/probes/recess.me
	cp src/probes/ssl.me $(CONFIG)/bin/makeme/probes/ssl.me
	cp src/probes/strip.me $(CONFIG)/bin/makeme/probes/strip.me
	cp src/probes/tidy.me $(CONFIG)/bin/makeme/probes/tidy.me
	cp src/probes/uglifyjs.me $(CONFIG)/bin/makeme/probes/uglifyjs.me
	cp src/probes/utest.me $(CONFIG)/bin/makeme/probes/utest.me
	cp src/probes/vxworks.me $(CONFIG)/bin/makeme/probes/vxworks.me
	cp src/probes/winsdk.me $(CONFIG)/bin/makeme/probes/winsdk.me
	cp src/probes/zip.me $(CONFIG)/bin/makeme/probes/zip.me
	cp src/sample-main.me $(CONFIG)/bin/makeme/sample-main.me
	cp src/sample-start.me $(CONFIG)/bin/makeme/sample-start.me
	cp src/simple.me $(CONFIG)/bin/makeme/simple.me
	cp src/standard.me $(CONFIG)/bin/makeme/standard.me
	cp src/vstudio.es $(CONFIG)/bin/makeme/vstudio.es
	cp src/xcode.es $(CONFIG)/bin/makeme/xcode.es

#
#   me.o
#
DEPS_35 += $(CONFIG)/inc/me.h
DEPS_35 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/me.o: \
    src/me.c $(DEPS_35)
	@echo '   [Compile] $(CONFIG)/obj/me.o'
	$(CC) -c -o $(CONFIG)/obj/me.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/me.c

#
#   bit
#
DEPS_36 += $(CONFIG)/inc/mpr.h
DEPS_36 += $(CONFIG)/inc/me.h
DEPS_36 += $(CONFIG)/inc/bitos.h
DEPS_36 += $(CONFIG)/obj/mprLib.o
DEPS_36 += $(CONFIG)/bin/libmpr.dylib
DEPS_36 += $(CONFIG)/inc/pcre.h
DEPS_36 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_36 += $(CONFIG)/bin/libpcre.dylib
endif
DEPS_36 += $(CONFIG)/inc/http.h
DEPS_36 += $(CONFIG)/obj/httpLib.o
DEPS_36 += $(CONFIG)/bin/libhttp.dylib
DEPS_36 += $(CONFIG)/inc/zlib.h
DEPS_36 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_36 += $(CONFIG)/bin/libzlib.dylib
endif
DEPS_36 += $(CONFIG)/inc/ejs.h
DEPS_36 += $(CONFIG)/inc/ejs.slots.h
DEPS_36 += $(CONFIG)/inc/ejsByteGoto.h
DEPS_36 += $(CONFIG)/obj/ejsLib.o
ifeq ($(ME_EXT_EJS),1)
    DEPS_36 += $(CONFIG)/bin/libejs.dylib
endif
DEPS_36 += $(CONFIG)/bin/makeme
DEPS_36 += $(CONFIG)/obj/me.o

LIBS_36 += -lmpr
LIBS_36 += -lhttp
ifeq ($(ME_EXT_PCRE),1)
    LIBS_36 += -lpcre
endif
ifeq ($(ME_EXT_EJS),1)
    LIBS_36 += -lejs
endif
ifeq ($(ME_EXT_ZLIB),1)
    LIBS_36 += -lzlib
endif

$(CONFIG)/bin/me: $(DEPS_36)
	@echo '      [Link] $(CONFIG)/bin/me'
	$(CC) -o $(CONFIG)/bin/me -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/me.o" $(LIBPATHS_36) $(LIBS_36) $(LIBS_36) $(LIBS) 

#
#   bower.json
#
DEPS_37 += package.json

bower.json: $(DEPS_37)
	@echo '      [Copy] bower.json'
	mkdir -p "."
	cp package.json bower.json

#
#   stop
#
stop: $(DEPS_38)

#
#   installBinary
#
installBinary: $(DEPS_39)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.9.4" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(CONFIG)/bin/me $(ME_VAPP_PREFIX)/bin/me ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/me" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/me" "$(ME_BIN_PREFIX)/me" ; \
	cp $(CONFIG)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
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
	cp makeme/me.es $(ME_VAPP_PREFIX)/bin/makeme/me.es ; \
	cp makeme/configure.es $(ME_VAPP_PREFIX)/bin/makeme/configure.es ; \
	cp makeme/embedthis-manifest.me $(ME_VAPP_PREFIX)/bin/makeme/embedthis-manifest.me ; \
	cp makeme/embedthis.me $(ME_VAPP_PREFIX)/bin/makeme/embedthis.me ; \
	cp makeme/embedthis.es $(ME_VAPP_PREFIX)/bin/makeme/embedthis.es ; \
	cp makeme/gendoc.es $(ME_VAPP_PREFIX)/bin/makeme/gendoc.es ; \
	cp makeme/generate.es $(ME_VAPP_PREFIX)/bin/makeme/generate.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/makeme/os" ; \
	cp makeme/os/freebsd.me $(ME_VAPP_PREFIX)/bin/makeme/os/freebsd.me ; \
	cp makeme/os/gcc.me $(ME_VAPP_PREFIX)/bin/makeme/os/gcc.me ; \
	cp makeme/os/linux.me $(ME_VAPP_PREFIX)/bin/makeme/os/linux.me ; \
	cp makeme/os/macosx.me $(ME_VAPP_PREFIX)/bin/makeme/os/macosx.me ; \
	cp makeme/os/solaris.me $(ME_VAPP_PREFIX)/bin/makeme/os/solaris.me ; \
	cp makeme/os/unix.me $(ME_VAPP_PREFIX)/bin/makeme/os/unix.me ; \
	cp makeme/os/vxworks.me $(ME_VAPP_PREFIX)/bin/makeme/os/vxworks.me ; \
	cp makeme/os/windows.me $(ME_VAPP_PREFIX)/bin/makeme/os/windows.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/makeme/probes" ; \
	cp makeme/probes/appwebcmd.me $(ME_VAPP_PREFIX)/bin/makeme/probes/appwebcmd.me ; \
	cp makeme/probes/compiler.me $(ME_VAPP_PREFIX)/bin/makeme/probes/compiler.me ; \
	cp makeme/probes/doxygen.me $(ME_VAPP_PREFIX)/bin/makeme/probes/doxygen.me ; \
	cp makeme/probes/dsi.me $(ME_VAPP_PREFIX)/bin/makeme/probes/dsi.me ; \
	cp makeme/probes/dumpbin.me $(ME_VAPP_PREFIX)/bin/makeme/probes/dumpbin.me ; \
	cp makeme/probes/ejscmd.me $(ME_VAPP_PREFIX)/bin/makeme/probes/ejscmd.me ; \
	cp makeme/probes/est.me $(ME_VAPP_PREFIX)/bin/makeme/probes/est.me ; \
	cp makeme/probes/gzip.me $(ME_VAPP_PREFIX)/bin/makeme/probes/gzip.me ; \
	cp makeme/probes/htmlmin.me $(ME_VAPP_PREFIX)/bin/makeme/probes/htmlmin.me ; \
	cp makeme/probes/httpcmd.me $(ME_VAPP_PREFIX)/bin/makeme/probes/httpcmd.me ; \
	cp makeme/probes/lib.me $(ME_VAPP_PREFIX)/bin/makeme/probes/lib.me ; \
	cp makeme/probes/link.me $(ME_VAPP_PREFIX)/bin/makeme/probes/link.me ; \
	cp makeme/probes/man.me $(ME_VAPP_PREFIX)/bin/makeme/probes/man.me ; \
	cp makeme/probes/man2html.me $(ME_VAPP_PREFIX)/bin/makeme/probes/man2html.me ; \
	cp makeme/probes/matrixssl.me $(ME_VAPP_PREFIX)/bin/makeme/probes/matrixssl.me ; \
	cp makeme/probes/md5.me $(ME_VAPP_PREFIX)/bin/makeme/probes/md5.me ; \
	cp makeme/probes/nanossl.me $(ME_VAPP_PREFIX)/bin/makeme/probes/nanossl.me ; \
	cp makeme/probes/ngmin.me $(ME_VAPP_PREFIX)/bin/makeme/probes/ngmin.me ; \
	cp makeme/probes/openssl.me $(ME_VAPP_PREFIX)/bin/makeme/probes/openssl.me ; \
	cp makeme/probes/pak.me $(ME_VAPP_PREFIX)/bin/makeme/probes/pak.me ; \
	cp makeme/probes/pmaker.me $(ME_VAPP_PREFIX)/bin/makeme/probes/pmaker.me ; \
	cp makeme/probes/ranlib.me $(ME_VAPP_PREFIX)/bin/makeme/probes/ranlib.me ; \
	cp makeme/probes/rc.me $(ME_VAPP_PREFIX)/bin/makeme/probes/rc.me ; \
	cp makeme/probes/recess.me $(ME_VAPP_PREFIX)/bin/makeme/probes/recess.me ; \
	cp makeme/probes/ssl.me $(ME_VAPP_PREFIX)/bin/makeme/probes/ssl.me ; \
	cp makeme/probes/strip.me $(ME_VAPP_PREFIX)/bin/makeme/probes/strip.me ; \
	cp makeme/probes/tidy.me $(ME_VAPP_PREFIX)/bin/makeme/probes/tidy.me ; \
	cp makeme/probes/uglifyjs.me $(ME_VAPP_PREFIX)/bin/makeme/probes/uglifyjs.me ; \
	cp makeme/probes/utest.me $(ME_VAPP_PREFIX)/bin/makeme/probes/utest.me ; \
	cp makeme/probes/vxworks.me $(ME_VAPP_PREFIX)/bin/makeme/probes/vxworks.me ; \
	cp makeme/probes/winsdk.me $(ME_VAPP_PREFIX)/bin/makeme/probes/winsdk.me ; \
	cp makeme/probes/zip.me $(ME_VAPP_PREFIX)/bin/makeme/probes/zip.me ; \
	cp makeme/sample-main.me $(ME_VAPP_PREFIX)/bin/makeme/sample-main.me ; \
	cp makeme/sample-start.me $(ME_VAPP_PREFIX)/bin/makeme/sample-start.me ; \
	cp makeme/simple.me $(ME_VAPP_PREFIX)/bin/makeme/simple.me ; \
	cp makeme/standard.me $(ME_VAPP_PREFIX)/bin/makeme/standard.me ; \
	cp makeme/vstudio.es $(ME_VAPP_PREFIX)/bin/makeme/vstudio.es ; \
	cp makeme/xcode.es $(ME_VAPP_PREFIX)/bin/makeme/xcode.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	)

#
#   start
#
start: $(DEPS_40)

#
#   install
#
DEPS_41 += stop
DEPS_41 += installBinary
DEPS_41 += start

install: $(DEPS_41)

#
#   uninstall
#
DEPS_42 += stop

uninstall: $(DEPS_42)
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

