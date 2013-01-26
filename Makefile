#
#   Makefile - Embedthis Bit Makefile wrapper for per-platform makefiles
#
#	This Makefile is for Unix/Linux and Cygwin. Use WinMake for windows.
#
#   You can use this Makefile and build via "make" with a pre-selected configuration. Alternatively,
#	you can build using the "bit" tool for for a fully configurable build. If you wish to 
#	cross-compile, you should use "bit".
#
#   Modify compiler and linker default definitions here:
#
#       export ARCH      = CPU architecture (x86, x64, ppc, ...)
#       export OS        = Operating system (linux, macosx, windows, vxworks, ...)
#       export CC        = Compiler to use 
#       export LD        = Linker to use
#       export DEBUG     = Set to debug or release for debug or optimized builds
#       export CONFIG    = Output directory for built items. Defaults to OS-ARCH-PROFILE
#       export CFLAGS    = Add compiler options. For example: -Wall
#       export DFLAGS    = Add compiler defines. For example: -DCOLOR=blue
#       export IFLAGS    = Add compiler include directories. For example: -I/extra/includes
#       export LDFLAGS   = Add linker options
#       export LIBPATHS  = Add linker library search directories. For example: -L/libraries
#       export LIBS      = Add linker libraries. For example: -lpthreads
#       export PROFILE   = Build profile, used in output products directory name
#
#	See projects/$(OS)-$(ARCH)-$(PROFILE)-bit.h for configuration default settings. Can override via
#	environment variables. For example: make BIT_PACK_SQLITE=0. These are converted to DFLAGS and 
#	will then override the bit.h default values.
#
NAME    := bit
OS      := $(shell uname | sed 's/CYGWIN.*/windows/;s/Darwin/macosx/' | tr '[A-Z]' '[a-z]')
MAKE    := $(shell if which gmake >/dev/null 2>&1; then echo gmake ; else echo make ; fi)
PROFILE := default
DEBUG	:= debug
EXT     := mk

ifeq ($(OS),windows)
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    ARCH?=x64
else
    ARCH?=x86
endif
    MAKE:= projects/windows.bat $(ARCH)
    EXT := nmake
else
	ARCH:= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
endif
BIN 	:= $(OS)-$(ARCH)-$(PROFILE)/bin

.EXPORT_ALL_VARIABLES:

all compile:
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) $@

clean clobber install uninstall run:
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) $@

help:
	@echo '' >&2
	@echo 'With make, the default configuration can be modified by setting make' >&2
	@echo 'variables. Set to 0 to disable and 1 to enable:' >&2
	@echo '' >&2
	@echo '      PROFILE            # Select default or static for static linking'
	@echo '      BIT_MPR_LOGGING    # Enable application logging'
	@echo '      BIT_MPR_TRACING    # Enable debug tracing'
	@echo '      BIT_PACK_EST       # Enable the EST SSL stack'
	@echo ''
	@echo 'For example, to disable CGI:' >&2
	@echo '' >&2
	@echo '      make BIT_PACK_CGI=0' >&2
	@echo '' >&2
	@echo 'Other make variables include:' >&2
	@echo '' >&2
	@echo '      ARCH, CC, CFLAGS, DFLAGS, IFLAGS, LD, LDFLAGS, LIBPATHS, LIBS, OS' >&2
	@echo '' >&2
	@echo 'Alternatively, for faster, easier and fully configurable building, install' >&2
	@echo 'bit from http://embedthis.com/downloads/bit/download.ejs and re-run'>&2
	@echo 'configure and then build with bit.' >&2
	@echo '' >&2
