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

NSFB_XCB_PKG_NAMES := xcb xcb-icccm xcb-image xcb-keysyms

$(eval $(call pkg_config_package_available,NSFB_VNC_AVAILABLE,libvncserver))
$(eval $(call pkg_config_package_available,NSFB_SDL_AVAILABLE,sdl))
$(eval $(call pkg_config_package_available,NSFB_XCB_AVAILABLE,$(NSFB_XCB_PKG_NAMES)))

ifeq ($(NSFB_SDL_AVAILABLE),yes)
  $(eval $(call pkg_config_package_add_flags,sdl,CFLAGS))
  $(eval $(call pkg_config_package_add_flags,sdl,TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) sdl
endif 

ifeq ($(NSFB_XCB_AVAILABLE),yes)
  $(eval $(call pkg_config_package_min_version,NSFB_XCB_NEW_API,xcb,0.23))

  ifeq ($(NSFB_XCB_NEW_API),yes)
    CFLAGS := $(CFLAGS) -DNEED_HINTS_ALLOC
  endif

  $(eval $(call pkg_config_package_add_flags,$(NSFB_XCB_PKG_NAMES),CFLAGS))
  $(eval $(call pkg_config_package_add_flags,$(NSFB_XCB_PKG_NAMES),TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) $(NSFB_XCB_PKG_NAMES)
endif

ifeq ($(NSFB_VNC_AVAILABLE),yes)
  $(eval $(call pkg_config_package_add_flags,libvncserver,CFLAGS))
  $(eval $(call pkg_config_package_add_flags,libvncserver,TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) libvncserver
endif 

TESTLDFLAGS := -Wl,--whole-archive -l$(COMPONENT) -Wl,--no-whole-archive $(TESTLDFLAGS)

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
