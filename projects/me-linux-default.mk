#
#   me-linux-default.mk -- Makefile to build Embedthis MakeMe for linux
#

NAME                  := me
VERSION               := 0.8.4
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

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EJS=$(ME_COM_EJS) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS              += -L$(BUILD)/bin
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
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


ifeq ($(ME_COM_EJS),1)
    TARGETS           += $(BUILD)/bin/ejs.mod
endif
TARGETS               += $(BUILD)/bin/ejs.testme.mod
ifeq ($(ME_COM_EJS),1)
    TARGETS           += $(BUILD)/bin/ejs
endif
TARGETS               += $(BUILD)/bin/ca.crt
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(BUILD)/bin/http
endif
ifeq ($(ME_COM_EST),1)
    TARGETS           += $(BUILD)/bin/libest.so
endif
TARGETS               += $(BUILD)/bin/libmprssl.so
TARGETS               += $(BUILD)/bin/libtestme.so
TARGETS               += $(BUILD)/bin/me
TARGETS               += $(BUILD)/bin/testme

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
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/me-linux-default-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/me-linux-default-me.h >/dev/null ; then\
		cp projects/me-linux-default-me.h $(BUILD)/inc/me.h  ; \
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
	rm -f "$(BUILD)/bin/ejsc"
	rm -f "$(BUILD)/bin/ejs"
	rm -f "$(BUILD)/bin/ca.crt"
	rm -f "$(BUILD)/bin/http"
	rm -f "$(BUILD)/bin/libejs.so"
	rm -f "$(BUILD)/bin/libest.so"
	rm -f "$(BUILD)/bin/libhttp.so"
	rm -f "$(BUILD)/bin/libmpr.so"
	rm -f "$(BUILD)/bin/libmprssl.so"
	rm -f "$(BUILD)/bin/libpcre.so"
	rm -f "$(BUILD)/bin/libtestme.so"
	rm -f "$(BUILD)/bin/libzlib.so"
	rm -f "$(BUILD)/bin/testme"

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
	$(CC) -c -o $(BUILD)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

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
	$(CC) -c -o $(BUILD)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

#
#   ejsc.o
#
DEPS_16 += src/paks/ejs/ejs.h

$(BUILD)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_16)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c -o $(BUILD)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

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
	$(CC) -c -o $(BUILD)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

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
	$(CC) -c -o $(BUILD)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   httpLib.o
#
DEPS_21 += src/paks/http/http.h

$(BUILD)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_21)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

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
	$(CC) -c -o $(BUILD)/obj/libtestme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/libtestme.c

#
#   me.o
#
DEPS_24 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/me.o: \
    src/me.c $(DEPS_24)
	@echo '   [Compile] $(BUILD)/obj/me.o'
	$(CC) -c -o $(BUILD)/obj/me.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/me.c

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
	$(CC) -c -o $(BUILD)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   mprSsl.o
#
DEPS_27 += $(BUILD)/inc/me.h
DEPS_27 += src/paks/mpr/mpr.h

$(BUILD)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_27)
	@echo '   [Compile] $(BUILD)/obj/mprSsl.o'
	$(CC) -c -o $(BUILD)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/paks/mpr/mprSsl.c

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
	$(CC) -c -o $(BUILD)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

#
#   testme.o
#
DEPS_30 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/testme.o: \
    src/tm/testme.c $(DEPS_30)
	@echo '   [Compile] $(BUILD)/obj/testme.o'
	$(CC) -c -o $(BUILD)/obj/testme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/testme.c

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
	$(CC) -c -o $(BUILD)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

#
#   libmpr
#
DEPS_33 += $(BUILD)/inc/mpr.h
DEPS_33 += $(BUILD)/obj/mprLib.o

$(BUILD)/bin/libmpr.so: $(DEPS_33)
	@echo '      [Link] $(BUILD)/bin/libmpr.so'
	$(CC) -shared -o $(BUILD)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/mprLib.o" $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_34 += $(BUILD)/inc/pcre.h
DEPS_34 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.so: $(DEPS_34)
	@echo '      [Link] $(BUILD)/bin/libpcre.so'
	$(CC) -shared -o $(BUILD)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_35 += $(BUILD)/bin/libmpr.so
