#
#   Makefile - Embedthis Bit Makefile wrapper for per-platform makefiles
#
#   This Makefile is for Unix/Linux and Cygwin. On windows, it can be invoked via make.bat.
#
#	See projects/$(OS)-$(ARCH)-$(PROFILE)-bit.h for configuration default settings. Can override 
#	via make environment variables. For example: make BIT_PACK_SQLITE=0. These are converted to 
#	DFLAGS and will then override the bit.h default values. Use "make help" for a list of available 
#	make variable options.
#
NAME    := bit
OS      := $(shell uname | sed 's/CYGWIN.*/windows/;s/Darwin/macosx/' | tr '[A-Z]' '[a-z]')
MAKE    := $(shell if which gmake >/dev/null 2>&1; then echo gmake ; else echo make ; fi) --no-print-directory
PROFILE := default
DEBUG	:= debug
EXT     := mk

ifeq ($(OS),windows)
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    ARCH?=x64
else
    ARCH?=x86
endif
    MAKE:= MAKEFLAGS= projects/windows.bat $(ARCH)
    EXT := nmake
else
	ARCH:= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
endif
BIN 	:= $(OS)-$(ARCH)-$(PROFILE)/bin

.EXPORT_ALL_VARIABLES:

all compile:
	@if [ ! -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) ] ; then \
		echo "The build configuration projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) is not supported" ; exit 255 ; \
	fi
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) $@
	@echo '      [Info] You can now install via "sudo make $(MAKEFLAGS) install"'

clean clobber install installBinary uninstall run:
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) $@

deploy:
	@echo '       [Deploy] $(MAKE) BIT_ROOT_PREFIX=$(OS)-$(ARCH)-$(PROFILE)/deploy -f projects/$(NAME)-$(OS)-$(PROFILE).  $(EXT) installBinary'
	@$(MAKE) BIT_ROOT_PREFIX=$(OS)-$(ARCH)-$(PROFILE)/deploy -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) installBinary

version:
	@$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) $@

boot:
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) compile
	$(OS)-$(ARCH)-$(PROFILE)/bin/bit --release configure build

update:
	git pull
	$(MAKE) -f projects/$(NAME)-$(OS)-$(PROFILE).$(EXT) compile
	$(OS)-$(ARCH)-$(PROFILE)/bin/bit build

help:
	@echo '' >&2
	@echo 'usage: make [clean, compile, deploy, install, run, uninstall]' >&2
	@echo '' >&2
	@echo 'The default configuration can be modified by setting make variables' >&2
	@echo 'Set to 0 to disable and 1 to enable:' >&2
	@echo '' >&2
	@echo '  PROFILE            # Select default or static for static linking' >&2
	@echo '  BIT_MPR_LOGGING    # Enable application logging' >&2
	@echo '  BIT_MPR_TRACING    # Enable debug tracing' >&2
	@echo '  BIT_PACK_EST       # Enable the EST SSL stack' >&2
	@echo '' >&2
	@echo 'For example, to disable EST:' >&2
	@echo '' >&2
	@echo '  BIT_PACK_EST=0 make' >&2
	@echo '' >&2
	@echo 'Other make variables:' >&2
	@echo '  ARCH               # CPU architecture (x86, x64, ppc, ...)' >&2
	@echo '  OS                 # Operating system (linux, macosx, windows, vxworks, ...)' >&2
	@echo '  CC                 # Compiler to use ' >&2
	@echo '  LD                 # Linker to use' >&2
	@echo '  DEBUG              # Set to debug or release for debug or optimized builds' >&2
	@echo '  CONFIG             # Output directory for built items. Defaults to OS-ARCH-PROFILE' >&2
	@echo '  CFLAGS             # Add compiler options. For example: -Wall' >&2
	@echo '  DFLAGS             # Add compiler defines. For example: -DCOLOR=blue' >&2
	@echo '  IFLAGS             # Add compiler include directories. For example: -I/extra/includes' >&2
	@echo '  LDFLAGS            # Add linker options' >&2
	@echo '  LIBPATHS           # Add linker library search directories. For example: -L/libraries' >&2
	@echo '  LIBS               # Add linker libraries. For example: -lpthreads' >&2
	@echo '  PROFILE            # Build profile, used in output products directory name' >&2
	@echo '' >&2
	@echo 'Use "SHOW=1 make" to show executed commands.' >&2
	@echo '' >&2

