#
#   me-macosx-default.mk -- Makefile to build Embedthis MakeMe for macosx
#

NAME                  := me
VERSION               := 0.8.5
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= macosx
CC                    ?= clang
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_EJS            ?= 1
ME_COM_EST            ?= 1
ME_COM_HTTP           ?= 1
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_SQLITE         ?= 0
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_WINSDK         ?= 1
ME_COM_ZLIB           ?= 1

ifeq ($(ME_COM_EST),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_EJS),1)
    ME_COM_ZLIB := 1
endif

CFLAGS                += -g -w
DFLAGS                +=  $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EJS=$(ME_COM_EJS) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS              += -L$(BUILD)/bin
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
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


ifeq ($(ME_COM_EJS),1)
    TARGETS           += $(BUILD)/bin/ejs.mod
endif
TARGETS               += $(BUILD)/bin/ejs.testme.es
TARGETS               += $(BUILD)/bin/ejs.testme.mod
ifeq ($(ME_COM_EJS),1)
    TARGETS           += $(BUILD)/bin/ejs
endif
TARGETS               += $(BUILD)/bin/ca.crt
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(BUILD)/bin/http
endif
ifeq ($(ME_COM_EST),1)
    TARGETS           += $(BUILD)/bin/libest.dylib
endif
TARGETS               += $(BUILD)/bin/libmprssl.dylib
TARGETS               += $(BUILD)/bin/libtestme.dylib
TARGETS               += $(BUILD)/bin/me
TARGETS               += $(BUILD)/bin/testme
TARGETS               += $(BUILD)/bin/testme.es

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
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/me-macosx-default-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/me-macosx-default-me.h >/dev/null ; then\
		cp projects/me-macosx-default-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo "$(MAKEFLAGS)" >$(BUILD)/.makeflags

clean:
	rm -f "$(BUILD)/obj/ejs.o"
	rm -f "$(BUILD)/obj/ejsLib.o"
	rm -f "$(BUILD)/obj/ejsc.o"
	rm -f "$(BUILD)/obj/estLib.o"
	rm -f "$(BUILD)/obj/http.o"
	rm -f "$(BUILD)/obj/httpLib.o"
	rm -f "$(BUILD)/obj/libtestme.o"
	rm -f "$(BUILD)/obj/me.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/mprSsl.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/testme.o"
	rm -f "$(BUILD)/obj/zlib.o"
	rm -f "$(BUILD)/bin/ejs.testme.es"
	rm -f "$(BUILD)/bin/ejsc"
	rm -f "$(BUILD)/bin/ejs"
	rm -f "$(BUILD)/bin/ca.crt"
	rm -f "$(BUILD)/bin/http"
	rm -f "$(BUILD)/bin/libejs.dylib"
	rm -f "$(BUILD)/bin/libest.dylib"
	rm -f "$(BUILD)/bin/libhttp.dylib"
	rm -f "$(BUILD)/bin/libmpr.dylib"
	rm -f "$(BUILD)/bin/libmprssl.dylib"
	rm -f "$(BUILD)/bin/libpcre.dylib"
	rm -f "$(BUILD)/bin/libtestme.dylib"
	rm -f "$(BUILD)/bin/libzlib.dylib"
	rm -f "$(BUILD)/bin/testme"
	rm -f "$(BUILD)/bin/testme.es"

clobber: clean
	rm -fr ./$(BUILD)

#
#   me.h
#

$(BUILD)/inc/me.h: $(DEPS_1)

#
#   osdep.h
#
DEPS_2 += src/paks/osdep/osdep.h
DEPS_2 += $(BUILD)/inc/me.h

$(BUILD)/inc/osdep.h: $(DEPS_2)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_3 += src/paks/mpr/mpr.h
DEPS_3 += $(BUILD)/inc/me.h
DEPS_3 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_3)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_4 += src/paks/http/http.h
DEPS_4 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_4)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/http/http.h $(BUILD)/inc/http.h

#
#   ejs.slots.h
#

src/paks/ejs/ejs.slots.h: $(DEPS_5)

#
#   pcre.h
#
DEPS_6 += src/paks/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_6)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   zlib.h
#
DEPS_7 += src/paks/zlib/zlib.h
DEPS_7 += $(BUILD)/inc/me.h

$(BUILD)/inc/zlib.h: $(DEPS_7)
	@echo '      [Copy] $(BUILD)/inc/zlib.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/zlib/zlib.h $(BUILD)/inc/zlib.h

#
#   ejs.h
#
DEPS_8 += src/paks/ejs/ejs.h
DEPS_8 += $(BUILD)/inc/me.h
DEPS_8 += $(BUILD)/inc/osdep.h
DEPS_8 += $(BUILD)/inc/mpr.h
DEPS_8 += $(BUILD)/inc/http.h
DEPS_8 += src/paks/ejs/ejs.slots.h
DEPS_8 += $(BUILD)/inc/pcre.h
DEPS_8 += $(BUILD)/inc/zlib.h