ifeq ($(ME_COM_PCRE),1)
    DEPS_35 += $(BUILD)/bin/libpcre.so
endif
DEPS_35 += $(BUILD)/inc/http.h
DEPS_35 += $(BUILD)/obj/httpLib.o

LIBS_35 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_35 += -lpcre
endif

$(BUILD)/bin/libhttp.so: $(DEPS_35)
	@echo '      [Link] $(BUILD)/bin/libhttp.so'
	$(CC) -shared -o $(BUILD)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/httpLib.o" $(LIBPATHS_35) $(LIBS_35) $(LIBS_35) $(LIBS) 
endif

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_36 += $(BUILD)/inc/zlib.h
DEPS_36 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.so: $(DEPS_36)
	@echo '      [Link] $(BUILD)/bin/libzlib.so'
	$(CC) -shared -o $(BUILD)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   libejs
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_37 += $(BUILD)/bin/libhttp.so
endif
ifeq ($(ME_COM_PCRE),1)
    DEPS_37 += $(BUILD)/bin/libpcre.so
endif
DEPS_37 += $(BUILD)/bin/libmpr.so
ifeq ($(ME_COM_ZLIB),1)
    DEPS_37 += $(BUILD)/bin/libzlib.so
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

$(BUILD)/bin/libejs.so: $(DEPS_37)
	@echo '      [Link] $(BUILD)/bin/libejs.so'
	$(CC) -shared -o $(BUILD)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsLib.o" $(LIBPATHS_37) $(LIBS_37) $(LIBS_37) $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   ejsc
#
DEPS_38 += $(BUILD)/bin/libejs.so
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
	$(CC) -o $(BUILD)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsc.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) $(LIBS) 
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
	../../../build/macosx-x64-debug/bin/ejsc --out ../../../$(BUILD)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.mod
#
DEPS_40 += src/tm/ejs.testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_40 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/ejs.testme.mod: $(DEPS_40)
	( \
	cd src/tm; \
	echo '   [Compile] ejs.testme.mod' ; \
	../../build/macosx-x64-debug/bin/ejsc --debug --out ../../$(BUILD)/bin/ejs.testme.mod --optimize 9 ejs.testme.es ; \
	)

ifeq ($(ME_COM_EJS),1)
#
#   ejscmd
#
DEPS_41 += $(BUILD)/bin/libejs.so
DEPS_41 += $(BUILD)/obj/ejs.o

LIBS_41 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_41 += -lhttp
endif
LIBS_41 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_41 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_41 += -lzlib
endif

$(BUILD)/bin/ejs: $(DEPS_41)
	@echo '      [Link] $(BUILD)/bin/ejs'
	$(CC) -o $(BUILD)/bin/ejs $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejs.o" $(LIBPATHS_41) $(LIBS_41) $(LIBS_41) $(LIBS) $(LIBS) 
endif


#
#   http-ca-crt
#
DEPS_42 += src/paks/http/ca.crt

$(BUILD)/bin/ca.crt: $(DEPS_42)
	@echo '      [Copy] $(BUILD)/bin/ca.crt'
	mkdir -p "$(BUILD)/bin"
	cp src/paks/http/ca.crt $(BUILD)/bin/ca.crt

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_43 += $(BUILD)/bin/libhttp.so
DEPS_43 += $(BUILD)/obj/http.o

LIBS_43 += -lhttp
LIBS_43 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_43 += -lpcre
endif

$(BUILD)/bin/http: $(DEPS_43)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/http.o" $(LIBPATHS_43) $(LIBS_43) $(LIBS_43) $(LIBS) $(LIBS) 
endif

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_44 += $(BUILD)/inc/est.h
DEPS_44 += $(BUILD)/obj/estLib.o

$(BUILD)/bin/libest.so: $(DEPS_44)
	@echo '      [Link] $(BUILD)/bin/libest.so'
	$(CC) -shared -o $(BUILD)/bin/libest.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/estLib.o" $(LIBS) 
endif

#
#   libmprssl
#
DEPS_45 += $(BUILD)/bin/libmpr.so
DEPS_45 += $(BUILD)/obj/mprSsl.o

