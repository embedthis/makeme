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
BUILD                 ?= build/$(OS)-$(ARCH)-$(PROFILE)
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

#
# Components
#
ME_COM_COMPILER       ?= 1
ME_COM_EJSCRIPT       ?= 1
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_MBEDTLS        ?= 0
ME_COM_MPR            ?= 1
ME_COM_OPENSSL        ?= 1
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_ZLIB           ?= 1

ME_COM_OPENSSL_PATH   ?= "/opt/homebrew"
ME_COM_MBEDTLS_PATH   ?= "/opt/homebrew"

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
ME_COMPILER_NOEXECSTACK ?= 0
ME_COMPILER_WARN64TO32 ?= 1
ME_COMPILER_WARN_UNUSED ?= 1
ME_CONFIGURE          ?= \"me -d -q -platform macosx-x64-default -configure . -gen make\"
ME_CONFIGURED         ?= 1
ME_DEBUG              ?= 1
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

CFLAGS                += -Wno-unknown-warning-option  -fPIC  -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_EJSCRIPT=$(ME_COM_EJSCRIPT) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_ZLIB=$(ME_COM_ZLIB) -DME_EJSCRIPT_COMPILE=$(ME_EJSCRIPT_COMPILE) -DME_EJSCRIPT_DB=$(ME_EJSCRIPT_DB) -DME_EJSCRIPT_MAIL=$(ME_EJSCRIPT_MAIL) -DME_EJSCRIPT_MAPPER=$(ME_EJSCRIPT_MAPPER) -DME_EJSCRIPT_SHELL=$(ME_EJSCRIPT_SHELL) -DME_EJSCRIPT_TAR=$(ME_EJSCRIPT_TAR) -DME_EJSCRIPT_TEMPLATE=$(ME_EJSCRIPT_TEMPLATE) -DME_EJSCRIPT_WEB=$(ME_EJSCRIPT_WEB) -DME_EJSCRIPT_ZLIB=$(ME_EJSCRIPT_ZLIB) -DME_HTTP_CMD=$(ME_HTTP_CMD) -DME_HTTP_PAM=$(ME_HTTP_PAM) -DME_MBEDTLS_COMPACT=$(ME_MBEDTLS_COMPACT) -DME_MPR_LOGGING=$(ME_MPR_LOGGING) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += -g -Wl,-no_warn_duplicate_libraries -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/
LIBPATHS              += -L$(BUILD)/bin
LIBS                  += -ldl -lpthread -lm

OPTIMIZE              ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG=1
LDFLAGS-debug         ?= -g
DFLAGS-release        ?= 
CFLAGS-release        ?= -O2
LDFLAGS-release       ?= 
CFLAGS                += $(CFLAGS-$(OPTIMIZE))
DFLAGS                += $(DFLAGS-$(OPTIMIZE))
LDFLAGS               += $(LDFLAGS-$(OPTIMIZE))

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
ME_VLIB_PREFIX        ?= $(ME_ROOT_PREFIX)/var/lib/$(NAME)
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
TARGETS               += $(BUILD)/.install-roots-modified
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
	@if [ "$(BUILD)" = "" ] ; then echo WARNING: BUILD not set ; exit 255 ; fi
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(BUILD)/bin ] && mkdir -p $(BUILD)/bin; true
	@[ ! -x $(BUILD)/inc ] && mkdir -p $(BUILD)/inc; true
	@[ ! -x $(BUILD)/obj ] && mkdir -p $(BUILD)/obj; true
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/makeme-macosx-$(PROFILE)-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/makeme-macosx-$(PROFILE)-me.h >/dev/null ; then\
		cp projects/makeme-macosx-$(PROFILE)-me.h $(BUILD)/inc/me.h  ; \
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
	rm -f "$(BUILD)/obj/me.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/testme.o"
	rm -f "$(BUILD)/obj/zlib.o"
	rm -f "$(BUILD)/bin/ejs.testme.es"
	rm -f "$(BUILD)/bin/makeme-ejsc"
	rm -f "$(BUILD)/bin/makeme-ejs"
	rm -f "$(BUILD)/bin/http"
	rm -f "$(BUILD)/.install-roots-modified"
	rm -f "$(BUILD)/bin/libejs.dylib"
	rm -f "$(BUILD)/bin/libhttp.dylib"
	rm -f "$(BUILD)/bin/libmpr.dylib"
	rm -f "$(BUILD)/bin/libpcre.dylib"
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
#   FreeRTOS.h
#

