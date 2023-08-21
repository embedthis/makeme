#
#   makeme-macosx-default.mk -- Makefile to build Embedthis MakeMe for macosx
#

NAME                  := makeme
VERSION               := 1.0.6
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= macosx
CC                    ?= clang
AR                    ?= ar
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

#
# Components
#
ME_COM_COMPILER       ?= 1
ME_COM_EJSCRIPT       ?= 1
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_MBEDTLS        ?= 1
ME_COM_MPR            ?= 1
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_ZLIB           ?= 1

ME_COM_OPENSSL_PATH   ?= "/path/to/openssl"

ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_EJSCRIPT),1)
    ME_COM_ZLIB := 1
endif

#
# Settings
#
ME_AUTHOR             ?= \"Embedthis Software\"
ME_CERTS_BITS         ?= 2048
ME_CERTS_DAYS         ?= 3650
ME_CERTS_GENDH        ?= 0
ME_COMPANY            ?= \"embedthis\"
ME_COMPATIBLE         ?= \"1.0\"
ME_COMPILER_HAS_ATOMIC ?= 1
ME_COMPILER_HAS_ATOMIC64 ?= 1
ME_COMPILER_HAS_DOUBLE_BRACES ?= 1
ME_COMPILER_HAS_DYN_LOAD ?= 1
ME_COMPILER_HAS_LIB_EDIT ?= 1
ME_COMPILER_HAS_LIB_RT ?= 0
ME_COMPILER_HAS_MMU   ?= 1
ME_COMPILER_HAS_MTUNE ?= 1
ME_COMPILER_HAS_PAM   ?= 1
ME_COMPILER_HAS_STACK_PROTECTOR ?= 1
ME_COMPILER_HAS_SYNC  ?= 1
ME_COMPILER_HAS_SYNC64 ?= 1
ME_COMPILER_HAS_SYNC_CAS ?= 1
ME_COMPILER_HAS_UNNAMED_UNIONS ?= 1
ME_COMPILER_WARN64TO32 ?= 1
ME_COMPILER_WARN_UNUSED ?= 1
ME_CONFIGURE          ?= \"me -d -q -platform macosx-x64-default -configure . -gen make\"
ME_CONFIGURED         ?= 1
ME_DEPTH              ?= 1
ME_DESCRIPTION        ?= \"Embedthis MakeMe\"
ME_EJS_ONE_MODULE     ?= 1
ME_EJSCRIPT_COMPILE   ?= \"--debug\"
ME_EJSCRIPT_DB        ?= 1
ME_EJSCRIPT_MAIL      ?= 1
ME_EJSCRIPT_MAPPER    ?= 1
ME_EJSCRIPT_SHELL     ?= 1
ME_EJSCRIPT_TAR       ?= 1
ME_EJSCRIPT_TEMPLATE  ?= 1
ME_EJSCRIPT_WEB       ?= 1
ME_EJSCRIPT_ZLIB      ?= 1
ME_HTTP_CMD           ?= 1
ME_HTTP_PAM           ?= 0
ME_INTEGRATE          ?= 1
ME_MANIFEST           ?= \"installs/manifest.me\"
ME_MBEDTLS_COMPACT    ?= 1
ME_MPR_LOGGING        ?= 1
ME_NAME               ?= \"makeme\"
ME_PARTS              ?= \"undefined\"
ME_PLATFORMS          ?= \"local\"
ME_PREFIXES           ?= \"install-prefixes\"
ME_TITLE              ?= \"Embedthis MakeMe\"
ME_TUNE               ?= \"speed\"
ME_VERSION            ?= \"1.0.6\"

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_EJSCRIPT=$(ME_COM_EJSCRIPT) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_ZLIB=$(ME_COM_ZLIB) -DME_CERTS_BITS=$(ME_CERTS_BITS) -DME_CERTS_DAYS=$(ME_CERTS_DAYS) -DME_CERTS_GENDH=$(ME_CERTS_GENDH) -DME_EJSCRIPT_COMPILE=$(ME_EJSCRIPT_COMPILE) -DME_EJSCRIPT_DB=$(ME_EJSCRIPT_DB) -DME_EJSCRIPT_MAIL=$(ME_EJSCRIPT_MAIL) -DME_EJSCRIPT_MAPPER=$(ME_EJSCRIPT_MAPPER) -DME_EJSCRIPT_SHELL=$(ME_EJSCRIPT_SHELL) -DME_EJSCRIPT_TAR=$(ME_EJSCRIPT_TAR) -DME_EJSCRIPT_TEMPLATE=$(ME_EJSCRIPT_TEMPLATE) -DME_EJSCRIPT_WEB=$(ME_EJSCRIPT_WEB) -DME_EJSCRIPT_ZLIB=$(ME_EJSCRIPT_ZLIB) -DME_HTTP_CMD=$(ME_HTTP_CMD) -DME_HTTP_PAM=$(ME_HTTP_PAM) -DME_MBEDTLS_COMPACT=$(ME_MBEDTLS_COMPACT) -DME_MPR_LOGGING=$(ME_MPR_LOGGING) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS              += -L$(BUILD)/bin
LIBS                  += -ldl -lpthread -lm

DEBUG                 ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG=1
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


ifeq ($(ME_COM_EJSCRIPT),1)
    TARGETS           += $(BUILD)/bin/ejs.mod
endif
TARGETS               += $(BUILD)/bin/ejs.testme.es
TARGETS               += $(BUILD)/bin/ejs.testme.mod
ifeq ($(ME_COM_EJSCRIPT),1)
    TARGETS           += $(BUILD)/bin/makeme-ejs