LIBS_45 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_45 += -lssl
    LIBPATHS_45 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_45 += -lcrypto
    LIBPATHS_45 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_EST),1)
    LIBS_45 += -lest
endif

$(BUILD)/bin/libmprssl.so: $(DEPS_45)
	@echo '      [Link] $(BUILD)/bin/libmprssl.so'
	$(CC) -shared -o $(BUILD)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/mprSsl.o" $(LIBPATHS_45) $(LIBS_45) $(LIBS_45) $(LIBS) 

#
#   libtestme
#
DEPS_46 += $(BUILD)/inc/testme.h
DEPS_46 += $(BUILD)/obj/libtestme.o

$(BUILD)/bin/libtestme.so: $(DEPS_46)
	@echo '      [Link] $(BUILD)/bin/libtestme.so'
	$(CC) -shared -o $(BUILD)/bin/libtestme.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/libtestme.o" $(LIBS) 

#
#   me.mod
#
DEPS_47 += src/Builder.es
DEPS_47 += src/Loader.es
DEPS_47 += src/MakeMe.es
DEPS_47 += src/Me.es
DEPS_47 += src/Script.es
DEPS_47 += src/Target.es
DEPS_47 += src/paks/ejs-version/Version.es
ifeq ($(ME_COM_EJS),1)
    DEPS_47 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/me.mod: $(DEPS_47)
	( \
	cd .; \
	echo '   [Compile] me.mod' ; \
	./build/macosx-x64-debug/bin/ejsc --debug --out ./$(BUILD)/bin/me.mod --optimize 9 src/Builder.es src/Loader.es src/MakeMe.es src/Me.es src/Script.es src/Target.es src/paks/ejs-version/Version.es ; \
	)

#
#   runtime
#
DEPS_48 += src/runtime/configure/appweb.me
DEPS_48 += src/runtime/configure/compiler.me
DEPS_48 += src/runtime/configure/lib.me
DEPS_48 += src/runtime/configure/link.me
DEPS_48 += src/runtime/configure/rc.me
DEPS_48 += src/runtime/configure/testme.me
DEPS_48 += src/runtime/configure/vxworks.me
DEPS_48 += src/runtime/configure/winsdk.me
DEPS_48 += src/runtime/master-main.me
DEPS_48 += src/runtime/master-start.me
DEPS_48 += src/runtime/os/freebsd.me
DEPS_48 += src/runtime/os/gcc.me
DEPS_48 += src/runtime/os/linux.me
DEPS_48 += src/runtime/os/macosx.me
DEPS_48 += src/runtime/os/solaris.me
DEPS_48 += src/runtime/os/unix.me
DEPS_48 += src/runtime/os/vxworks.me
DEPS_48 += src/runtime/os/windows.me
DEPS_48 += src/runtime/plugins/Configure.es
DEPS_48 += src/runtime/plugins/Generator.es
DEPS_48 += src/runtime/plugins/Vstudio.es
DEPS_48 += src/runtime/plugins/Xcode.es
DEPS_48 += src/runtime/simple.me
DEPS_48 += src/runtime/standard.me

