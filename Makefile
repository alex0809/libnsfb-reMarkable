# Component settings
COMPONENT := nsfb
COMPONENT_VERSION := 0.0.1
# Default to a static library
COMPONENT_TYPE ?= lib-static

# Setup the tooling
include build/makefiles/Makefile.tools

# Reevaluate when used, as BUILDDIR won't be defined yet
TESTRUNNER = test/runtest.sh $(BUILDDIR) $(EXEEXT)

# Toolchain flags
WARNFLAGS := -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align \
	-Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Werror -pedantic \
	-Wno-overlength-strings # For nsglobe.c
CFLAGS := -g -std=c99 -D_BSD_SOURCE -I$(CURDIR)/include/ \
	-I$(CURDIR)/src $(WARNFLAGS) $(CFLAGS)

# TODO: probably want to retrieve SDL from pkg-config
TESTLDFLAGS = -Wl,--whole-archive -l$(COMPONENT) -Wl,--no-whole-archive -lSDL

include build/makefiles/Makefile.top

# Extra installation rules
I := /include
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_plot.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_plot_util.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_event.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_cursor.h
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib/pkgconfig:lib$(COMPONENT).pc.in
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib:$(OUTPUT)