endif
TARGETS               += $(BUILD)/.extras-modified
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(BUILD)/bin/http
endif
TARGETS               += .install-certs-modified
TARGETS               += $(BUILD)/bin
TARGETS               += $(BUILD)/bin/me
TARGETS               += $(BUILD)/bin/testme
TARGETS               += $(BUILD)/bin/testme.es


DEPEND := $(strip $(wildcard ./projects/depend.mk))
ifneq ($(DEPEND),)
include $(DEPEND)
endif

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
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/makeme-macosx-default-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/makeme-macosx-default-me.h >/dev/null ; then\
		cp projects/makeme-macosx-default-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build" ; \
			echo "   [Warning] Previous build command: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo "$(MAKEFLAGS)" >$(BUILD)/.makeflags

clean:
	rm -f "$(BUILD)/obj/ejs.o"
	rm -f "$(BUILD)/obj/ejsLib.o"
	rm -f "$(BUILD)/obj/ejsc.o"
	rm -f "$(BUILD)/obj/http.o"
	rm -f "$(BUILD)/obj/httpLib.o"
	rm -f "$(BUILD)/obj/mbedtls.o"
	rm -f "$(BUILD)/obj/me.o"
	rm -f "$(BUILD)/obj/mpr-mbedtls.o"
	rm -f "$(BUILD)/obj/mpr-openssl.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/testme.o"
	rm -f "$(BUILD)/obj/zlib.o"
	rm -f "$(BUILD)/bin/ejs.testme.es"
	rm -f "$(BUILD)/bin/makeme-ejsc"
	rm -f "$(BUILD)/bin/makeme-ejs"
	rm -f "$(BUILD)/bin/http"
	rm -f ".install-certs-modified"
	rm -f "$(BUILD)/bin"
	rm -f "$(BUILD)/bin/libejs.dylib"
	rm -f "$(BUILD)/bin/libhttp.dylib"
	rm -f "$(BUILD)/bin/libmbedtls.a"
	rm -f "$(BUILD)/bin/libmpr.dylib"
	rm -f "$(BUILD)/bin/libmpr-mbedtls.a"
	rm -f "$(BUILD)/bin/libpcre.dylib"
	rm -f "$(BUILD)/bin/libzlib.dylib"
	rm -f "$(BUILD)/bin/testme"
	rm -f "$(BUILD)/bin/testme.es"

clobber: clean
	rm -fr ./$(BUILD)

#
#   config.h
#
DEPS_1 += src/mbedtls/config.h

$(BUILD)/inc/config.h: $(DEPS_1)
	@echo '      [Copy] $(BUILD)/inc/config.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/config.h $(BUILD)/inc/config.h

#
#   me.h
#

$(BUILD)/inc/me.h: $(DEPS_2)

#
#   osdep.h
#
DEPS_3 += src/osdep/osdep.h
DEPS_3 += $(BUILD)/inc/me.h

$(BUILD)/inc/osdep.h: $(DEPS_3)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_4 += src/mpr/mpr.h
DEPS_4 += $(BUILD)/inc/me.h
DEPS_4 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_4)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_5 += src/http/http.h
DEPS_5 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_5)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/http/http.h $(BUILD)/inc/http.h

#
#   ejs.slots.h
#

src/ejscript/ejs.slots.h: $(DEPS_6)

#
#   pcre.h
#
DEPS_7 += src/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_7)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   zlib.h
#
DEPS_8 += src/zlib/zlib.h
DEPS_8 += $(BUILD)/inc/me.h

$(BUILD)/inc/zlib.h: $(DEPS_8)
	@echo '      [Copy] $(BUILD)/inc/zlib.h'
	mkdir -p "$(BUILD)/inc"
	cp src/zlib/zlib.h $(BUILD)/inc/zlib.h

#
#   ejs.h
#
DEPS_9 += src/ejscript/ejs.h
DEPS_9 += $(BUILD)/inc/me.h
DEPS_9 += $(BUILD)/inc/osdep.h
DEPS_9 += $(BUILD)/inc/mpr.h
DEPS_9 += $(BUILD)/inc/http.h
DEPS_9 += src/ejscript/ejs.slots.h
DEPS_9 += $(BUILD)/inc/pcre.h
DEPS_9 += $(BUILD)/inc/zlib.h

$(BUILD)/inc/ejs.h: $(DEPS_9)
	@echo '      [Copy] $(BUILD)/inc/ejs.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejs.h $(BUILD)/inc/ejs.h

#
#   ejs.slots.h
#
DEPS_10 += src/ejscript/ejs.slots.h

$(BUILD)/inc/ejs.slots.h: $(DEPS_10)
	@echo '      [Copy] $(BUILD)/inc/ejs.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejs.slots.h $(BUILD)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
DEPS_11 += src/ejscript/ejsByteGoto.h

$(BUILD)/inc/ejsByteGoto.h: $(DEPS_11)
	@echo '      [Copy] $(BUILD)/inc/ejsByteGoto.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejsByteGoto.h $(BUILD)/inc/ejsByteGoto.h

#
#   embedtls.h
#
DEPS_12 += src/mbedtls/embedtls.h

$(BUILD)/inc/embedtls.h: $(DEPS_12)
	@echo '      [Copy] $(BUILD)/inc/embedtls.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/embedtls.h $(BUILD)/inc/embedtls.h

#
#   mbedtls-config.h
#
DEPS_13 += src/mbedtls/mbedtls-config.h

$(BUILD)/inc/mbedtls-config.h: $(DEPS_13)
	@echo '      [Copy] $(BUILD)/inc/mbedtls-config.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/mbedtls-config.h $(BUILD)/inc/mbedtls-config.h

#
#   mbedtls.h
#
DEPS_14 += src/mbedtls/mbedtls.h