$(BUILD)/.modify-runtime: $(DEPS_48)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin/configure"
	cp src/runtime/configure/appweb.me $(BUILD)/bin/configure/appweb.me
	cp src/runtime/configure/compiler.me $(BUILD)/bin/configure/compiler.me
	cp src/runtime/configure/lib.me $(BUILD)/bin/configure/lib.me
	cp src/runtime/configure/link.me $(BUILD)/bin/configure/link.me
	cp src/runtime/configure/rc.me $(BUILD)/bin/configure/rc.me
	cp src/runtime/configure/testme.me $(BUILD)/bin/configure/testme.me
	cp src/runtime/configure/vxworks.me $(BUILD)/bin/configure/vxworks.me
	cp src/runtime/configure/winsdk.me $(BUILD)/bin/configure/winsdk.me
	mkdir -p "$(BUILD)/bin"
	cp src/runtime/master-main.me $(BUILD)/bin/master-main.me
	cp src/runtime/master-start.me $(BUILD)/bin/master-start.me
	mkdir -p "$(BUILD)/bin/os"
	cp src/runtime/os/freebsd.me $(BUILD)/bin/os/freebsd.me
	cp src/runtime/os/gcc.me $(BUILD)/bin/os/gcc.me
	cp src/runtime/os/linux.me $(BUILD)/bin/os/linux.me
	cp src/runtime/os/macosx.me $(BUILD)/bin/os/macosx.me
	cp src/runtime/os/solaris.me $(BUILD)/bin/os/solaris.me
	cp src/runtime/os/unix.me $(BUILD)/bin/os/unix.me
	cp src/runtime/os/vxworks.me $(BUILD)/bin/os/vxworks.me
	cp src/runtime/os/windows.me $(BUILD)/bin/os/windows.me
	mkdir -p "$(BUILD)/bin/plugins"
	cp src/runtime/plugins/Configure.es $(BUILD)/bin/plugins/Configure.es
	cp src/runtime/plugins/Generator.es $(BUILD)/bin/plugins/Generator.es
	cp src/runtime/plugins/Vstudio.es $(BUILD)/bin/plugins/Vstudio.es
	cp src/runtime/plugins/Xcode.es $(BUILD)/bin/plugins/Xcode.es
	cp src/runtime/simple.me $(BUILD)/bin/simple.me
	cp src/runtime/standard.me $(BUILD)/bin/standard.me
	touch "./$(BUILD)/.modify-runtime"

#
#   me
#
DEPS_49 += $(BUILD)/bin/libmpr.so
ifeq ($(ME_COM_HTTP),1)
    DEPS_49 += $(BUILD)/bin/libhttp.so
endif
ifeq ($(ME_COM_EJS),1)
    DEPS_49 += $(BUILD)/bin/libejs.so
endif
DEPS_49 += $(BUILD)/bin/me.mod
DEPS_49 += $(BUILD)/bin
DEPS_49 += $(BUILD)/obj/me.o

LIBS_49 += -lmpr
ifeq ($(ME_COM_HTTP),1)
    LIBS_49 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_49 += -lpcre
endif
ifeq ($(ME_COM_EJS),1)
    LIBS_49 += -lejs
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_49 += -lzlib
endif

$(BUILD)/bin/me: $(DEPS_49)
	@echo '      [Link] $(BUILD)/bin/me'
	$(CC) -o $(BUILD)/bin/me $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/me.o" $(LIBPATHS_49) $(LIBS_49) $(LIBS_49) $(LIBS) $(LIBS) 

#
#   testme.mod
#
DEPS_50 += src/tm/testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_50 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/testme.mod: $(DEPS_50)
	( \
	cd src/tm; \
	echo '   [Compile] testme.mod' ; \
	../../build/macosx-x64-debug/bin/ejsc --debug --out ../../$(BUILD)/bin/testme.mod --optimize 9 testme.es ; \
	)

#
#   testme
#
ifeq ($(ME_COM_EJS),1)
    DEPS_51 += $(BUILD)/bin/libejs.so
endif
DEPS_51 += $(BUILD)/bin/testme.mod
DEPS_51 += $(BUILD)/bin/ejs.testme.mod
DEPS_51 += $(BUILD)/obj/testme.o

ifeq ($(ME_COM_EJS),1)
    LIBS_51 += -lejs
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_51 += -lhttp
endif
LIBS_51 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_51 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_51 += -lzlib
endif

$(BUILD)/bin/testme: $(DEPS_51)
	@echo '      [Link] $(BUILD)/bin/testme'
	$(CC) -o $(BUILD)/bin/testme $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/testme.o" $(LIBPATHS_51) $(LIBS_51) $(LIBS_51) $(LIBS) $(LIBS) 