$(BUILD)/inc/freertos/FreeRTOS.h: $(DEPS_2)

#
#   event_groups.h
#

$(BUILD)/inc/freertos/event_groups.h: $(DEPS_3)

#
#   task.h
#

$(BUILD)/inc/freertos/task.h: $(DEPS_4)

#
#   time.h
#

$(BUILD)/inc/time.h: $(DEPS_5)

#
#   esp_system.h
#

$(BUILD)/inc/esp_system.h: $(DEPS_6)

#
#   esp_log.h
#

$(BUILD)/inc/esp_log.h: $(DEPS_7)

#
#   esp_heap_caps.h
#

$(BUILD)/inc/esp_heap_caps.h: $(DEPS_8)

#
#   esp_err.h
#

$(BUILD)/inc/esp_err.h: $(DEPS_9)

#
#   esp_event.h
#

$(BUILD)/inc/esp_event.h: $(DEPS_10)

#
#   esp_psram.h
#

$(BUILD)/inc/esp_psram.h: $(DEPS_11)

#
#   esp_pthread.h
#

$(BUILD)/inc/esp_pthread.h: $(DEPS_12)

#
#   esp_littlefs.h
#

$(BUILD)/inc/esp_littlefs.h: $(DEPS_13)

#
#   esp_crt_bundle.h
#

$(BUILD)/inc/esp_crt_bundle.h: $(DEPS_14)

#
#   esp_wifi.h
#

$(BUILD)/inc/esp_wifi.h: $(DEPS_15)

#
#   esp_netif.h
#

$(BUILD)/inc/esp_netif.h: $(DEPS_16)

#
#   nvs_flash.h
#

$(BUILD)/inc/nvs_flash.h: $(DEPS_17)

#
#   err.h
#

$(BUILD)/inc/lwip/err.h: $(DEPS_18)

#
#   sockets.h
#

$(BUILD)/inc/lwip/sockets.h: $(DEPS_19)

#
#   sys.h
#

$(BUILD)/inc/lwip/sys.h: $(DEPS_20)

#
#   netdb.h
#

$(BUILD)/inc/lwip/netdb.h: $(DEPS_21)

#
#   osdep.h
#
DEPS_22 += src/osdep/osdep.h
DEPS_22 += $(BUILD)/inc/me.h
DEPS_22 += $(BUILD)/inc/freertos/FreeRTOS.h
DEPS_22 += $(BUILD)/inc/freertos/event_groups.h
DEPS_22 += $(BUILD)/inc/freertos/task.h
DEPS_22 += $(BUILD)/inc/time.h
DEPS_22 += $(BUILD)/inc/esp_system.h
DEPS_22 += $(BUILD)/inc/esp_log.h
DEPS_22 += $(BUILD)/inc/esp_heap_caps.h
DEPS_22 += $(BUILD)/inc/esp_err.h
DEPS_22 += $(BUILD)/inc/esp_event.h
DEPS_22 += $(BUILD)/inc/esp_psram.h
DEPS_22 += $(BUILD)/inc/esp_pthread.h
DEPS_22 += $(BUILD)/inc/esp_littlefs.h
DEPS_22 += $(BUILD)/inc/esp_crt_bundle.h
DEPS_22 += $(BUILD)/inc/esp_wifi.h
DEPS_22 += $(BUILD)/inc/esp_netif.h
DEPS_22 += $(BUILD)/inc/nvs_flash.h
DEPS_22 += $(BUILD)/inc/lwip/err.h
DEPS_22 += $(BUILD)/inc/lwip/sockets.h
DEPS_22 += $(BUILD)/inc/lwip/sys.h
DEPS_22 += $(BUILD)/inc/lwip/netdb.h