$(BUILD)/inc/mbedtls.h: $(DEPS_14)
	@echo '      [Copy] $(BUILD)/inc/mbedtls.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/mbedtls.h $(BUILD)/inc/mbedtls.h

#
#   testme.h
#
DEPS_15 += src/tm/testme.h

$(BUILD)/inc/testme.h: $(DEPS_15)
	@echo '      [Copy] $(BUILD)/inc/testme.h'
	mkdir -p "$(BUILD)/inc"
	cp src/tm/testme.h $(BUILD)/inc/testme.h

#
#   ejs.h
#

src/ejscript/ejs.h: $(DEPS_16)

#
#   ejs.o
#
DEPS_17 += src/ejscript/ejs.h

$(BUILD)/obj/ejs.o: \
    src/ejscript/ejs.c $(DEPS_17)
	@echo '   [Compile] $(BUILD)/obj/ejs.o'
	$(CC) -c -o $(BUILD)/obj/ejs.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/ejscript/ejs.c

#
#   ejsLib.o
#
DEPS_18 += src/ejscript/ejs.h
DEPS_18 += $(BUILD)/inc/mpr.h
DEPS_18 += $(BUILD)/inc/pcre.h
DEPS_18 += $(BUILD)/inc/me.h

$(BUILD)/obj/ejsLib.o: \
    src/ejscript/ejsLib.c $(DEPS_18)
	@echo '   [Compile] $(BUILD)/obj/ejsLib.o'
	$(CC) -c -o $(BUILD)/obj/ejsLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/ejscript/ejsLib.c

#
#   ejsc.o
#
DEPS_19 += src/ejscript/ejs.h

$(BUILD)/obj/ejsc.o: \
    src/ejscript/ejsc.c $(DEPS_19)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c -o $(BUILD)/obj/ejsc.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/ejscript/ejsc.c

#
#   http.h
#

src/http/http.h: $(DEPS_20)

#
#   http.o
#
DEPS_21 += src/http/http.h

$(BUILD)/obj/http.o: \
    src/http/http.c $(DEPS_21)
	@echo '   [Compile] $(BUILD)/obj/http.o'
	$(CC) -c -o $(BUILD)/obj/http.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/http/http.c

#
#   httpLib.o
#
DEPS_22 += src/http/http.h
DEPS_22 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/httpLib.o: \
    src/http/httpLib.c $(DEPS_22)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/http/httpLib.c

#
#   mbedtls.h
#

src/mbedtls/mbedtls.h: $(DEPS_23)

#
#   mbedtls.o
#
DEPS_24 += src/mbedtls/mbedtls.h

$(BUILD)/obj/mbedtls.o: \
    src/mbedtls/mbedtls.c $(DEPS_24)
	@echo '   [Compile] $(BUILD)/obj/mbedtls.o'
	$(CC) -c -o $(BUILD)/obj/mbedtls.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/mbedtls/mbedtls.c

#
#   me.o
#
DEPS_25 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/me.o: \
    src/me.c $(DEPS_25)
	@echo '   [Compile] $(BUILD)/obj/me.o'
	$(CC) -c -o $(BUILD)/obj/me.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/me.c

#
#   mpr-mbedtls.o
#
DEPS_26 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/mpr-mbedtls.o: \
    src/mpr-mbedtls/mpr-mbedtls.c $(DEPS_26)
	@echo '   [Compile] $(BUILD)/obj/mpr-mbedtls.o'
	$(CC) -c -o $(BUILD)/obj/mpr-mbedtls.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/mpr-mbedtls/mpr-mbedtls.c

#
#   mpr-openssl.o
#
DEPS_27 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/mpr-openssl.o: \
    src/mpr-openssl/mpr-openssl.c $(DEPS_27)
	@echo '   [Compile] $(BUILD)/obj/mpr-openssl.o'
	$(CC) -c -o $(BUILD)/obj/mpr-openssl.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(BUILD)/inc" "-I$(ME_COM_OPENSSL_PATH)/include" src/mpr-openssl/mpr-openssl.c

#
#   mpr.h
#

src/mpr/mpr.h: $(DEPS_28)

#
#   mprLib.o
#
DEPS_29 += src/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/mpr/mprLib.c $(DEPS_29)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c -o $(BUILD)/obj/mprLib.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/mpr/mprLib.c

#
#   pcre.h
#

src/pcre/pcre.h: $(DEPS_30)

#
#   pcre.o
#
DEPS_31 += $(BUILD)/inc/me.h
DEPS_31 += src/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/pcre/pcre.c $(DEPS_31)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c -o $(BUILD)/obj/pcre.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) $(IFLAGS) src/pcre/pcre.c

#
#   testme.o
#
DEPS_32 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/testme.o: \
    src/tm/testme.c $(DEPS_32)
	@echo '   [Compile] $(BUILD)/obj/testme.o'
	$(CC) -c -o $(BUILD)/obj/testme.o -arch $(CC_ARCH) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/tm/testme.c

#
#   zlib.h
#

src/zlib/zlib.h: $(DEPS_33)

#
#   zlib.o
#
DEPS_34 += $(BUILD)/inc/me.h
DEPS_34 += src/zlib/zlib.h

$(BUILD)/obj/zlib.o: \
    src/zlib/zlib.c $(DEPS_34)
	@echo '   [Compile] $(BUILD)/obj/zlib.o'
	$(CC) -c -o $(BUILD)/obj/zlib.o -arch $(CC_ARCH) $(CFLAGS) -Wno-deprecated-non-prototype $(DFLAGS) $(IFLAGS) src/zlib/zlib.c

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmbedtls
#
DEPS_35 += $(BUILD)/inc/osdep.h
DEPS_35 += $(BUILD)/inc/config.h
DEPS_35 += $(BUILD)/inc/embedtls.h
DEPS_35 += $(BUILD)/inc/mbedtls-config.h
DEPS_35 += $(BUILD)/inc/mbedtls.h
DEPS_35 += $(BUILD)/obj/mbedtls.o