#
#   installBinary
#
installBinary: $(DEPS_52)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.8.4" "$(ME_APP_PREFIX)/latest" ; \
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
	cp $(BUILD)/bin/ejsc $(ME_VAPP_PREFIX)/bin/ejsc ; \
	cp $(BUILD)/bin/http $(ME_VAPP_PREFIX)/bin/http ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
	cp $(BUILD)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(BUILD)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(BUILD)/bin/libmprssl.so $(ME_VAPP_PREFIX)/bin/libmprssl.so ; \
	cp $(BUILD)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	cp $(BUILD)/bin/libzlib.so $(ME_VAPP_PREFIX)/bin/libzlib.so ; \
	cp $(BUILD)/bin/libtestme.so $(ME_VAPP_PREFIX)/bin/libtestme.so ; \
	if [ "$(ME_COM_EST)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libest.so $(ME_VAPP_PREFIX)/bin/libest.so ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libssl*.so* $(ME_VAPP_PREFIX)/bin/libssl*.so* ; \
	cp $(BUILD)/bin/libcrypto*.so* $(ME_VAPP_PREFIX)/bin/libcrypto*.so* ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	cp $(BUILD)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp $(BUILD)/bin/me.mod $(ME_VAPP_PREFIX)/bin/me.mod ; \
	cp $(BUILD)/bin/testme.mod $(ME_VAPP_PREFIX)/bin/testme.mod ; \
	cp $(BUILD)/bin/ejs.testme.mod $(ME_VAPP_PREFIX)/bin/ejs.testme.mod ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp src/tm/testme.h $(ME_VAPP_PREFIX)/inc/testme.h ; \
	mkdir -p "/usr/src/runtime" ; \
	cp src/runtime/configure /usr/src/runtime/configure ; \
	mkdir -p "/usr/src/runtime/configure" ; \
	cp src/runtime/configure/appweb.me /usr/src/runtime/configure/appweb.me ; \
	cp src/runtime/configure/compiler.me /usr/src/runtime/configure/compiler.me ; \
	cp src/runtime/configure/lib.me /usr/src/runtime/configure/lib.me ; \
	cp src/runtime/configure/link.me /usr/src/runtime/configure/link.me ; \
	cp src/runtime/configure/rc.me /usr/src/runtime/configure/rc.me ; \
	cp src/runtime/configure/testme.me /usr/src/runtime/configure/testme.me ; \
	cp src/runtime/configure/vxworks.me /usr/src/runtime/configure/vxworks.me ; \
	cp src/runtime/configure/winsdk.me /usr/src/runtime/configure/winsdk.me ; \
	cp src/runtime/master-main.me /usr/src/runtime/master-main.me ; \
	cp src/runtime/master-start.me /usr/src/runtime/master-start.me ; \
	cp src/runtime/os /usr/src/runtime/os ; \
	mkdir -p "/usr/src/runtime/os" ; \
	cp src/runtime/os/freebsd.me /usr/src/runtime/os/freebsd.me ; \
	cp src/runtime/os/gcc.me /usr/src/runtime/os/gcc.me ; \
	cp src/runtime/os/linux.me /usr/src/runtime/os/linux.me ; \
	cp src/runtime/os/macosx.me /usr/src/runtime/os/macosx.me ; \
	cp src/runtime/os/solaris.me /usr/src/runtime/os/solaris.me ; \
	cp src/runtime/os/unix.me /usr/src/runtime/os/unix.me ; \
	cp src/runtime/os/vxworks.me /usr/src/runtime/os/vxworks.me ; \
	cp src/runtime/os/windows.me /usr/src/runtime/os/windows.me ; \
	cp src/runtime/plugins /usr/src/runtime/plugins ; \
	mkdir -p "/usr/src/runtime/plugins" ; \
	cp src/runtime/plugins/Configure.es /usr/src/runtime/plugins/Configure.es ; \
	cp src/runtime/plugins/Generator.es /usr/src/runtime/plugins/Generator.es ; \
	cp src/runtime/plugins/Vstudio.es /usr/src/runtime/plugins/Vstudio.es ; \
	cp src/runtime/plugins/Xcode.es /usr/src/runtime/plugins/Xcode.es ; \
	cp src/runtime/simple.me /usr/src/runtime/simple.me ; \
	cp src/runtime/standard.me /usr/src/runtime/standard.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/public/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	cp doc/public/man/testme.1 $(ME_VAPP_PREFIX)/doc/man/man1/testme.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/testme.1" "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	)


#
#   install
#
DEPS_53 += stop
DEPS_53 += installBinary
DEPS_53 += start

install: $(DEPS_53)

#
#   uninstall
#
DEPS_54 += stop

uninstall: $(DEPS_54)
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

#
#   version
#
version: $(DEPS_55)
	( \
	cd build/macosx-x64-release/bin; \
	echo 0.8.4 ; \
	)