$(BUILD)/inc/osdep.h: $(DEPS_22)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_23 += src/mpr/mpr.h
DEPS_23 += $(BUILD)/inc/me.h
DEPS_23 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_23)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_24 += src/http/http.h
DEPS_24 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_24)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/http/http.h $(BUILD)/inc/http.h

#
#   ejs.slots.h
#

src/ejscript/ejs.slots.h: $(DEPS_25)

#
#   pcre.h
#
DEPS_26 += src/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_26)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   zlib.h
#
DEPS_27 += src/zlib/zlib.h
DEPS_27 += $(BUILD)/inc/me.h

$(BUILD)/inc/zlib.h: $(DEPS_27)
	@echo '      [Copy] $(BUILD)/inc/zlib.h'
	mkdir -p "$(BUILD)/inc"
	cp src/zlib/zlib.h $(BUILD)/inc/zlib.h

#
#   ejs.h
#
DEPS_28 += src/ejscript/ejs.h
DEPS_28 += $(BUILD)/inc/me.h
DEPS_28 += $(BUILD)/inc/osdep.h
DEPS_28 += $(BUILD)/inc/mpr.h
DEPS_28 += $(BUILD)/inc/http.h
DEPS_28 += src/ejscript/ejs.slots.h
DEPS_28 += $(BUILD)/inc/pcre.h
DEPS_28 += $(BUILD)/inc/zlib.h

$(BUILD)/inc/ejs.h: $(DEPS_28)
	@echo '      [Copy] $(BUILD)/inc/ejs.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejs.h $(BUILD)/inc/ejs.h

#
#   ejs.slots.h
#
DEPS_29 += src/ejscript/ejs.slots.h

$(BUILD)/inc/ejs.slots.h: $(DEPS_29)
	@echo '      [Copy] $(BUILD)/inc/ejs.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejs.slots.h $(BUILD)/inc/ejs.slots.h

#
#   ejsByteGoto.h
#
DEPS_30 += src/ejscript/ejsByteGoto.h

$(BUILD)/inc/ejsByteGoto.h: $(DEPS_30)
	@echo '      [Copy] $(BUILD)/inc/ejsByteGoto.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejscript/ejsByteGoto.h $(BUILD)/inc/ejsByteGoto.h

#
#   testme.h
#
DEPS_31 += src/tm/testme.h

$(BUILD)/inc/testme.h: $(DEPS_31)
	@echo '      [Copy] $(BUILD)/inc/testme.h'
	mkdir -p "$(BUILD)/inc"
	cp src/tm/testme.h $(BUILD)/inc/testme.h

#
#   ejs.h
#

src/ejscript/ejs.h: $(DEPS_32)

#
#   ejs.o
#
DEPS_33 += src/ejscript/ejs.h

$(BUILD)/obj/ejs.o: \
    src/ejscript/ejs.c $(DEPS_33)
	@echo '   [Compile] $(BUILD)/obj/ejs.o'
	$(CC) -c -o $(BUILD)/obj/ejs.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/ejscript/ejs.c

#
#   ejsLib.o
#
DEPS_34 += src/ejscript/ejs.h
DEPS_34 += $(BUILD)/inc/mpr.h
DEPS_34 += $(BUILD)/inc/pcre.h
DEPS_34 += $(BUILD)/inc/me.h

$(BUILD)/obj/ejsLib.o: \
    src/ejscript/ejsLib.c $(DEPS_34)
	@echo '   [Compile] $(BUILD)/obj/ejsLib.o'
	$(CC) -c -o $(BUILD)/obj/ejsLib.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/ejscript/ejsLib.c