$(BUILD)/bin/libmbedtls.a: $(DEPS_35)
	@echo '      [Link] $(BUILD)/bin/libmbedtls.a'
	$(AR) -cr $(BUILD)/bin/libmbedtls.a "$(BUILD)/obj/mbedtls.o"
endif

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmpr-mbedtls
#
DEPS_36 += $(BUILD)/bin/libmbedtls.a
DEPS_36 += $(BUILD)/obj/mpr-mbedtls.o

$(BUILD)/bin/libmpr-mbedtls.a: $(DEPS_36)
	@echo '      [Link] $(BUILD)/bin/libmpr-mbedtls.a'
	$(AR) -cr $(BUILD)/bin/libmpr-mbedtls.a "$(BUILD)/obj/mpr-mbedtls.o"
endif

ifeq ($(ME_COM_OPENSSL),1)
#
#   libmpr-openssl
#
DEPS_37 += $(BUILD)/obj/mpr-openssl.o

$(BUILD)/bin/libmpr-openssl.a: $(DEPS_37)
	@echo '      [Link] $(BUILD)/bin/libmpr-openssl.a'
	$(AR) -cr $(BUILD)/bin/libmpr-openssl.a "$(BUILD)/obj/mpr-openssl.o"
endif

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_38 += $(BUILD)/inc/zlib.h
DEPS_38 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.dylib: $(DEPS_38)
	@echo '      [Link] $(BUILD)/bin/libzlib.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libzlib.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libzlib.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

#
#   libmpr
#
DEPS_39 += $(BUILD)/inc/osdep.h
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_39 += $(BUILD)/bin/libmpr-mbedtls.a
endif
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_39 += $(BUILD)/bin/libmbedtls.a
endif
ifeq ($(ME_COM_OPENSSL),1)
    DEPS_39 += $(BUILD)/bin/libmpr-openssl.a
endif
ifeq ($(ME_COM_ZLIB),1)
    DEPS_39 += $(BUILD)/bin/libzlib.dylib
endif
DEPS_39 += $(BUILD)/inc/mpr.h
DEPS_39 += $(BUILD)/obj/mprLib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_39 += -lssl
    LIBPATHS_39 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lcrypto
    LIBPATHS_39 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_39 += -lzlib
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_39 += -lzlib
endif

$(BUILD)/bin/libmpr.dylib: $(DEPS_39)
	@echo '      [Link] $(BUILD)/bin/libmpr.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libmpr.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  -install_name @rpath/libmpr.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/mprLib.o" $(LIBPATHS_39) $(LIBS_39) $(LIBS_39) $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_40 += $(BUILD)/inc/pcre.h
DEPS_40 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.dylib: $(DEPS_40)
	@echo '      [Link] $(BUILD)/bin/libpcre.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libpcre.dylib -arch $(CC_ARCH) $(LDFLAGS) -compatibility_version 1.0 -current_version 1.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_41 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_PCRE),1)
    DEPS_41 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_41 += $(BUILD)/inc/http.h
DEPS_41 += $(BUILD)/obj/httpLib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_41 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_41 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_41 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_41 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_41 += -lssl
    LIBPATHS_41 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_41 += -lcrypto
    LIBPATHS_41 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_41 += -lzlib
endif
LIBS_41 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_41 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_41 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_41 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_41 += -lpcre
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_41 += -lpcre
endif
LIBS_41 += -lmpr

$(BUILD)/bin/libhttp.dylib: $(DEPS_41)
	@echo '      [Link] $(BUILD)/bin/libhttp.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libhttp.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  -install_name @rpath/libhttp.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/httpLib.o" $(LIBPATHS_41) $(LIBS_41) $(LIBS_41) $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   libejs
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_42 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_PCRE),1)
    DEPS_42 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_42 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_ZLIB),1)
    DEPS_42 += $(BUILD)/bin/libzlib.dylib
endif
DEPS_42 += $(BUILD)/inc/ejs.h
DEPS_42 += $(BUILD)/inc/ejs.slots.h
DEPS_42 += $(BUILD)/inc/ejsByteGoto.h
DEPS_42 += $(BUILD)/obj/ejsLib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_42 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_42 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_42 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_42 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_42 += -lssl
    LIBPATHS_42 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_42 += -lcrypto
    LIBPATHS_42 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_42 += -lzlib
endif
LIBS_42 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_42 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_42 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_42 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_42 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_42 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_42 += -lpcre
endif
LIBS_42 += -lmpr
ifeq ($(ME_COM_ZLIB),1)
    LIBS_42 += -lzlib
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_42 += -lhttp
endif

$(BUILD)/bin/libejs.dylib: $(DEPS_42)
	@echo '      [Link] $(BUILD)/bin/libejs.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libejs.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  -install_name @rpath/libejs.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/ejsLib.o" $(LIBPATHS_42) $(LIBS_42) $(LIBS_42) $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejsc
#
DEPS_43 += $(BUILD)/bin/libejs.dylib
DEPS_43 += $(BUILD)/obj/ejsc.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_43 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_43 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_43 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_43 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_43 += -lssl
    LIBPATHS_43 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_43 += -lcrypto
    LIBPATHS_43 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_43 += -lzlib
endif
LIBS_43 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_43 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_43 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_43 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_43 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_43 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_43 += -lpcre
endif
LIBS_43 += -lmpr
LIBS_43 += -lejs
ifeq ($(ME_COM_ZLIB),1)
    LIBS_43 += -lzlib
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_43 += -lhttp
endif

