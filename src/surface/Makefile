# Sources
DIR_SOURCES := surface.c able.c ram.c linux.c

ifeq ($(NSFB_SDL_AVAILABLE),yes)
  DIR_SOURCES := $(DIR_SOURCES) sdl.c
endif

ifeq ($(NSFB_XCB_AVAILABLE),yes)
  DIR_SOURCES := $(DIR_SOURCES) x.c
endif

ifeq ($(NSFB_VNC_AVAILABLE),yes)
  DIR_SOURCES := $(DIR_SOURCES) vnc.c
endif

include $(NSBUILD)/Makefile.subdir