#
#   ejsc.o
#
DEPS_35 += src/ejscript/ejs.h

$(BUILD)/obj/ejsc.o: \
    src/ejscript/ejsc.c $(DEPS_35)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c -o $(BUILD)/obj/ejsc.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/ejscript/ejsc.c

#
#   http.h
#

src/http/http.h: $(DEPS_36)

#
#   http.o
#
DEPS_37 += src/http/http.h

$(BUILD)/obj/http.o: \
    src/http/http.c $(DEPS_37)
	@echo '   [Compile] $(BUILD)/obj/http.o'
	$(CC) -c -o $(BUILD)/obj/http.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/http/http.c

#
#   httpLib.o
#
DEPS_38 += src/http/http.h
DEPS_38 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/httpLib.o: \
    src/http/httpLib.c $(DEPS_38)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/http/httpLib.c

#
#   me.o
#
DEPS_39 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/me.o: \
    src/me.c $(DEPS_39)
	@echo '   [Compile] $(BUILD)/obj/me.o'
	$(CC) -c -o $(BUILD)/obj/me.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/me.c

#
#   mpr.h
#

src/mpr/mpr.h: $(DEPS_40)

#
#   mprLib.o
#
DEPS_41 += src/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/mpr/mprLib.c $(DEPS_41)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c -o $(BUILD)/obj/mprLib.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/mpr/mprLib.c

#
#   pcre.h
#

src/pcre/pcre.h: $(DEPS_42)

#
#   pcre.o
#
DEPS_43 += $(BUILD)/inc/me.h
DEPS_43 += src/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/pcre/pcre.c $(DEPS_43)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c -o $(BUILD)/obj/pcre.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) $(IFLAGS) src/pcre/pcre.c

#
#   testme.o
#
DEPS_44 += $(BUILD)/inc/ejs.h

$(BUILD)/obj/testme.o: \
    src/tm/testme.c $(DEPS_44)
	@echo '   [Compile] $(BUILD)/obj/testme.o'
	$(CC) -c -o $(BUILD)/obj/testme.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC $(DFLAGS) -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) -DME_COM_MBEDTLS_PATH=$(ME_COM_MBEDTLS_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MBEDTLS_PATH)/include" src/tm/testme.c

#
#   zlib.h
#

src/zlib/zlib.h: $(DEPS_45)

#
#   zlib.o
#
DEPS_46 += $(BUILD)/inc/me.h
DEPS_46 += src/zlib/zlib.h

$(BUILD)/obj/zlib.o: \
    src/zlib/zlib.c $(DEPS_46)
	@echo '   [Compile] $(BUILD)/obj/zlib.o'
	$(CC) -c -o $(BUILD)/obj/zlib.o -arch $(CC_ARCH) -Wno-unknown-warning-option -fPIC -Wno-deprecated-non-prototype $(DFLAGS) $(IFLAGS) src/zlib/zlib.c

#
#   libmpr
#
DEPS_47 += $(BUILD)/inc/osdep.h
DEPS_47 += $(BUILD)/inc/mpr.h
DEPS_47 += $(BUILD)/obj/mprLib.o

ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmbedtls
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmbedcrypto
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_47 += -lmbedx509
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_47 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_47 += -lssl
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_47 += -lcrypto
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_47 += -L"$(ME_COM_OPENSSL_PATH)"
endif

$(BUILD)/bin/libmpr.dylib: $(DEPS_47)
	@echo '      [Link] $(BUILD)/bin/libmpr.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libmpr.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     -install_name @rpath/libmpr.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/mprLib.o" $(LIBPATHS_47) $(LIBS_47) $(LIBS_47) $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_48 += $(BUILD)/inc/pcre.h