$(BUILD)/bin/makeme-ejsc: $(DEPS_43)
	@echo '      [Link] $(BUILD)/bin/makeme-ejsc'
	$(CC) -o $(BUILD)/bin/makeme-ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/ejsc.o" $(LIBPATHS_43) $(LIBS_43) $(LIBS_43) $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejs.mod
#
DEPS_44 += src/ejscript/ejs.es
DEPS_44 += $(BUILD)/bin/makeme-ejsc

$(BUILD)/bin/ejs.mod: $(DEPS_44)
	( \
	cd src/ejscript; \
	echo '   [Compile] ejs.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --out "../../$(BUILD)/bin/ejs.mod" --debug --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.es
#
DEPS_45 += src/tm/ejs.testme.es

$(BUILD)/bin/ejs.testme.es: $(DEPS_45)
	@echo '      [Copy] $(BUILD)/bin/ejs.testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/ejs.testme.es $(BUILD)/bin/ejs.testme.es

#
#   ejs.testme.mod
#
DEPS_46 += src/tm/ejs.testme.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_46 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/ejs.testme.mod: $(DEPS_46)
	( \
	cd src/tm; \
	echo '   [Compile] ejs.testme.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --debug --out "../../$(BUILD)/bin/ejs.testme.mod" --optimize 9 ejs.testme.es ; \
	)

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejscmd
#
DEPS_47 += $(BUILD)/bin/libejs.dylib
DEPS_47 += $(BUILD)/obj/ejs.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_47 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_47 += -lssl
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_47 += -lcrypto
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_47 += -lzlib
endif
LIBS_47 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_47 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_47 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_47 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_47 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_47 += -lpcre
endif
LIBS_47 += -lmpr
LIBS_47 += -lejs
ifeq ($(ME_COM_ZLIB),1)
    LIBS_47 += -lzlib
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_47 += -lhttp
endif

$(BUILD)/bin/makeme-ejs: $(DEPS_47)
	@echo '      [Link] $(BUILD)/bin/makeme-ejs'
	$(CC) -o $(BUILD)/bin/makeme-ejs -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/ejs.o" $(LIBPATHS_47) $(LIBS_47) $(LIBS_47) $(LIBS) -ledit 
endif

#
#   extras
#
DEPS_48 += src/Configure.es
DEPS_48 += src/Generate.es
DEPS_48 += src/vcvars.bat

$(BUILD)/.extras-modified: $(DEPS_48)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/Configure.es $(BUILD)/bin/Configure.es
	cp src/Generate.es $(BUILD)/bin/Generate.es
	cp src/vcvars.bat $(BUILD)/bin/vcvars.bat
	touch "$(BUILD)/.extras-modified"

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_49 += $(BUILD)/bin/libhttp.dylib
DEPS_49 += $(BUILD)/obj/http.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_49 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_49 += -lssl
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_49 += -lcrypto
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_49 += -lzlib
endif
LIBS_49 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_49 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_49 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_49 += -lpcre
endif
LIBS_49 += -lhttp
ifeq ($(ME_COM_PCRE),1)
    LIBS_49 += -lpcre
endif
LIBS_49 += -lmpr

$(BUILD)/bin/http: $(DEPS_49)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/http.o" $(LIBPATHS_49) $(LIBS_49) $(LIBS_49) $(LIBS) 
endif

#
#   install-certs
#
DEPS_50 += src/certs/samples/ca.crt
DEPS_50 += src/certs/samples/ca.key
DEPS_50 += src/certs/samples/ec.crt
DEPS_50 += src/certs/samples/ec.key
DEPS_50 += src/certs/samples/roots.crt
DEPS_50 += src/certs/samples/self.crt
DEPS_50 += src/certs/samples/self.key
DEPS_50 += src/certs/samples/test.crt
DEPS_50 += src/certs/samples/test.key

.install-certs-modified: $(DEPS_50)
	@echo '      [Copy] certs'
	mkdir -p "certs"
	cp src/certs/samples/ca.crt certs/ca.crt
	cp src/certs/samples/ca.key certs/ca.key
	cp src/certs/samples/ec.crt certs/ec.crt
	cp src/certs/samples/ec.key certs/ec.key
	cp src/certs/samples/roots.crt certs/roots.crt
	cp src/certs/samples/self.crt certs/self.crt
	cp src/certs/samples/self.key certs/self.key
	cp src/certs/samples/test.crt certs/test.crt
	cp src/certs/samples/test.key certs/test.key
	touch ".install-certs-modified"

#
#   install-roots
#
DEPS_51 += src/certs/samples/roots.crt

$(BUILD)/bin: $(DEPS_51)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/certs/samples/roots.crt $(BUILD)/bin/roots.crt

#
#   me.mod
#
DEPS_52 += paks/ejs.version/Version.es
DEPS_52 += src/Builder.es
DEPS_52 += src/Loader.es
DEPS_52 += src/MakeMe.es
DEPS_52 += src/Me.es
DEPS_52 += src/Script.es
DEPS_52 += src/Target.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_52 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/me.mod: $(DEPS_52)
	echo '   [Compile] me.mod' ; \
	"./$(BUILD)/bin/makeme-ejsc" --debug --out "./$(BUILD)/bin/me.mod" --optimize 9 paks/ejs.version/Version.es src/Builder.es src/Loader.es src/MakeMe.es src/Me.es src/Script.es src/Target.es

#
#   pakrun
#
DEPS_53 += paks/me-components/appweb.me
DEPS_53 += paks/me-components/compiler.me
DEPS_53 += paks/me-components/components.me
DEPS_53 += paks/me-components/ejscript.me
DEPS_53 += paks/me-components/lib.me
DEPS_53 += paks/me-components/link.me
DEPS_53 += paks/me-components/pak.json
DEPS_53 += paks/me-components/rc.me
DEPS_53 += paks/me-components/testme.me
DEPS_53 += paks/me-installs/Installs.es
DEPS_53 += paks/me-installs/installs.me
DEPS_53 += paks/me-installs/manifest.me
DEPS_53 += paks/me-installs/pak.json
DEPS_53 += paks/me-make/Make.es
DEPS_53 += paks/me-make/make.me
DEPS_53 += paks/me-make/pak.json
DEPS_53 += paks/me-os/freebsd.me
DEPS_53 += paks/me-os/gcc.me
DEPS_53 += paks/me-os/linux.me
DEPS_53 += paks/me-os/macosx.me
DEPS_53 += paks/me-os/os.me
DEPS_53 += paks/me-os/pak.json
DEPS_53 += paks/me-os/solaris.me
DEPS_53 += paks/me-os/unix.me
DEPS_53 += paks/me-os/vxworks.me
DEPS_53 += paks/me-os/windows.me
DEPS_53 += paks/me-vstudio/Vstudio.es
DEPS_53 += paks/me-vstudio/pak.json
DEPS_53 += paks/me-vstudio/vstudio.me
DEPS_53 += paks/me-win/make.bat
DEPS_53 += paks/me-win/pak.json
DEPS_53 += paks/me-win/win.me
DEPS_53 += paks/me-win/windows.bat
DEPS_53 += paks/me-xcode/Xcode.es
DEPS_53 += paks/me-xcode/pak.json
DEPS_53 += paks/me-xcode/xcode.me

$(BUILD)/.pakrun-modified: $(DEPS_53)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin/paks/me-components"
	cp paks/me-components/appweb.me $(BUILD)/bin/paks/me-components/appweb.me
	cp paks/me-components/compiler.me $(BUILD)/bin/paks/me-components/compiler.me
	cp paks/me-components/components.me $(BUILD)/bin/paks/me-components/components.me
	cp paks/me-components/ejscript.me $(BUILD)/bin/paks/me-components/ejscript.me
	cp paks/me-components/lib.me $(BUILD)/bin/paks/me-components/lib.me
	cp paks/me-components/link.me $(BUILD)/bin/paks/me-components/link.me
	cp paks/me-components/pak.json $(BUILD)/bin/paks/me-components/pak.json
	cp paks/me-components/rc.me $(BUILD)/bin/paks/me-components/rc.me
	cp paks/me-components/testme.me $(BUILD)/bin/paks/me-components/testme.me
	mkdir -p "$(BUILD)/bin/paks/me-installs"
	cp paks/me-installs/Installs.es $(BUILD)/bin/paks/me-installs/Installs.es
	cp paks/me-installs/installs.me $(BUILD)/bin/paks/me-installs/installs.me
	cp paks/me-installs/manifest.me $(BUILD)/bin/paks/me-installs/manifest.me
	cp paks/me-installs/pak.json $(BUILD)/bin/paks/me-installs/pak.json
	mkdir -p "$(BUILD)/bin/paks/me-make"
	cp paks/me-make/Make.es $(BUILD)/bin/paks/me-make/Make.es
	cp paks/me-make/make.me $(BUILD)/bin/paks/me-make/make.me
	cp paks/me-make/pak.json $(BUILD)/bin/paks/me-make/pak.json
	mkdir -p "$(BUILD)/bin/paks/me-os"
	cp paks/me-os/freebsd.me $(BUILD)/bin/paks/me-os/freebsd.me
	cp paks/me-os/gcc.me $(BUILD)/bin/paks/me-os/gcc.me
	cp paks/me-os/linux.me $(BUILD)/bin/paks/me-os/linux.me
	cp paks/me-os/macosx.me $(BUILD)/bin/paks/me-os/macosx.me
	cp paks/me-os/os.me $(BUILD)/bin/paks/me-os/os.me
	cp paks/me-os/pak.json $(BUILD)/bin/paks/me-os/pak.json
	cp paks/me-os/solaris.me $(BUILD)/bin/paks/me-os/solaris.me
	cp paks/me-os/unix.me $(BUILD)/bin/paks/me-os/unix.me
	cp paks/me-os/vxworks.me $(BUILD)/bin/paks/me-os/vxworks.me
	cp paks/me-os/windows.me $(BUILD)/bin/paks/me-os/windows.me
	mkdir -p "$(BUILD)/bin/paks/me-vstudio"
	cp paks/me-vstudio/Vstudio.es $(BUILD)/bin/paks/me-vstudio/Vstudio.es
	cp paks/me-vstudio/pak.json $(BUILD)/bin/paks/me-vstudio/pak.json
	cp paks/me-vstudio/vstudio.me $(BUILD)/bin/paks/me-vstudio/vstudio.me
	mkdir -p "$(BUILD)/bin/paks/me-win"
	cp paks/me-win/make.bat $(BUILD)/bin/paks/me-win/make.bat
	cp paks/me-win/pak.json $(BUILD)/bin/paks/me-win/pak.json
	cp paks/me-win/win.me $(BUILD)/bin/paks/me-win/win.me
	cp paks/me-win/windows.bat $(BUILD)/bin/paks/me-win/windows.bat
	mkdir -p "$(BUILD)/bin/paks/me-xcode"
	cp paks/me-xcode/Xcode.es $(BUILD)/bin/paks/me-xcode/Xcode.es
	cp paks/me-xcode/pak.json $(BUILD)/bin/paks/me-xcode/pak.json
	cp paks/me-xcode/xcode.me $(BUILD)/bin/paks/me-xcode/xcode.me
	touch "$(BUILD)/.pakrun-modified"

#
#   runtime
#
DEPS_54 += src/master-main.me
DEPS_54 += src/master-start.me
DEPS_54 += src/simple.me
DEPS_54 += src/standard.me
DEPS_54 += $(BUILD)/.pakrun-modified

$(BUILD)/.runtime-modified: $(DEPS_54)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/master-main.me $(BUILD)/bin/master-main.me
	cp src/master-start.me $(BUILD)/bin/master-start.me
	cp src/simple.me $(BUILD)/bin/simple.me
	cp src/standard.me $(BUILD)/bin/standard.me
	touch "$(BUILD)/.runtime-modified"

#
#   me
#
DEPS_55 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_HTTP),1)
    DEPS_55 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_55 += $(BUILD)/bin/libejs.dylib
