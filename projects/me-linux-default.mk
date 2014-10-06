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


TARGETS               += build/$(CONFIG)/bin
ifeq ($(ME_COM_EJS),1)
    TARGETS           += build/$(CONFIG)/bin/ejs.mod
endif
TARGETS               += build/$(CONFIG)/bin/ejs.testme.es
TARGETS               += build/$(CONFIG)/bin/ejs.testme.mod
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
	rm -f "build/$(CONFIG)/bin/ejs.testme.es"
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
	rm -f "build/$(CONFIG)/bin/testme.es"

clobber: clean
	rm -fr ./$(BUILD)


#
#   core
#
DEPS_1 += src/configure/appweb.me
DEPS_1 += src/configure/compiler.me
DEPS_1 += src/configure/lib.me
DEPS_1 += src/configure/link.me
DEPS_1 += src/configure/rc.me
DEPS_1 += src/configure/testme.me
DEPS_1 += src/configure/vxworks.me
DEPS_1 += src/configure/winsdk.me
DEPS_1 += src/configure.es
DEPS_1 += src/generate.es
DEPS_1 += src/master-main.me
DEPS_1 += src/master-start.me
DEPS_1 += src/me.es
DEPS_1 += src/os/freebsd.me
DEPS_1 += src/os/gcc.me
DEPS_1 += src/os/linux.me
DEPS_1 += src/os/macosx.me
DEPS_1 += src/os/solaris.me
DEPS_1 += src/os/unix.me
DEPS_1 += src/os/vxworks.me
DEPS_1 += src/os/windows.me
DEPS_1 += src/simple.me
DEPS_1 += src/standard.me
DEPS_1 += src/vstudio.es
DEPS_1 += src/xcode.es

.force: .PHONY

build/$(CONFIG)/bin: $(DEPS_1) .force
	@echo '      [Copy] build/$(CONFIG)/bin'
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/appweb.me build/$(CONFIG)/bin/configure/appweb.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/compiler.me build/$(CONFIG)/bin/configure/compiler.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/lib.me build/$(CONFIG)/bin/configure/lib.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/link.me build/$(CONFIG)/bin/configure/link.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/rc.me build/$(CONFIG)/bin/configure/rc.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/testme.me build/$(CONFIG)/bin/configure/testme.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/vxworks.me build/$(CONFIG)/bin/configure/vxworks.me
	mkdir -p "build/$(CONFIG)/bin/configure"
	cp src/configure/winsdk.me build/$(CONFIG)/bin/configure/winsdk.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/configure.es build/$(CONFIG)/bin/configure.es
	mkdir -p "build/$(CONFIG)/bin"
	cp src/generate.es build/$(CONFIG)/bin/generate.es
	mkdir -p "build/$(CONFIG)/bin"
	cp src/master-main.me build/$(CONFIG)/bin/master-main.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/master-start.me build/$(CONFIG)/bin/master-start.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/me.es build/$(CONFIG)/bin/me.es
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/freebsd.me build/$(CONFIG)/bin/os/freebsd.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/gcc.me build/$(CONFIG)/bin/os/gcc.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/linux.me build/$(CONFIG)/bin/os/linux.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/macosx.me build/$(CONFIG)/bin/os/macosx.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/solaris.me build/$(CONFIG)/bin/os/solaris.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/unix.me build/$(CONFIG)/bin/os/unix.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/vxworks.me build/$(CONFIG)/bin/os/vxworks.me
	mkdir -p "build/$(CONFIG)/bin/os"
	cp src/os/windows.me build/$(CONFIG)/bin/os/windows.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/simple.me build/$(CONFIG)/bin/simple.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/standard.me build/$(CONFIG)/bin/standard.me
	mkdir -p "build/$(CONFIG)/bin"
	cp src/vstudio.es build/$(CONFIG)/bin/vstudio.es
	mkdir -p "build/$(CONFIG)/bin"
	cp src/xcode.es build/$(CONFIG)/bin/xcode.es

#
#   me.h
#
build/$(CONFIG)/inc/me.h: $(DEPS_2)