DEPS_48 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.dylib: $(DEPS_48)
	@echo '      [Link] $(BUILD)/bin/libpcre.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libpcre.dylib -arch $(CC_ARCH) $(LDFLAGS) -compatibility_version 1.0 -current_version 1.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_49 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_PCRE),1)
    DEPS_49 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_49 += $(BUILD)/inc/http.h
DEPS_49 += $(BUILD)/obj/httpLib.o

LIBS_49 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmbedtls
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmbedcrypto
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_49 += -lmbedx509
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_49 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_49 += -lssl
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_49 += -lcrypto
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_49 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_49 += -lpcre
endif

$(BUILD)/bin/libhttp.dylib: $(DEPS_49)
	@echo '      [Link] $(BUILD)/bin/libhttp.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libhttp.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     -install_name @rpath/libhttp.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/httpLib.o" $(LIBPATHS_49) $(LIBS_49) $(LIBS_49) $(LIBS) 
endif

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_50 += $(BUILD)/inc/zlib.h
DEPS_50 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.dylib: $(DEPS_50)
	@echo '      [Link] $(BUILD)/bin/libzlib.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libzlib.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libzlib.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   libejs
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_51 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_PCRE),1)
    DEPS_51 += $(BUILD)/bin/libpcre.dylib
endif
DEPS_51 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_ZLIB),1)
    DEPS_51 += $(BUILD)/bin/libzlib.dylib
endif
DEPS_51 += $(BUILD)/inc/ejs.h
DEPS_51 += $(BUILD)/inc/ejs.slots.h
DEPS_51 += $(BUILD)/inc/ejsByteGoto.h
DEPS_51 += $(BUILD)/obj/ejsLib.o

LIBS_51 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedtls
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedcrypto
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedx509
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_51 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_51 += -lssl
    LIBPATHS_51 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_51 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_51 += -lcrypto
    LIBPATHS_51 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_51 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_51 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_51 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_51 += -lzlib
endif

$(BUILD)/bin/libejs.dylib: $(DEPS_51)
	@echo '      [Link] $(BUILD)/bin/libejs.dylib'
	$(CC) -dynamiclib -o $(BUILD)/bin/libejs.dylib -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     -install_name @rpath/libejs.dylib -compatibility_version 1.0 -current_version 1.0 "$(BUILD)/obj/ejsLib.o" $(LIBPATHS_51) $(LIBS_51) $(LIBS_51) $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejsc
#
DEPS_52 += $(BUILD)/bin/libejs.dylib
DEPS_52 += $(BUILD)/obj/ejsc.o

LIBS_52 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedtls
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedcrypto
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedx509
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_52 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_52 += -lssl
    LIBPATHS_52 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_52 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_52 += -lcrypto
    LIBPATHS_52 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_52 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_52 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_52 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_52 += -lzlib
endif
LIBS_52 += -lejs

$(BUILD)/bin/makeme-ejsc: $(DEPS_52)
	@echo '      [Link] $(BUILD)/bin/makeme-ejsc'
	$(CC) -o $(BUILD)/bin/makeme-ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     "$(BUILD)/obj/ejsc.o" $(LIBPATHS_52) $(LIBS_52) $(LIBS_52) $(LIBS) 
endif

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejs.mod
#
DEPS_53 += src/ejscript/ejs.es
DEPS_53 += $(BUILD)/bin/makeme-ejsc