endif
DEPS_55 += $(BUILD)/bin/me.mod
DEPS_55 += $(BUILD)/.runtime-modified
DEPS_55 += $(BUILD)/obj/me.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_55 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_55 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_55 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_55 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_55 += -lssl
    LIBPATHS_55 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_55 += -lcrypto
    LIBPATHS_55 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_55 += -lzlib
endif
LIBS_55 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_55 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_55 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_55 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_55 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_55 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_55 += -lpcre
endif
LIBS_55 += -lmpr
ifeq ($(ME_COM_EJSCRIPT),1)
    LIBS_55 += -lejs
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_55 += -lzlib
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_55 += -lhttp
endif

$(BUILD)/bin/me: $(DEPS_55)
	@echo '      [Link] $(BUILD)/bin/me'
	$(CC) -o $(BUILD)/bin/me -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/me.o" $(LIBPATHS_55) $(LIBS_55) $(LIBS_55) $(LIBS) 

#
#   testme.mod
#
DEPS_56 += src/tm/testme.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_56 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/testme.mod: $(DEPS_56)
	( \
	cd src/tm; \
	echo '   [Compile] testme.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --debug --out "../../$(BUILD)/bin/testme.mod" --optimize 9 testme.es ; \
	)

#
#   testme
#
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_57 += $(BUILD)/bin/libejs.dylib
endif
DEPS_57 += $(BUILD)/bin/testme.mod
DEPS_57 += $(BUILD)/bin/ejs.testme.mod
DEPS_57 += $(BUILD)/inc/testme.h
DEPS_57 += $(BUILD)/obj/testme.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_57 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_57 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_57 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_57 += -lmpr-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_57 += -lssl
    LIBPATHS_57 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_57 += -lcrypto
    LIBPATHS_57 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_57 += -lzlib
endif
LIBS_57 += -lmpr
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_57 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_57 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_57 += -lzlib
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_57 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_57 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_57 += -lpcre
endif
LIBS_57 += -lmpr
ifeq ($(ME_COM_EJSCRIPT),1)
    LIBS_57 += -lejs
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_57 += -lzlib
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_57 += -lhttp
endif

$(BUILD)/bin/testme: $(DEPS_57)
	@echo '      [Link] $(BUILD)/bin/testme'
	$(CC) -o $(BUILD)/bin/testme -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)  "$(BUILD)/obj/testme.o" $(LIBPATHS_57) $(LIBS_57) $(LIBS_57) $(LIBS) 