$(BUILD)/inc/ejs.h: $(DEPS_8)
	@echo '      [Copy] $(BUILD)/inc/ejs.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/ejs/ejs.h $(BUILD)/inc/ejs.h

#
#   ejs.slots.h
#
DEPS_9 += src/paks/ejs/ejs.slots.h

$(BUILD)/inc/ejs.slots.h: $(DEPS_9)
	@echo '      [Copy] $(BUILD)/inc/ejs.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/ejs/ejs.slots.h $(BUILD)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
DEPS_10 += src/paks/ejs/ejsByteGoto.h

$(BUILD)/inc/ejsByteGoto.h: $(DEPS_10)
	@echo '      [Copy] $(BUILD)/inc/ejsByteGoto.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/ejs/ejsByteGoto.h $(BUILD)/inc/ejsByteGoto.h

#
#   est.h
#
DEPS_11 += src/paks/est/est.h

$(BUILD)/inc/est.h: $(DEPS_11)
	@echo '      [Copy] $(BUILD)/inc/est.h'
	mkdir -p "$(BUILD)/inc"
	cp src/paks/est/est.h $(BUILD)/inc/est.h

#
#   testme.h
#
DEPS_12 += src/tm/testme.h

$(BUILD)/inc/testme.h: $(DEPS_12)
	@echo '      [Copy] $(BUILD)/inc/testme.h'
	mkdir -p "$(BUILD)/inc"
	cp src/tm/testme.h $(BUILD)/inc/testme.h

#
#   ejs.h
#

src/paks/ejs/ejs.h: $(DEPS_13)

#
#   ejs.o
#
DEPS_14 += src/paks/ejs/ejs.h

$(BUILD)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_14)
	@echo '   [Compile] $(BUILD)/obj/ejs.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/ejs.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

#
#   ejsLib.o
#
DEPS_15 += src/paks/ejs/ejs.h
DEPS_15 += $(BUILD)/inc/mpr.h
DEPS_15 += $(BUILD)/inc/pcre.h
DEPS_15 += $(BUILD)/inc/me.h

$(BUILD)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_15)
	@echo '   [Compile] $(BUILD)/obj/ejsLib.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/ejsLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

#
#   ejsc.o
#
DEPS_16 += src/paks/ejs/ejs.h

$(BUILD)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_16)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/ejsc.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

#
#   est.h
#

src/paks/est/est.h: $(DEPS_17)

#
#   estLib.o
#
DEPS_18 += src/paks/est/est.h

$(BUILD)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_18)
	@echo '   [Compile] $(BUILD)/obj/estLib.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/estLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/est/estLib.c

#
#   http.h
#

src/paks/http/http.h: $(DEPS_19)

#
#   http.o
#
DEPS_20 += src/paks/http/http.h

$(BUILD)/obj/http.o: \
    src/paks/http/http.c $(DEPS_20)
	@echo '   [Compile] $(BUILD)/obj/http.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/http.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   httpLib.o
#
DEPS_21 += src/paks/http/http.h

$(BUILD)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_21)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/httpLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/http/httpLib.c

#
#   testme.h
#

src/tm/testme.h: $(DEPS_22)

#
#   libtestme.o
#
DEPS_23 += src/tm/testme.h

$(BUILD)/obj/libtestme.o: \
    src/tm/libtestme.c $(DEPS_23)
	@echo '   [Compile] $(BUILD)/obj/libtestme.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/libtestme.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/tm/libtestme.c

#
#   me.o
#
DEPS_24 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/me.o: \
    src/me.c $(DEPS_24)
	@echo '   [Compile] $(BUILD)/obj/me.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/me.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/me.c

#
#   mpr.h
#

src/paks/mpr/mpr.h: $(DEPS_25)

#
#   mprLib.o
#
DEPS_26 += src/paks/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_26)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/mprLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   mprSsl.o
#
DEPS_27 += src/paks/mpr/mpr.h

$(BUILD)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_27)
	@echo '   [Compile] $(BUILD)/obj/mprSsl.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/mprSsl.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/paks/mpr/mprSsl.c

#
#   pcre.h
#

src/paks/pcre/pcre.h: $(DEPS_28)

#
#   pcre.o
#
DEPS_29 += $(BUILD)/inc/me.h
DEPS_29 += src/paks/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_29)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/pcre.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

#
#   testme.o
#
DEPS_30 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/testme.o: \
    src/tm/testme.c $(DEPS_30)
	@echo '   [Compile] $(BUILD)/obj/testme.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/testme.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/tm/testme.c

#
#   zlib.h
#

src/paks/zlib/zlib.h: $(DEPS_31)