#
#   osdep.h
#
DEPS_3 += src/paks/osdep/osdep.h
DEPS_3 += build/$(CONFIG)/inc/me.h

build/$(CONFIG)/inc/osdep.h: $(DEPS_3)
	@echo '      [Copy] build/$(CONFIG)/inc/osdep.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/osdep/osdep.h build/$(CONFIG)/inc/osdep.h

#
#   mpr.h
#
DEPS_4 += src/paks/mpr/mpr.h
DEPS_4 += build/$(CONFIG)/inc/me.h
DEPS_4 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/inc/mpr.h: $(DEPS_4)
	@echo '      [Copy] build/$(CONFIG)/inc/mpr.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h build/$(CONFIG)/inc/mpr.h

#
#   mpr.h
#
src/paks/mpr/mpr.h: $(DEPS_5)

#
#   mprLib.o
#
DEPS_6 += src/paks/mpr/mpr.h

build/$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_6)
	@echo '   [Compile] build/$(CONFIG)/obj/mprLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_7 += build/$(CONFIG)/inc/mpr.h
DEPS_7 += build/$(CONFIG)/obj/mprLib.o

build/$(CONFIG)/bin/libmpr.so: $(DEPS_7)
	@echo '      [Link] build/$(CONFIG)/bin/libmpr.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/mprLib.o" $(LIBS) 

#
#   pcre.h
#
DEPS_8 += src/paks/pcre/pcre.h

build/$(CONFIG)/inc/pcre.h: $(DEPS_8)
	@echo '      [Copy] build/$(CONFIG)/inc/pcre.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h build/$(CONFIG)/inc/pcre.h

#
#   pcre.h
#
src/paks/pcre/pcre.h: $(DEPS_9)

#
#   pcre.o
#
DEPS_10 += build/$(CONFIG)/inc/me.h
DEPS_10 += src/paks/pcre/pcre.h

build/$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_10)
	@echo '   [Compile] build/$(CONFIG)/obj/pcre.o'
	$(CC) -c -o build/$(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_11 += build/$(CONFIG)/inc/pcre.h
DEPS_11 += build/$(CONFIG)/obj/pcre.o

build/$(CONFIG)/bin/libpcre.so: $(DEPS_11)
	@echo '      [Link] build/$(CONFIG)/bin/libpcre.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/pcre.o" $(LIBS) 
endif

#
#   http.h
#
DEPS_12 += src/paks/http/http.h
DEPS_12 += build/$(CONFIG)/inc/mpr.h

build/$(CONFIG)/inc/http.h: $(DEPS_12)
	@echo '      [Copy] build/$(CONFIG)/inc/http.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/http/http.h build/$(CONFIG)/inc/http.h

#
#   http.h
#
src/paks/http/http.h: $(DEPS_13)

#
#   httpLib.o
#
DEPS_14 += src/paks/http/http.h

build/$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_14)
	@echo '   [Compile] build/$(CONFIG)/obj/httpLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_15 += build/$(CONFIG)/bin/libmpr.so
ifeq ($(ME_COM_PCRE),1)
    DEPS_15 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_15 += build/$(CONFIG)/inc/http.h
DEPS_15 += build/$(CONFIG)/obj/httpLib.o

LIBS_15 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_15 += -lpcre
endif

build/$(CONFIG)/bin/libhttp.so: $(DEPS_15)
	@echo '      [Link] build/$(CONFIG)/bin/libhttp.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/httpLib.o" $(LIBPATHS_15) $(LIBS_15) $(LIBS_15) $(LIBS) 
endif

#
#   zlib.h
#
DEPS_16 += src/paks/zlib/zlib.h
DEPS_16 += build/$(CONFIG)/inc/me.h

build/$(CONFIG)/inc/zlib.h: $(DEPS_16)
	@echo '      [Copy] build/$(CONFIG)/inc/zlib.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h build/$(CONFIG)/inc/zlib.h

#
#   zlib.h
#
src/paks/zlib/zlib.h: $(DEPS_17)

