# Sources
DIR_SOURCES := surface.c vnc.c able.c ram.c linux.c

ifeq ($(NSFB_SDL_AVAILABLE),yes)
  DIR_SOURCES := $(DIR_SOURCES) sdl.c
endif

ifeq ($(NSFB_XCB_AVAILABLE),yes)
  DIR_SOURCES := $(DIR_SOURCES) x.c
endif

include build/makefiles/Makefile.subdir