#
#   zlib.o
#
DEPS_32 += $(BUILD)/inc/me.h
DEPS_32 += src/paks/zlib/zlib.h

$(BUILD)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_32)
	@echo '   [Compile] $(BUILD)/obj/zlib.o'
	$(CC) -c $(DFLAGS) -o $(BUILD)/obj/zlib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

#
#   libmpr
#
DEPS_33 += $(BUILD)/inc/osdep.h
DEPS_33 += $(BUILD)/inc/mpr.h
DEPS_33 += $(BUILD)/obj/mprLib.o

$(BUILD)/bin/libmpr.dylib: $(DEPS_33)
	@echo '      [Link] $(BUILD)/bin/libmpr.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libmpr.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/mprLib.o" $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_34 += $(BUILD)/inc/pcre.h
DEPS_34 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.dylib: $(DEPS_34)
	@echo '      [Link] $(BUILD)/bin/libpcre.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libpcre.dylib -arch $(CC_ARCH) $(LDFLAGS) -compatibility_version 0.8 -current_version 0.8 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_35 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_PCRE),1)
    DEPS_35 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_35 += $(BUILD)/inc/http.h
DEPS_35 += $(BUILD)/obj/httpLib.o

LIBS_35 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_35 += -lpcre
endif

$(BUILD)/bin/libhttp.dylib: $(DEPS_35)
	@echo '      [Link] $(BUILD)/bin/libhttp.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libhttp.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/httpLib.o" $(LIBPATHS_35) $(LIBS_35) $(LIBS_35) $(LIBS) 
endif

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_36 += $(BUILD)/inc/zlib.h
DEPS_36 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.dylib: $(DEPS_36)
	@echo '      [Link] $(BUILD)/bin/libzlib.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libzlib.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libzlib.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   libejs
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_37 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_PCRE),1)
    DEPS_37 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_37 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_ZLIB),1)
    DEPS_37 += $(BUILD)/bin/libzlib.dylib
endif
DEPS_37 += $(BUILD)/inc/ejs.h
DEPS_37 += $(BUILD)/inc/ejs.slots.h
DEPS_37 += $(BUILD)/inc/ejsByteGoto.h
DEPS_37 += $(BUILD)/obj/ejsLib.o

ifeq ($(ME_COM_HTTP),1)
    LIBS_37 += -lhttp
endif
LIBS_37 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_37 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_37 += -lzlib
endif

$(BUILD)/bin/libejs.dylib: $(DEPS_37)
	@echo '      [Link] $(BUILD)/bin/libejs.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libejs.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libejs.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/ejsLib.o" $(LIBPATHS_37) $(LIBS_37) $(LIBS_37) $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   ejsc
#
DEPS_38 += $(BUILD)/bin/libejs.dylib
DEPS_38 += $(BUILD)/obj/ejsc.o

LIBS_38 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_38 += -lhttp
endif
LIBS_38 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_38 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_38 += -lzlib
endif

$(BUILD)/bin/ejsc: $(DEPS_38)
	@echo '      [Link] $(BUILD)/bin/ejsc'
	$(CC) -o $(BUILD)/bin/ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsc.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   ejs.mod
#
DEPS_39 += src/paks/ejs/ejs.es
DEPS_39 += $(BUILD)/bin/ejsc

$(BUILD)/bin/ejs.mod: $(DEPS_39)
	( \
	cd src/paks/ejs; \
	echo '   [Compile] ejs.mod' ; \
	../../../$(BUILD)/bin/ejsc --out ../../../$(BUILD)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.es
#
DEPS_40 += src/tm/ejs.testme.es

$(BUILD)/bin/ejs.testme.es: $(DEPS_40)
	@echo '      [Copy] $(BUILD)/bin/ejs.testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/ejs.testme.es $(BUILD)/bin/ejs.testme.es

#
#   ejs.testme.mod
#
DEPS_41 += src/tm/ejs.testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_41 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/ejs.testme.mod: $(DEPS_41)
	( \
	cd src/tm; \
	echo '   [Compile] ejs.testme.mod' ; \
	../../$(BUILD)/bin/ejsc --debug --out ../../$(BUILD)/bin/ejs.testme.mod --optimize 9 ejs.testme.es ; \
	)

ifeq ($(ME_COM_EJS),1)
#
#   ejscmd
#
DEPS_42 += $(BUILD)/bin/libejs.dylib
DEPS_42 += $(BUILD)/obj/ejs.o

LIBS_42 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_42 += -lhttp
endif
LIBS_42 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_42 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_42 += -lzlib
endif

$(BUILD)/bin/ejs: $(DEPS_42)
	@echo '      [Link] $(BUILD)/bin/ejs'
	$(CC) -o $(BUILD)/bin/ejs -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejs.o" $(LIBPATHS_42) $(LIBS_42) $(LIBS_42) $(LIBS) -ledit 
endif

#
#   http-ca-crt
#
DEPS_43 += src/paks/http/ca.crt

$(BUILD)/bin/ca.crt: $(DEPS_43)
	@echo '      [Copy] $(BUILD)/bin/ca.crt'
	mkdir -p "$(BUILD)/bin"
	cp src/paks/http/ca.crt $(BUILD)/bin/ca.crt

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_44 += $(BUILD)/bin/libhttp.dylib
DEPS_44 += $(BUILD)/obj/http.o

LIBS_44 += -lhttp
LIBS_44 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_44 += -lpcre
endif

$(BUILD)/bin/http: $(DEPS_44)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/http.o" $(LIBPATHS_44) $(LIBS_44) $(LIBS_44) $(LIBS) 
endif

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_45 += $(BUILD)/inc/osdep.h
DEPS_45 += $(BUILD)/inc/est.h
DEPS_45 += $(BUILD)/obj/estLib.o

$(BUILD)/bin/libest.dylib: $(DEPS_45)
	@echo '      [Link] $(BUILD)/bin/libest.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libest.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libest.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/estLib.o" $(LIBS) 
endif

#
#   libmprssl
#
DEPS_46 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_EST),1)
    DEPS_46 += $(BUILD)/bin/libest.dylib