$(BUILD)/bin/ejs.mod: $(DEPS_53)
	( \
	cd src/ejscript; \
	echo '   [Compile] ejs.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --out "../../$(BUILD)/bin/ejs.mod" --debug --bind --require null ejs.es ; \
	)
endif

#
#   ejs.testme.es
#
DEPS_54 += src/tm/ejs.testme.es

$(BUILD)/bin/ejs.testme.es: $(DEPS_54)
	@echo '      [Copy] $(BUILD)/bin/ejs.testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/ejs.testme.es $(BUILD)/bin/ejs.testme.es

#
#   ejs.testme.mod
#
DEPS_55 += src/tm/ejs.testme.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_55 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/ejs.testme.mod: $(DEPS_55)
	( \
	cd src/tm; \
	echo '   [Compile] ejs.testme.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --debug --out "../../$(BUILD)/bin/ejs.testme.mod" --optimize 9 ejs.testme.es ; \
	)

ifeq ($(ME_COM_EJSCRIPT),1)
#
#   ejscmd
#
DEPS_56 += $(BUILD)/bin/libejs.dylib
DEPS_56 += $(BUILD)/obj/ejs.o

LIBS_56 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedtls
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedcrypto
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedx509
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_56 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_56 += -lssl
    LIBPATHS_56 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_56 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_56 += -lcrypto
    LIBPATHS_56 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_56 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_56 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_56 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_56 += -lzlib
endif
LIBS_56 += -lejs

$(BUILD)/bin/makeme-ejs: $(DEPS_56)
	@echo '      [Link] $(BUILD)/bin/makeme-ejs'
	$(CC) -o $(BUILD)/bin/makeme-ejs -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     "$(BUILD)/obj/ejs.o" $(LIBPATHS_56) $(LIBS_56) $(LIBS_56) $(LIBS) -ledit 
endif

#
#   extras
#
DEPS_57 += src/Configure.es
DEPS_57 += src/Generate.es
DEPS_57 += src/vcvars.bat

$(BUILD)/.extras-modified: $(DEPS_57)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/Configure.es $(BUILD)/bin/Configure.es
	cp src/Generate.es $(BUILD)/bin/Generate.es
	cp src/vcvars.bat $(BUILD)/bin/vcvars.bat
	touch "$(BUILD)/.extras-modified" 2>/dev/null

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_58 += $(BUILD)/bin/libhttp.dylib
DEPS_58 += $(BUILD)/obj/http.o

LIBS_58 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_58 += -lmbedtls
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_58 += -lmbedcrypto
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_58 += -lmbedx509
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_58 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_58 += -lssl
    LIBPATHS_58 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_58 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_58 += -lcrypto
    LIBPATHS_58 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_58 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_58 += -lpcre
endif
LIBS_58 += -lhttp

$(BUILD)/bin/http: $(DEPS_58)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     "$(BUILD)/obj/http.o" $(LIBPATHS_58) $(LIBS_58) $(LIBS_58) $(LIBS) 
endif

#
#   install-roots
#
DEPS_59 += certs/roots.crt

$(BUILD)/.install-roots-modified: $(DEPS_59)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp certs/roots.crt $(BUILD)/bin/roots.crt
	touch "$(BUILD)/.install-roots-modified" 2>/dev/null

#
#   me.mod
#
DEPS_60 += paks/ejs.version/Version.es
DEPS_60 += src/Builder.es
DEPS_60 += src/Loader.es
DEPS_60 += src/MakeMe.es
DEPS_60 += src/Me.es
DEPS_60 += src/Script.es
DEPS_60 += src/Target.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_60 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/me.mod: $(DEPS_60)
	echo '   [Compile] me.mod' ; \
	"./$(BUILD)/bin/makeme-ejsc" --debug --out "./$(BUILD)/bin/me.mod" --optimize 9 paks/ejs.version/Version.es src/Builder.es src/Loader.es src/MakeMe.es src/Me.es src/Script.es src/Target.es

#
#   pakrun
#
DEPS_61 += paks/me-components/appweb.me
DEPS_61 += paks/me-components/compiler.me
DEPS_61 += paks/me-components/components.me
DEPS_61 += paks/me-components/ejscript.me
DEPS_61 += paks/me-components/lib.me
DEPS_61 += paks/me-components/link.me
DEPS_61 += paks/me-components/pak.json
DEPS_61 += paks/me-components/rc.me
DEPS_61 += paks/me-components/testme.me
DEPS_61 += paks/me-dev/dev.es
DEPS_61 += paks/me-dev/dev.me
DEPS_61 += paks/me-dev/pak.json
DEPS_61 += paks/me-doc/doc.es
DEPS_61 += paks/me-doc/doc.me
DEPS_61 += paks/me-doc/gendoc.es
DEPS_61 += paks/me-doc/pak.json
DEPS_61 += paks/me-installs/Installs.es
DEPS_61 += paks/me-installs/installs.me
DEPS_61 += paks/me-installs/manifest.me
DEPS_61 += paks/me-installs/pak.json
DEPS_61 += paks/me-make/Make.es
DEPS_61 += paks/me-make/make.me
DEPS_61 += paks/me-make/pak.json
DEPS_61 += paks/me-os/freebsd.me
DEPS_61 += paks/me-os/freertos.me
DEPS_61 += paks/me-os/gcc.me
DEPS_61 += paks/me-os/linux.me
DEPS_61 += paks/me-os/macosx.me
DEPS_61 += paks/me-os/os.me
DEPS_61 += paks/me-os/pak.json
DEPS_61 += paks/me-os/solaris.me
DEPS_61 += paks/me-os/unix.me
DEPS_61 += paks/me-os/vxworks.me
DEPS_61 += paks/me-os/windows.me
DEPS_61 += paks/me-vstudio/Vstudio.es
DEPS_61 += paks/me-vstudio/pak.json
DEPS_61 += paks/me-vstudio/vstudio.me
DEPS_61 += paks/me-win/make.bat
DEPS_61 += paks/me-win/pak.json
DEPS_61 += paks/me-win/win.me
DEPS_61 += paks/me-win/windows.bat
DEPS_61 += paks/me-xcode/Xcode.es
DEPS_61 += paks/me-xcode/pak.json
DEPS_61 += paks/me-xcode/xcode.me

$(BUILD)/.pakrun-modified: $(DEPS_61)
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
	mkdir -p "$(BUILD)/bin/paks/me-dev"
	cp paks/me-dev/dev.es $(BUILD)/bin/paks/me-dev/dev.es
	cp paks/me-dev/dev.me $(BUILD)/bin/paks/me-dev/dev.me
	cp paks/me-dev/pak.json $(BUILD)/bin/paks/me-dev/pak.json
	mkdir -p "$(BUILD)/bin/paks/me-doc"
	cp paks/me-doc/doc.es $(BUILD)/bin/paks/me-doc/doc.es
	cp paks/me-doc/doc.me $(BUILD)/bin/paks/me-doc/doc.me
	cp paks/me-doc/gendoc.es $(BUILD)/bin/paks/me-doc/gendoc.es
	cp paks/me-doc/pak.json $(BUILD)/bin/paks/me-doc/pak.json
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
	cp paks/me-os/freertos.me $(BUILD)/bin/paks/me-os/freertos.me
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
	touch "$(BUILD)/.pakrun-modified" 2>/dev/null

#
#   runtime
#
DEPS_62 += src/master-main.me
DEPS_62 += src/master-start.me
DEPS_62 += src/simple.me
DEPS_62 += src/standard.me
DEPS_62 += $(BUILD)/.pakrun-modified

$(BUILD)/.runtime-modified: $(DEPS_62)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/master-main.me $(BUILD)/bin/master-main.me
	cp src/master-start.me $(BUILD)/bin/master-start.me
	cp src/simple.me $(BUILD)/bin/simple.me
	cp src/standard.me $(BUILD)/bin/standard.me
	touch "$(BUILD)/.runtime-modified" 2>/dev/null

#
#   me
#
DEPS_63 += $(BUILD)/bin/libmpr.dylib
ifeq ($(ME_COM_HTTP),1)
    DEPS_63 += $(BUILD)/bin/libhttp.dylib
endif
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_63 += $(BUILD)/bin/libejs.dylib
endif
DEPS_63 += $(BUILD)/bin/me.mod
DEPS_63 += $(BUILD)/.runtime-modified
DEPS_63 += $(BUILD)/obj/me.o

LIBS_63 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedtls
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedcrypto
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedx509
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_63 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_63 += -lssl
    LIBPATHS_63 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_63 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_63 += -lcrypto
    LIBPATHS_63 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_63 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_63 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_63 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_63 += -lzlib
endif
ifeq ($(ME_COM_EJSCRIPT),1)
    LIBS_63 += -lejs
endif

$(BUILD)/bin/me: $(DEPS_63)
	@echo '      [Link] $(BUILD)/bin/me'
	$(CC) -o $(BUILD)/bin/me -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     "$(BUILD)/obj/me.o" $(LIBPATHS_63) $(LIBS_63) $(LIBS_63) $(LIBS) 

#
#   testme.mod
#
DEPS_64 += src/tm/testme.es
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_64 += $(BUILD)/bin/ejs.mod
endif

$(BUILD)/bin/testme.mod: $(DEPS_64)
	( \
	cd src/tm; \
	echo '   [Compile] testme.mod' ; \
	"../../$(BUILD)/bin/makeme-ejsc" --debug --out "../../$(BUILD)/bin/testme.mod" --optimize 9 testme.es ; \
	)

#
#   testme
#
ifeq ($(ME_COM_EJSCRIPT),1)
    DEPS_65 += $(BUILD)/bin/libejs.dylib
endif
DEPS_65 += $(BUILD)/bin/testme.mod
DEPS_65 += $(BUILD)/bin/ejs.testme.mod
DEPS_65 += $(BUILD)/inc/testme.h
DEPS_65 += $(BUILD)/obj/testme.o

LIBS_65 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_65 += -lmbedtls
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_65 += -lmbedcrypto
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_65 += -lmbedx509
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/lib"
    LIBPATHS_65 += -L"$(ME_COM_MBEDTLS_PATH)/library"
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_65 += -lssl
    LIBPATHS_65 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_65 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_65 += -lcrypto
    LIBPATHS_65 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_65 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_65 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_65 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_65 += -lzlib
endif
ifeq ($(ME_COM_EJSCRIPT),1)
    LIBS_65 += -lejs
endif

$(BUILD)/bin/testme: $(DEPS_65)
	@echo '      [Link] $(BUILD)/bin/testme'
	$(CC) -o $(BUILD)/bin/testme -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS)     "$(BUILD)/obj/testme.o" $(LIBPATHS_65) $(LIBS_65) $(LIBS_65) $(LIBS) 

#
#   testme.es
#
DEPS_66 += src/tm/testme.es

$(BUILD)/bin/testme.es: $(DEPS_66)
	@echo '      [Copy] $(BUILD)/bin/testme.es'
	mkdir -p "$(BUILD)/bin"
	cp src/tm/testme.es $(BUILD)/bin/testme.es

#
#   installPrep
#

installPrep: $(DEPS_67)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with sudo." ; \
	exit 255 ; \
	fi

#
#   stop
#

stop: $(DEPS_68)

#
#   installBinary
#

installBinary: $(DEPS_69)
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
	cp src/roots.crt $(ME_VAPP_PREFIX)/bin/roots.crt ; \
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
	cp paks/me-os/freertos.me $(ME_VAPP_PREFIX)/bin/paks/me-os/freertos.me ; \
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

start: $(DEPS_70)

#
#   install
#
DEPS_71 += installPrep
DEPS_71 += stop
DEPS_71 += installBinary
DEPS_71 += start

install: $(DEPS_71)

#
#   uninstall
#
DEPS_72 += stop

uninstall: $(DEPS_72)

#
#   uninstallBinary
#

uninstallBinary: $(DEPS_73)
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true

#
#   version
#

version: $(DEPS_74)
	echo $(VERSION)


EXTRA_MAKEFILE := $(strip $(wildcard ./projects/extra.mk))
ifneq ($(EXTRA_MAKEFILE),)
include $(EXTRA_MAKEFILE)
endif