#
#   testme.es
#
DEPS_58 += src/tm/testme.es

$(BUILD)/bin/testme.es: $(DEPS_58)
	@echo '      [Copy] $(BUILD)/bin/testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/testme.es $(BUILD)/bin/testme.es

#
#   installPrep
#

installPrep: $(DEPS_59)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with sudo." ; \
	exit 255 ; \
	fi

#
#   stop
#

stop: $(DEPS_60)

#
#   installBinary
#

installBinary: $(DEPS_61)
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "$(VERSION)" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	chmod 755 "$(ME_MAN_PREFIX)/man1" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/me $(ME_VAPP_PREFIX)/bin/me ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/me" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/me" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/me" "$(ME_BIN_PREFIX)/me" ; \
	cp $(BUILD)/bin/testme $(ME_VAPP_PREFIX)/bin/testme ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/testme" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/testme" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/testme" "$(ME_BIN_PREFIX)/testme" ; \
	cp $(BUILD)/bin/makeme-ejs $(ME_VAPP_PREFIX)/bin/makeme-ejs ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/makeme-ejs" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/makeme-ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/makeme-ejs" "$(ME_BIN_PREFIX)/makeme-ejs" ; \
	cp $(BUILD)/bin/makeme-ejsc $(ME_VAPP_PREFIX)/bin/makeme-ejsc ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/makeme-ejsc" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/makeme-ejsc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/makeme-ejsc" "$(ME_BIN_PREFIX)/makeme-ejsc" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libejs.dylib $(ME_VAPP_PREFIX)/bin/libejs.dylib ; \
	cp $(BUILD)/bin/libhttp.dylib $(ME_VAPP_PREFIX)/bin/libhttp.dylib ; \
	cp $(BUILD)/bin/libmpr.dylib $(ME_VAPP_PREFIX)/bin/libmpr.dylib ; \
	cp $(BUILD)/bin/libpcre.dylib $(ME_VAPP_PREFIX)/bin/libpcre.dylib ; \
	cp $(BUILD)/bin/libzlib.dylib $(ME_VAPP_PREFIX)/bin/libzlib.dylib ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/roots.crt $(ME_VAPP_PREFIX)/bin/roots.crt ; \
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
	cp src/Configure.es $(ME_VAPP_PREFIX)/bin/Configure.es ; \
	cp src/Generate.es $(ME_VAPP_PREFIX)/bin/Generate.es ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-components" ; \
	cp paks/me-components/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-components/LICENSE.md ; \
	cp paks/me-components/README.md $(ME_VAPP_PREFIX)/bin/paks/me-components/README.md ; \
	cp paks/me-components/appweb.me $(ME_VAPP_PREFIX)/bin/paks/me-components/appweb.me ; \
	cp paks/me-components/compiler.me $(ME_VAPP_PREFIX)/bin/paks/me-components/compiler.me ; \
	cp paks/me-components/components.me $(ME_VAPP_PREFIX)/bin/paks/me-components/components.me ; \
	cp paks/me-components/ejscript.me $(ME_VAPP_PREFIX)/bin/paks/me-components/ejscript.me ; \
	cp paks/me-components/lib.me $(ME_VAPP_PREFIX)/bin/paks/me-components/lib.me ; \
	cp paks/me-components/link.me $(ME_VAPP_PREFIX)/bin/paks/me-components/link.me ; \
	cp paks/me-components/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-components/pak.json ; \
	cp paks/me-components/rc.me $(ME_VAPP_PREFIX)/bin/paks/me-components/rc.me ; \
	cp paks/me-components/testme.me $(ME_VAPP_PREFIX)/bin/paks/me-components/testme.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-installs" ; \
	cp paks/me-installs/Installs.es $(ME_VAPP_PREFIX)/bin/paks/me-installs/Installs.es ; \
	cp paks/me-installs/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-installs/LICENSE.md ; \
	cp paks/me-installs/README.md $(ME_VAPP_PREFIX)/bin/paks/me-installs/README.md ; \
	cp paks/me-installs/installs.me $(ME_VAPP_PREFIX)/bin/paks/me-installs/installs.me ; \
	cp paks/me-installs/manifest.me $(ME_VAPP_PREFIX)/bin/paks/me-installs/manifest.me ; \
	cp paks/me-installs/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-installs/pak.json ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-make" ; \
	cp paks/me-make/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-make/LICENSE.md ; \
	cp paks/me-make/Make.es $(ME_VAPP_PREFIX)/bin/paks/me-make/Make.es ; \
	cp paks/me-make/README.md $(ME_VAPP_PREFIX)/bin/paks/me-make/README.md ; \
	cp paks/me-make/make.me $(ME_VAPP_PREFIX)/bin/paks/me-make/make.me ; \
	cp paks/me-make/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-make/pak.json ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-os" ; \
	cp paks/me-os/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-os/LICENSE.md ; \
	cp paks/me-os/README.md $(ME_VAPP_PREFIX)/bin/paks/me-os/README.md ; \
	cp paks/me-os/freebsd.me $(ME_VAPP_PREFIX)/bin/paks/me-os/freebsd.me ; \
	cp paks/me-os/gcc.me $(ME_VAPP_PREFIX)/bin/paks/me-os/gcc.me ; \
	cp paks/me-os/linux.me $(ME_VAPP_PREFIX)/bin/paks/me-os/linux.me ; \
	cp paks/me-os/macosx.me $(ME_VAPP_PREFIX)/bin/paks/me-os/macosx.me ; \
	cp paks/me-os/os.me $(ME_VAPP_PREFIX)/bin/paks/me-os/os.me ; \
	cp paks/me-os/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-os/pak.json ; \
	cp paks/me-os/solaris.me $(ME_VAPP_PREFIX)/bin/paks/me-os/solaris.me ; \
	cp paks/me-os/unix.me $(ME_VAPP_PREFIX)/bin/paks/me-os/unix.me ; \
	cp paks/me-os/vxworks.me $(ME_VAPP_PREFIX)/bin/paks/me-os/vxworks.me ; \
	cp paks/me-os/windows.me $(ME_VAPP_PREFIX)/bin/paks/me-os/windows.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-vstudio" ; \
	cp paks/me-vstudio/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/LICENSE.md ; \
	cp paks/me-vstudio/README.md $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/README.md ; \
	cp paks/me-vstudio/Vstudio.es $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/Vstudio.es ; \
	cp paks/me-vstudio/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/pak.json ; \
	cp paks/me-vstudio/vstudio.me $(ME_VAPP_PREFIX)/bin/paks/me-vstudio/vstudio.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/paks/me-xcode" ; \
	cp paks/me-xcode/LICENSE.md $(ME_VAPP_PREFIX)/bin/paks/me-xcode/LICENSE.md ; \
	cp paks/me-xcode/README.md $(ME_VAPP_PREFIX)/bin/paks/me-xcode/README.md ; \
	cp paks/me-xcode/Xcode.es $(ME_VAPP_PREFIX)/bin/paks/me-xcode/Xcode.es ; \
	cp paks/me-xcode/pak.json $(ME_VAPP_PREFIX)/bin/paks/me-xcode/pak.json ; \
	cp paks/me-xcode/xcode.me $(ME_VAPP_PREFIX)/bin/paks/me-xcode/xcode.me ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/dist/man/me.1 $(ME_VAPP_PREFIX)/doc/man/man1/me.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/me.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/me.1" "$(ME_MAN_PREFIX)/man1/me.1" ; \
	cp doc/dist/man/testme.1 $(ME_VAPP_PREFIX)/doc/man/man1/testme.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/testme.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/testme.1" "$(ME_MAN_PREFIX)/man1/testme.1"

#
#   start
#

start: $(DEPS_62)

#
#   install
#
DEPS_63 += installPrep
DEPS_63 += stop
DEPS_63 += installBinary
DEPS_63 += start

install: $(DEPS_63)

#
#   uninstall
#
DEPS_64 += stop

uninstall: $(DEPS_64)

#
#   uninstallBinary
#

uninstallBinary: $(DEPS_65)
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true

#
#   version
#

version: $(DEPS_66)
	echo $(VERSION)


EXTRA_MAKEFILE := $(strip $(wildcard ./projects/extra.mk))
ifneq ($(EXTRA_MAKEFILE),)
include $(EXTRA_MAKEFILE)
endif