endif
DEPS_46 += $(BUILD)/obj/mprSsl.o

LIBS_46 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_46 += -lssl
    LIBPATHS_46 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_46 += -lcrypto
    LIBPATHS_46 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_EST),1)
    LIBS_46 += -lest
endif

$(BUILD)/bin/libmprssl.dylib: $(DEPS_46)
	@echo '      [Link] $(BUILD)/bin/libmprssl.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libmprssl.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  -install_name @rpath/libmprssl.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/mprSsl.o" $(LIBPATHS_46) $(LIBS_46) $(LIBS_46) $(LIBS) 

#
#   libtestme
#
DEPS_47 += $(BUILD)/inc/testme.h
DEPS_47 += $(BUILD)/obj/libtestme.o

$(BUILD)/bin/libtestme.dylib: $(DEPS_47)
	@echo '      [Link] $(BUILD)/bin/libtestme.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libtestme.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libtestme.dylib -compatibility_version 0.8 -current_version 0.8 "$(BUILD)/obj/libtestme.o" $(LIBS) 

#
#   me.mod
#
DEPS_48 += src/Builder.es
DEPS_48 += src/Loader.es
DEPS_48 += src/MakeMe.es
DEPS_48 += src/Me.es
DEPS_48 += src/Script.es
DEPS_48 += src/Target.es
DEPS_48 += src/paks/ejs-version/Version.es
ifeq ($(ME_COM_EJS),1)
    DEPS_48 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/me.mod: $(DEPS_48)
	echo '   [Compile] me.mod' ; \
	./$(BUILD)/bin/ejsc --debug --out ./$(BUILD)/bin/me.mod --optimize 9 src/Builder.es src/Loader.es src/MakeMe.es src/Me.es src/Script.es src/Target.es src/paks/ejs-version/Version.es

#
#   runtime
#
DEPS_49 += src/master-main.me
DEPS_49 += src/master-start.me
DEPS_49 += src/simple.me
DEPS_49 += src/standard.me
DEPS_49 += src/paks/me-configuration/Configuration.es
DEPS_49 += src/paks/me-configuration/configuration.me
DEPS_49 += src/paks/me-configuration/LICENSE.md
DEPS_49 += src/paks/me-configuration/package.json
DEPS_49 += src/paks/me-configuration/README.md
DEPS_49 += src/paks/me-components/appweb.me
DEPS_49 += src/paks/me-components/compiler.me
DEPS_49 += src/paks/me-components/components.me
DEPS_49 += src/paks/me-components/lib.me
DEPS_49 += src/paks/me-components/LICENSE.md
DEPS_49 += src/paks/me-components/link.me
DEPS_49 += src/paks/me-components/package.json
DEPS_49 += src/paks/me-components/rc.me
DEPS_49 += src/paks/me-components/README.md
DEPS_49 += src/paks/me-components/testme.me
DEPS_49 += src/paks/me-components/vxworks.me
DEPS_49 += src/paks/me-components/winsdk.me
DEPS_49 += src/paks/me-package/LICENSE.md
DEPS_49 += src/paks/me-package/manifest.me
DEPS_49 += src/paks/me-package/Package.es
DEPS_49 += src/paks/me-package/package.json
DEPS_49 += src/paks/me-package/package.me
DEPS_49 += src/paks/me-package/README.md
DEPS_49 += src/paks/me-project/LICENSE.md
DEPS_49 += src/paks/me-project/package.json
DEPS_49 += src/paks/me-project/Project.es
DEPS_49 += src/paks/me-project/project.me
DEPS_49 += src/paks/me-project/README.md
DEPS_49 += src/paks/me-os/freebsd.me
DEPS_49 += src/paks/me-os/gcc.me
DEPS_49 += src/paks/me-os/LICENSE.md
DEPS_49 += src/paks/me-os/linux.me
DEPS_49 += src/paks/me-os/macosx.me
DEPS_49 += src/paks/me-os/os.me
DEPS_49 += src/paks/me-os/package.json
DEPS_49 += src/paks/me-os/README.md
DEPS_49 += src/paks/me-os/solaris.me
DEPS_49 += src/paks/me-os/unix.me
DEPS_49 += src/paks/me-os/vxworks.me
DEPS_49 += src/paks/me-os/windows.me
DEPS_49 += src/paks/me-vstudio/LICENSE.md
DEPS_49 += src/paks/me-vstudio/package.json
DEPS_49 += src/paks/me-vstudio/README.md
DEPS_49 += src/paks/me-vstudio/Vstudio.es
DEPS_49 += src/paks/me-vstudio/vstudio.me
DEPS_49 += src/paks/me-xcode/LICENSE.md
DEPS_49 += src/paks/me-xcode/package.json
DEPS_49 += src/paks/me-xcode/README.md
DEPS_49 += src/paks/me-xcode/Xcode.es
DEPS_49 += src/paks/me-xcode/xcode.me