#
#   zlib.o
#
DEPS_18 += build/$(CONFIG)/inc/me.h
DEPS_18 += src/paks/zlib/zlib.h

build/$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_18)
	@echo '   [Compile] build/$(CONFIG)/obj/zlib.o'
	$(CC) -c -o build/$(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_19 += build/$(CONFIG)/inc/zlib.h
DEPS_19 += build/$(CONFIG)/obj/zlib.o

build/$(CONFIG)/bin/libzlib.so: $(DEPS_19)
	@echo '      [Link] build/$(CONFIG)/bin/libzlib.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   ejs.slots.h
#
src/paks/ejs/ejs.slots.h: $(DEPS_20)

#
#   ejs.h
#
DEPS_21 += src/paks/ejs/ejs.h
DEPS_21 += build/$(CONFIG)/inc/me.h
DEPS_21 += build/$(CONFIG)/inc/osdep.h
DEPS_21 += build/$(CONFIG)/inc/mpr.h
DEPS_21 += build/$(CONFIG)/inc/http.h
DEPS_21 += src/paks/ejs/ejs.slots.h
DEPS_21 += build/$(CONFIG)/inc/pcre.h
DEPS_21 += build/$(CONFIG)/inc/zlib.h

build/$(CONFIG)/inc/ejs.h: $(DEPS_21)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejs.h build/$(CONFIG)/inc/ejs.h

#
#   ejs.slots.h
#
DEPS_22 += src/paks/ejs/ejs.slots.h

build/$(CONFIG)/inc/ejs.slots.h: $(DEPS_22)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejs.slots.h build/$(CONFIG)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
DEPS_23 += src/paks/ejs/ejsByteGoto.h

build/$(CONFIG)/inc/ejsByteGoto.h: $(DEPS_23)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsByteGoto.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/ejs/ejsByteGoto.h build/$(CONFIG)/inc/ejsByteGoto.h

#
#   ejs.h
#
src/paks/ejs/ejs.h: $(DEPS_24)

#
#   ejsLib.o
#
DEPS_25 += src/paks/ejs/ejs.h
DEPS_25 += build/$(CONFIG)/inc/mpr.h
DEPS_25 += build/$(CONFIG)/inc/pcre.h
DEPS_25 += build/$(CONFIG)/inc/me.h

build/$(CONFIG)/obj/ejsLib.o: \
    src/paks/ejs/ejsLib.c $(DEPS_25)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejsLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsLib.c

ifeq ($(ME_COM_EJS),1)
#
#   libejs
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_26 += build/$(CONFIG)/bin/libhttp.so
endif
ifeq ($(ME_COM_PCRE),1)
    DEPS_26 += build/$(CONFIG)/bin/libpcre.so
endif
DEPS_26 += build/$(CONFIG)/bin/libmpr.so
ifeq ($(ME_COM_ZLIB),1)
    DEPS_26 += build/$(CONFIG)/bin/libzlib.so
endif
DEPS_26 += build/$(CONFIG)/inc/ejs.h
DEPS_26 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_26 += build/$(CONFIG)/inc/ejsByteGoto.h
DEPS_26 += build/$(CONFIG)/obj/ejsLib.o

ifeq ($(ME_COM_HTTP),1)
    LIBS_26 += -lhttp
endif
LIBS_26 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_26 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_26 += -lzlib
endif

build/$(CONFIG)/bin/libejs.so: $(DEPS_26)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsLib.o" $(LIBPATHS_26) $(LIBS_26) $(LIBS_26) $(LIBS) 
endif

#
#   ejsc.o
#
DEPS_27 += src/paks/ejs/ejs.h

build/$(CONFIG)/obj/ejsc.o: \
    src/paks/ejs/ejsc.c $(DEPS_27)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsc.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejsc.c

ifeq ($(ME_COM_EJS),1)
#
#   ejsc
#
DEPS_28 += build/$(CONFIG)/bin/libejs.so
DEPS_28 += build/$(CONFIG)/obj/ejsc.o

LIBS_28 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_28 += -lhttp
endif
LIBS_28 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_28 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_28 += -lzlib
endif

build/$(CONFIG)/bin/ejsc: $(DEPS_28)
	@echo '      [Link] build/$(CONFIG)/bin/ejsc'
	$(CC) -o build/$(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsc.o" $(LIBPATHS_28) $(LIBS_28) $(LIBS_28) $(LIBS) $(LIBS) 
endif

ifeq ($(ME_COM_EJS),1)
#
#   ejs.mod
#
DEPS_29 += src/paks/ejs/ejs.es
DEPS_29 += build/$(CONFIG)/bin/ejsc

build/$(CONFIG)/bin/ejs.mod: $(DEPS_29)
	( \
	cd src/paks/ejs; \
	echo '   [Compile] ejs.mod' ; \
	../../../build/$(CONFIG)/bin/ejsc --debug --out ../../../build/$(CONFIG)/bin/ejs.mod --optimize 9 --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.es
#
DEPS_30 += src/tm/ejs.testme.es

build/$(CONFIG)/bin/ejs.testme.es: $(DEPS_30)
	@echo '      [Copy] build/$(CONFIG)/bin/ejs.testme.es'
	mkdir -p "build/$(CONFIG)/bin"
	cp src/tm/ejs.testme.es build/$(CONFIG)/bin/ejs.testme.es

#
#   ejs.testme.mod
#
DEPS_31 += src/tm/ejs.testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_31 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/ejs.testme.mod: $(DEPS_31)
	( \
	cd src/tm; \
	echo '   [Compile] ejs.testme.mod' ; \
	../../build/$(CONFIG)/bin/ejsc --debug --out ../../build/$(CONFIG)/bin/ejs.testme.mod --optimize 9 ejs.testme.es ; \
	)

#
#   ejs.o
#
DEPS_32 += src/paks/ejs/ejs.h

build/$(CONFIG)/obj/ejs.o: \
    src/paks/ejs/ejs.c $(DEPS_32)
	@echo '   [Compile] build/$(CONFIG)/obj/ejs.o'
	$(CC) -c -o build/$(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/ejs/ejs.c

ifeq ($(ME_COM_EJS),1)
#
#   ejscmd
#
DEPS_33 += build/$(CONFIG)/bin/libejs.so
DEPS_33 += build/$(CONFIG)/obj/ejs.o

LIBS_33 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_33 += -lhttp
endif
LIBS_33 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_33 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_33 += -lzlib
endif

build/$(CONFIG)/bin/ejs: $(DEPS_33)
	@echo '      [Link] build/$(CONFIG)/bin/ejs'
	$(CC) -o build/$(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejs.o" $(LIBPATHS_33) $(LIBS_33) $(LIBS_33) $(LIBS) $(LIBS) 
endif


#
#   http-ca-crt
#
DEPS_34 += src/paks/http/ca.crt

build/$(CONFIG)/bin/ca.crt: $(DEPS_34)
	@echo '      [Copy] build/$(CONFIG)/bin/ca.crt'
	mkdir -p "build/$(CONFIG)/bin"
	cp src/paks/http/ca.crt build/$(CONFIG)/bin/ca.crt

#
#   http.o
#
DEPS_35 += src/paks/http/http.h

build/$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_35)
	@echo '   [Compile] build/$(CONFIG)/obj/http.o'
	$(CC) -c -o build/$(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_36 += build/$(CONFIG)/bin/libhttp.so
DEPS_36 += build/$(CONFIG)/obj/http.o

LIBS_36 += -lhttp
LIBS_36 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_36 += -lpcre
endif

build/$(CONFIG)/bin/http: $(DEPS_36)
	@echo '      [Link] build/$(CONFIG)/bin/http'
	$(CC) -o build/$(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/http.o" $(LIBPATHS_36) $(LIBS_36) $(LIBS_36) $(LIBS) $(LIBS) 
endif

#
#   est.h
#
DEPS_37 += src/paks/est/est.h

build/$(CONFIG)/inc/est.h: $(DEPS_37)
	@echo '      [Copy] build/$(CONFIG)/inc/est.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/est/est.h build/$(CONFIG)/inc/est.h

#
#   est.h
#
src/paks/est/est.h: $(DEPS_38)

#
#   estLib.o
#
DEPS_39 += src/paks/est/est.h

build/$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_39)
	@echo '   [Compile] build/$(CONFIG)/obj/estLib.o'
	$(CC) -c -o build/$(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_40 += build/$(CONFIG)/inc/est.h
DEPS_40 += build/$(CONFIG)/obj/estLib.o

build/$(CONFIG)/bin/libest.so: $(DEPS_40)
	@echo '      [Link] build/$(CONFIG)/bin/libest.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   mprSsl.o
#
DEPS_41 += build/$(CONFIG)/inc/me.h
DEPS_41 += src/paks/mpr/mpr.h

build/$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_41)
	@echo '   [Compile] build/$(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o build/$(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_42 += build/$(CONFIG)/bin/libmpr.so
DEPS_42 += build/$(CONFIG)/obj/mprSsl.o

LIBS_42 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_42 += -lssl
    LIBPATHS_42 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_42 += -lcrypto
    LIBPATHS_42 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_EST),1)
    LIBS_42 += -lest
endif

build/$(CONFIG)/bin/libmprssl.so: $(DEPS_42)
	@echo '      [Link] build/$(CONFIG)/bin/libmprssl.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS)  "build/$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_42) $(LIBS_42) $(LIBS_42) $(LIBS) 

#
#   testme.h
#
DEPS_43 += src/tm/testme.h

build/$(CONFIG)/inc/testme.h: $(DEPS_43)
	@echo '      [Copy] build/$(CONFIG)/inc/testme.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/tm/testme.h build/$(CONFIG)/inc/testme.h

#
#   testme.h
#
src/tm/testme.h: $(DEPS_44)

#
#   libtestme.o
#
DEPS_45 += src/tm/testme.h

build/$(CONFIG)/obj/libtestme.o: \
    src/tm/libtestme.c $(DEPS_45)
	@echo '   [Compile] build/$(CONFIG)/obj/libtestme.o'
	$(CC) -c -o build/$(CONFIG)/obj/libtestme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/libtestme.c

#
#   libtestme
#
DEPS_46 += build/$(CONFIG)/inc/testme.h
DEPS_46 += build/$(CONFIG)/obj/libtestme.o

build/$(CONFIG)/bin/libtestme.so: $(DEPS_46)
	@echo '      [Link] build/$(CONFIG)/bin/libtestme.so'
	$(CC) -shared -o build/$(CONFIG)/bin/libtestme.so $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/libtestme.o" $(LIBS) 

#
#   me.mod
#
DEPS_47 += src/me.es
DEPS_47 += src/paks/ejs-version/Version.es
ifeq ($(ME_COM_EJS),1)
    DEPS_47 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/me.mod: $(DEPS_47)
	( \
	cd .; \
	echo '   [Compile] me.mod' ; \
	./build/$(CONFIG)/bin/ejsc --debug --out ./build/$(CONFIG)/bin/me.mod --optimize 9 src/me.es src/paks/ejs-version/Version.es ; \
	)

#
#   me.o
#
DEPS_48 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/me.o: \
    src/me.c $(DEPS_48)
	@echo '   [Compile] build/$(CONFIG)/obj/me.o'
	$(CC) -c -o build/$(CONFIG)/obj/me.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/me.c

#
#   me
#
DEPS_49 += build/$(CONFIG)/bin/libmpr.so
ifeq ($(ME_COM_HTTP),1)
    DEPS_49 += build/$(CONFIG)/bin/libhttp.so
endif
ifeq ($(ME_COM_EJS),1)
    DEPS_49 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_49 += build/$(CONFIG)/bin/me.mod
DEPS_49 += build/$(CONFIG)/obj/me.o

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

build/$(CONFIG)/bin/me: $(DEPS_49)
	@echo '      [Link] build/$(CONFIG)/bin/me'
	$(CC) -o build/$(CONFIG)/bin/me $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/me.o" $(LIBPATHS_49) $(LIBS_49) $(LIBS_49) $(LIBS) $(LIBS) 

#
#   testme.mod
#
DEPS_50 += src/tm/testme.es
ifeq ($(ME_COM_EJS),1)
    DEPS_50 += build/$(CONFIG)/bin/ejsc
endif

build/$(CONFIG)/bin/testme.mod: $(DEPS_50)
	( \
	cd src/tm; \
	echo '   [Compile] testme.mod' ; \
	../../build/$(CONFIG)/bin/ejsc --debug --out ../../build/$(CONFIG)/bin/testme.mod --optimize 9 testme.es ; \
	)

#
#   testme.o
#
DEPS_51 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/testme.o: \
    src/tm/testme.c $(DEPS_51)
	@echo '   [Compile] build/$(CONFIG)/obj/testme.o'
	$(CC) -c -o build/$(CONFIG)/obj/testme.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/tm/testme.c

#
#   testme
#
ifeq ($(ME_COM_EJS),1)
    DEPS_52 += build/$(CONFIG)/bin/libejs.so
endif
DEPS_52 += build/$(CONFIG)/bin/testme.mod
DEPS_52 += build/$(CONFIG)/bin/ejs.testme.mod
DEPS_52 += build/$(CONFIG)/obj/testme.o

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

build/$(CONFIG)/bin/testme: $(DEPS_52)
	@echo '      [Link] build/$(CONFIG)/bin/testme'
	$(CC) -o build/$(CONFIG)/bin/testme $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/testme.o" $(LIBPATHS_52) $(LIBS_52) $(LIBS_52) $(LIBS) $(LIBS) 

#
#   testme.es
#
DEPS_53 += src/tm/testme.es

build/$(CONFIG)/bin/testme.es: $(DEPS_53)
	@echo '      [Copy] build/$(CONFIG)/bin/testme.es'
	mkdir -p "build/$(CONFIG)/bin"
	cp src/tm/testme.es build/$(CONFIG)/bin/testme.es

#
#   stop
#
stop: $(DEPS_54)

#
#   installBinary
#
installBinary: $(DEPS_55)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "0.8.4" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/me,./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/me,./build/$(CONFIG)/bin/testme $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/me,./build/$(CONFIG)/bin/testme ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/ejs,./build/$(CONFIG)/bin/ejsc,./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/ejs,./build/$(CONFIG)/bin/ejsc,./build/$(CONFIG)/bin/http $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/ejs,./build/$(CONFIG)/bin/ejsc,./build/$(CONFIG)/bin/http ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/libejs.so,./build/$(CONFIG)/bin/libhttp.so,./build/$(CONFIG)/bin/libmpr.so,./build/$(CONFIG)/bin/libmprssl.so,./build/$(CONFIG)/bin/libpcre.so,./build/$(CONFIG)/bin/libzlib.so,./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/libejs.so,./build/$(CONFIG)/bin/libhttp.so,./build/$(CONFIG)/bin/libmpr.so,./build/$(CONFIG)/bin/libmprssl.so,./build/$(CONFIG)/bin/libpcre.so,./build/$(CONFIG)/bin/libzlib.so,./build/$(CONFIG)/bin/libtestme.so $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/libejs.so,./build/$(CONFIG)/bin/libhttp.so,./build/$(CONFIG)/bin/libmpr.so,./build/$(CONFIG)/bin/libmprssl.so,./build/$(CONFIG)/bin/libpcre.so,./build/$(CONFIG)/bin/libzlib.so,./build/$(CONFIG)/bin/libtestme.so ; \
	if [ "$(ME_COM_EST)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/libest.so $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/libest.so ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/libssl*.so*,./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/libssl*.so*,./build/$(CONFIG)/bin/libcrypto*.so* $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/libssl*.so*,./build/$(CONFIG)/bin/libcrypto*.so* ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/ca.crt,./build/$(CONFIG)/bin/ejs.mod,./build/$(CONFIG)/bin/me.mod,./build/$(CONFIG)/bin/testme.mod,./build/$(CONFIG)/bin" ; \
	cp build/$(CONFIG)/bin/ca.crt,./build/$(CONFIG)/bin/ejs.mod,./build/$(CONFIG)/bin/me.mod,./build/$(CONFIG)/bin/testme.mod,./build/$(CONFIG)/bin/ejs.testme.mod $(ME_VAPP_PREFIX)/bin/./build/$(CONFIG)/bin/ca.crt,./build/$(CONFIG)/bin/ejs.mod,./build/$(CONFIG)/bin/me.mod,./build/$(CONFIG)/bin/testme.mod,./build/$(CONFIG)/bin/ejs.testme.mod ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc/src/tm" ; \
	cp src/tm/testme.h $(ME_VAPP_PREFIX)/inc/src/tm/testme.h ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/appweb.me $(ME_VAPP_PREFIX)/bin/src/configure/appweb.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/compiler.me $(ME_VAPP_PREFIX)/bin/src/configure/compiler.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/lib.me $(ME_VAPP_PREFIX)/bin/src/configure/lib.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/link.me $(ME_VAPP_PREFIX)/bin/src/configure/link.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/rc.me $(ME_VAPP_PREFIX)/bin/src/configure/rc.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/testme.me $(ME_VAPP_PREFIX)/bin/src/configure/testme.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/vxworks.me $(ME_VAPP_PREFIX)/bin/src/configure/vxworks.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/configure" ; \
	cp src/configure/winsdk.me $(ME_VAPP_PREFIX)/bin/src/configure/winsdk.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/configure.es $(ME_VAPP_PREFIX)/bin/src/configure.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/generate.es $(ME_VAPP_PREFIX)/bin/src/generate.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/master-main.me $(ME_VAPP_PREFIX)/bin/src/master-main.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/master-start.me $(ME_VAPP_PREFIX)/bin/src/master-start.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/me.es $(ME_VAPP_PREFIX)/bin/src/me.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/freebsd.me $(ME_VAPP_PREFIX)/bin/src/os/freebsd.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/gcc.me $(ME_VAPP_PREFIX)/bin/src/os/gcc.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/linux.me $(ME_VAPP_PREFIX)/bin/src/os/linux.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/macosx.me $(ME_VAPP_PREFIX)/bin/src/os/macosx.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/solaris.me $(ME_VAPP_PREFIX)/bin/src/os/solaris.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/unix.me $(ME_VAPP_PREFIX)/bin/src/os/unix.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/vxworks.me $(ME_VAPP_PREFIX)/bin/src/os/vxworks.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/os" ; \
	cp src/os/windows.me $(ME_VAPP_PREFIX)/bin/src/os/windows.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/simple.me $(ME_VAPP_PREFIX)/bin/src/simple.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/standard.me $(ME_VAPP_PREFIX)/bin/src/standard.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/ejs.testme.es $(ME_VAPP_PREFIX)/bin/src/tm/ejs.testme.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/libtestme.c $(ME_VAPP_PREFIX)/bin/src/tm/libtestme.c ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/sample.ct $(ME_VAPP_PREFIX)/bin/src/tm/sample.ct ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/testme.c $(ME_VAPP_PREFIX)/bin/src/tm/testme.c ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/testme.es $(ME_VAPP_PREFIX)/bin/src/tm/testme.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/testme.h $(ME_VAPP_PREFIX)/bin/src/tm/testme.h ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src/tm" ; \
	cp src/tm/testme.me $(ME_VAPP_PREFIX)/bin/src/tm/testme.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/vstudio.es $(ME_VAPP_PREFIX)/bin/src/vstudio.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/src" ; \
	cp src/xcode.es $(ME_VAPP_PREFIX)/bin/src/xcode.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1/doc/public/man" ; \
	cp doc/public/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/doc/public/man/me.1 ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1/doc/public/man" ; \
	cp doc/public/man/testme.1 $(ME_VAPP_PREFIX)/doc/man/man1/doc/public/man/testme.1 ; \
	)

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
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

#
#   version
#
version: $(DEPS_59)
	( \
	cd build/macosx-x64-debug/bin; \
	echo 0.8.4 ; \
	)