$(BUILD)/.runtime-modified: $(DEPS_49)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/master-main.me $(BUILD)/bin/master-main.me
	cp src/master-start.me $(BUILD)/bin/master-start.me
	cp src/simple.me $(BUILD)/bin/simple.me
	cp src/standard.me $(BUILD)/bin/standard.me
	mkdir -p "$(BUILD)/bin/paks/me-configuration"
	cp src/paks/me-configuration/Configuration.es $(BUILD)/bin/paks/me-configuration/Configuration.es
	cp src/paks/me-configuration/configuration.me $(BUILD)/bin/paks/me-configuration/configuration.me
	cp src/paks/me-configuration/LICENSE.md $(BUILD)/bin/paks/me-configuration/LICENSE.md
	cp src/paks/me-configuration/package.json $(BUILD)/bin/paks/me-configuration/package.json
	cp src/paks/me-configuration/README.md $(BUILD)/bin/paks/me-configuration/README.md
	mkdir -p "$(BUILD)/bin/paks/me-components"
	cp src/paks/me-components/appweb.me $(BUILD)/bin/paks/me-components/appweb.me
	cp src/paks/me-components/compiler.me $(BUILD)/bin/paks/me-components/compiler.me
	cp src/paks/me-components/components.me $(BUILD)/bin/paks/me-components/components.me
	cp src/paks/me-components/lib.me $(BUILD)/bin/paks/me-components/lib.me
	cp src/paks/me-components/LICENSE.md $(BUILD)/bin/paks/me-components/LICENSE.md
	cp src/paks/me-components/link.me $(BUILD)/bin/paks/me-components/link.me
	cp src/paks/me-components/package.json $(BUILD)/bin/paks/me-components/package.json
	cp src/paks/me-components/rc.me $(BUILD)/bin/paks/me-components/rc.me
	cp src/paks/me-components/README.md $(BUILD)/bin/paks/me-components/README.md
	cp src/paks/me-components/testme.me $(BUILD)/bin/paks/me-components/testme.me
	cp src/paks/me-components/vxworks.me $(BUILD)/bin/paks/me-components/vxworks.me
	cp src/paks/me-components/winsdk.me $(BUILD)/bin/paks/me-components/winsdk.me
	mkdir -p "$(BUILD)/bin/paks/me-package"
	cp src/paks/me-package/LICENSE.md $(BUILD)/bin/paks/me-package/LICENSE.md
	cp src/paks/me-package/manifest.me $(BUILD)/bin/paks/me-package/manifest.me
	cp src/paks/me-package/Package.es $(BUILD)/bin/paks/me-package/Package.es
	cp src/paks/me-package/package.json $(BUILD)/bin/paks/me-package/package.json
	cp src/paks/me-package/package.me $(BUILD)/bin/paks/me-package/package.me
	cp src/paks/me-package/README.md $(BUILD)/bin/paks/me-package/README.md
	mkdir -p "$(BUILD)/bin/paks/me-project"
	cp src/paks/me-project/LICENSE.md $(BUILD)/bin/paks/me-project/LICENSE.md
	cp src/paks/me-project/package.json $(BUILD)/bin/paks/me-project/package.json
	cp src/paks/me-project/Project.es $(BUILD)/bin/paks/me-project/Project.es
	cp src/paks/me-project/project.me $(BUILD)/bin/paks/me-project/project.me
	cp src/paks/me-project/README.md $(BUILD)/bin/paks/me-project/README.md
	mkdir -p "$(BUILD)/bin/paks/me-os"
	cp src/paks/me-os/freebsd.me $(BUILD)/bin/paks/me-os/freebsd.me
	cp src/paks/me-os/gcc.me $(BUILD)/bin/paks/me-os/gcc.me
	cp src/paks/me-os/LICENSE.md $(BUILD)/bin/paks/me-os/LICENSE.md
	cp src/paks/me-os/linux.me $(BUILD)/bin/paks/me-os/linux.me
	cp src/paks/me-os/macosx.me $(BUILD)/bin/paks/me-os/macosx.me
	cp src/paks/me-os/os.me $(BUILD)/bin/paks/me-os/os.me
	cp src/paks/me-os/package.json $(BUILD)/bin/paks/me-os/package.json
	cp src/paks/me-os/README.md $(BUILD)/bin/paks/me-os/README.md
	cp src/paks/me-os/solaris.me $(BUILD)/bin/paks/me-os/solaris.me
	cp src/paks/me-os/unix.me $(BUILD)/bin/paks/me-os/unix.me
	cp src/paks/me-os/vxworks.me $(BUILD)/bin/paks/me-os/vxworks.me
	cp src/paks/me-os/windows.me $(BUILD)/bin/paks/me-os/windows.me
	mkdir -p "$(BUILD)/bin/paks/me-vstudio"
	cp src/paks/me-vstudio/LICENSE.md $(BUILD)/bin/paks/me-vstudio/LICENSE.md
	cp src/paks/me-vstudio/package.json $(BUILD)/bin/paks/me-vstudio/package.json
	cp src/paks/me-vstudio/README.md $(BUILD)/bin/paks/me-vstudio/README.md
	cp src/paks/me-vstudio/Vstudio.es $(BUILD)/bin/paks/me-vstudio/Vstudio.es
	cp src/paks/me-vstudio/vstudio.me $(BUILD)/bin/paks/me-vstudio/vstudio.me
	mkdir -p "$(BUILD)/bin/paks/me-xcode"
	cp src/paks/me-xcode/LICENSE.md $(BUILD)/bin/paks/me-xcode/LICENSE.md
	cp src/paks/me-xcode/package.json $(BUILD)/bin/paks/me-xcode/package.json
	cp src/paks/me-xcode/README.md $(BUILD)/bin/paks/me-xcode/README.md
	cp src/paks/me-xcode/Xcode.es $(BUILD)/bin/paks/me-xcode/Xcode.es
	cp src/paks/me-xcode/xcode.me $(BUILD)/bin/paks/me-xcode/xcode.me
	touch "$(BUILD)/.runtime-modified"

#
#   me
#
DEPS_50 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_HTTP),1)
    DEPS_50 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_EJS),1)
    DEPS_50 += $(BUILD)/bin/libejs.dylib
endif
DEPS_50 += $(BUILD)/bin/me.mod
DEPS_50 += $(BUILD)/.runtime-modified
DEPS_50 += $(BUILD)/obj/me.o

LIBS_50 += -lmpr
ifeq ($(ME_COM_HTTP),1)
    LIBS_50 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_50 += -lpcre
endif
ifeq ($(ME_COM_EJS),1)
    LIBS_50 += -lejs
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_50 += -lzlib
endif

$(BUILD)/bin/me: $(DEPS_50)
	@echo '      [Link] $(BUILD)/bin/me'
	$(CC) -o $(BUILD)/bin/me -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/me.o" $(LIBPATHS_50) $(LIBS_50) $(LIBS_50) $(LIBS) 

#
#   testme.mod
#
DEPS_51 += src/tm/testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_51 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/testme.mod: $(DEPS_51)
	( \
	cd src/tm; \
	echo '   [Compile] testme.mod' ; \
	../../$(BUILD)/bin/ejsc --debug --out ../../$(BUILD)/bin/testme.mod --optimize 9 testme.es ; \
	)

#
#   testme
#
ifeq ($(ME_COM_EJS),1)
    DEPS_52 += $(BUILD)/bin/libejs.dylib
endif
DEPS_52 += $(BUILD)/bin/testme.mod
DEPS_52 += $(BUILD)/bin/ejs.testme.mod
DEPS_52 += $(BUILD)/obj/testme.o

ifeq ($(ME_COM_EJS),1)
    LIBS_52 += -lejs
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_52 += -lhttp
endif
LIBS_52 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_52 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_52 += -lzlib
endif

$(BUILD)/bin/testme: $(DEPS_52)
	@echo '      [Link] $(BUILD)/bin/testme'
	$(CC) -o $(BUILD)/bin/testme -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/testme.o" $(LIBPATHS_52) $(LIBS_52) $(LIBS_52) $(LIBS) 

#
#   testme.es
#
DEPS_53 += src/tm/testme.es

$(BUILD)/bin/testme.es: $(DEPS_53)
	@echo '      [Copy] $(BUILD)/bin/testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/testme.es $(BUILD)/bin/testme.es

#
#   stop
#

stop: $(DEPS_54)

#
#   installBinary
#

installBinary: $(DEPS_55)
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.8.5" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/me $(ME_VAPP_PREFIX)/bin/me ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/me" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/me" "$(ME_BIN_PREFIX)/me" ; \
	cp $(BUILD)/bin/testme $(ME_VAPP_PREFIX)/bin/testme ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/testme" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/testme" "$(ME_BIN_PREFIX)/testme" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejs" "$(ME_BIN_PREFIX)/ejs" ; \
	cp $(BUILD)/bin/ejsc $(ME_VAPP_PREFIX)/bin/ejsc ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejsc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsc" "$(ME_BIN_PREFIX)/ejsc" ; \
	cp $(BUILD)/bin/http $(ME_VAPP_PREFIX)/bin/http ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/http" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/http" "$(ME_BIN_PREFIX)/http" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libejs.dylib $(ME_VAPP_PREFIX)/bin/libejs.dylib ; \
	cp $(BUILD)/bin/libhttp.dylib $(ME_VAPP_PREFIX)/bin/libhttp.dylib ; \
	cp $(BUILD)/bin/libmpr.dylib $(ME_VAPP_PREFIX)/bin/libmpr.dylib ; \
	cp $(BUILD)/bin/libmprssl.dylib $(ME_VAPP_PREFIX)/bin/libmprssl.dylib ; \
	cp $(BUILD)/bin/libpcre.dylib $(ME_VAPP_PREFIX)/bin/libpcre.dylib ; \
	cp $(BUILD)/bin/libzlib.dylib $(ME_VAPP_PREFIX)/bin/libzlib.dylib ; \
	cp $(BUILD)/bin/libtestme.dylib $(ME_VAPP_PREFIX)/bin/libtestme.dylib ; \
	if [ "$(ME_COM_EST)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libest.dylib $(ME_VAPP_PREFIX)/bin/libest.dylib ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libssl*.dylib* $(ME_VAPP_PREFIX)/bin/libssl*.dylib* ; \
	cp $(BUILD)/bin/libcrypto*.dylib* $(ME_VAPP_PREFIX)/bin/libcrypto*.dylib* ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	cp $(BUILD)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp $(BUILD)/bin/me.mod $(ME_VAPP_PREFIX)/bin/me.mod ; \
	cp $(BUILD)/bin/testme.mod $(ME_VAPP_PREFIX)/bin/testme.mod ; \
	cp $(BUILD)/bin/ejs.testme.mod $(ME_VAPP_PREFIX)/bin/ejs.testme.mod ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp src/tm/testme.h $(ME_VAPP_PREFIX)/inc/testme.h ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp src/master-main.me $(ME_VAPP_PREFIX)/bin/master-main.me ; \
	cp src/master-start.me $(ME_VAPP_PREFIX)/bin/master-start.me ; \
	cp src/simple.me $(ME_VAPP_PREFIX)/bin/simple.me ; \
	cp src/standard.me $(ME_VAPP_PREFIX)/bin/standard.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-components" ; \
	cp src/paks/me-components/appweb.me $(ME_VAPP_PREFIX)/bin/paks/me-components/appweb.me ; \
	cp src/paks/me-components/compiler.me $(ME_VAPP_PREFIX)/bin/paks/me-components/compiler.me ; \
	cp src/paks/me-components/components.me $(ME_VAPP_PREFIX)/bin/paks/me-components/components.me ; \
	cp src/paks/me-components/lib.me $(ME_VAPP_PREFIX)/bin/paks/me-components/lib.me ; \
	cp src/paks/me-components/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-components/LICENSE.md ; \
	cp src/paks/me-components/link.me $(ME_VAPP_PREFIX)/bin/paks/me-components/link.me ; \
	cp src/paks/me-components/package.json $(ME_VAPP_PREFIX)/bin/paks/me-components/package.json ; \
	cp src/paks/me-components/rc.me $(ME_VAPP_PREFIX)/bin/paks/me-components/rc.me ; \
	cp src/paks/me-components/README.md $(ME_VAPP_PREFIX)/bin/paks/me-components/README.md ; \
	cp src/paks/me-components/testme.me $(ME_VAPP_PREFIX)/bin/paks/me-components/testme.me ; \
	cp src/paks/me-components/vxworks.me $(ME_VAPP_PREFIX)/bin/paks/me-components/vxworks.me ; \
	cp src/paks/me-components/winsdk.me $(ME_VAPP_PREFIX)/bin/paks/me-components/winsdk.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-configuration" ; \
	cp src/paks/me-configuration/Configuration.es $(ME_VAPP_PREFIX)/bin/paks/me-configuration/Configuration.es ; \
	cp src/paks/me-configuration/configuration.me $(ME_VAPP_PREFIX)/bin/paks/me-configuration/configuration.me ; \
	cp src/paks/me-configuration/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-configuration/LICENSE.md ; \
	cp src/paks/me-configuration/package.json $(ME_VAPP_PREFIX)/bin/paks/me-configuration/package.json ; \
	cp src/paks/me-configuration/README.md $(ME_VAPP_PREFIX)/bin/paks/me-configuration/README.md ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-os" ; \
	cp src/paks/me-os/freebsd.me $(ME_VAPP_PREFIX)/bin/paks/me-os/freebsd.me ; \
	cp src/paks/me-os/gcc.me $(ME_VAPP_PREFIX)/bin/paks/me-os/gcc.me ; \
	cp src/paks/me-os/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-os/LICENSE.md ; \
	cp src/paks/me-os/linux.me $(ME_VAPP_PREFIX)/bin/paks/me-os/linux.me ; \
	cp src/paks/me-os/macosx.me $(ME_VAPP_PREFIX)/bin/paks/me-os/macosx.me ; \
	cp src/paks/me-os/os.me $(ME_VAPP_PREFIX)/bin/paks/me-os/os.me ; \
	cp src/paks/me-os/package.json $(ME_VAPP_PREFIX)/bin/paks/me-os/package.json ; \
	cp src/paks/me-os/README.md $(ME_VAPP_PREFIX)/bin/paks/me-os/README.md ; \
	cp src/paks/me-os/solaris.me $(ME_VAPP_PREFIX)/bin/paks/me-os/solaris.me ; \
	cp src/paks/me-os/unix.me $(ME_VAPP_PREFIX)/bin/paks/me-os/unix.me ; \
	cp src/paks/me-os/vxworks.me $(ME_VAPP_PREFIX)/bin/paks/me-os/vxworks.me ; \
	cp src/paks/me-os/windows.me $(ME_VAPP_PREFIX)/bin/paks/me-os/windows.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-package" ; \
	cp src/paks/me-package/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-package/LICENSE.md ; \
	cp src/paks/me-package/manifest.me $(ME_VAPP_PREFIX)/bin/paks/me-package/manifest.me ; \
	cp src/paks/me-package/Package.es $(ME_VAPP_PREFIX)/bin/paks/me-package/Package.es ; \
	cp src/paks/me-package/package.json $(ME_VAPP_PREFIX)/bin/paks/me-package/package.json ; \
	cp src/paks/me-package/package.me $(ME_VAPP_PREFIX)/bin/paks/me-package/package.me ; \
	cp src/paks/me-package/README.md $(ME_VAPP_PREFIX)/bin/paks/me-package/README.md ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-project" ; \
	cp src/paks/me-project/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-project/LICENSE.md ; \
	cp src/paks/me-project/package.json $(ME_VAPP_PREFIX)/bin/paks/me-project/package.json ; \
	cp src/paks/me-project/Project.es $(ME_VAPP_PREFIX)/bin/paks/me-project/Project.es ; \
	cp src/paks/me-project/project.me $(ME_VAPP_PREFIX)/bin/paks/me-project/project.me ; \
	cp src/paks/me-project/README.md $(ME_VAPP_PREFIX)/bin/paks/me-project/README.md ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-vstudio" ; \
	cp src/paks/me-vstudio/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/LICENSE.md ; \
	cp src/paks/me-vstudio/package.json $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/package.json ; \
	cp src/paks/me-vstudio/README.md $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/README.md ; \
	cp src/paks/me-vstudio/Vstudio.es $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/Vstudio.es ; \
	cp src/paks/me-vstudio/vstudio.me $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/vstudio.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-xcode" ; \
	cp src/paks/me-xcode/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-xcode/LICENSE.md ; \
	cp src/paks/me-xcode/package.json $(ME_VAPP_PREFIX)/bin/paks/me-xcode/package.json ; \
	cp src/paks/me-xcode/README.md $(ME_VAPP_PREFIX)/bin/paks/me-xcode/README.md ; \
	cp src/paks/me-xcode/Xcode.es $(ME_VAPP_PREFIX)/bin/paks/me-xcode/Xcode.es ; \
	cp src/paks/me-xcode/xcode.me $(ME_VAPP_PREFIX)/bin/paks/me-xcode/xcode.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/public/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	cp doc/public/man/testme.1 $(ME_VAPP_PREFIX)/doc/man/man1/testme.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/testme.1" "$(ME_MAN_PREFIX)/man1/testme.1"

#
#   start
#

start: $(DEPS_56)

#
#   install
#
DEPS_57 += stop
DEPS_57 += installBinary
DEPS_57 += start

install: $(DEPS_57)

#
#   uninstall
#
DEPS_58 += stop

uninstall: $(DEPS_58)
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true

#
#   version
#

version: $(DEPS_59)
	echo 0.8.5

