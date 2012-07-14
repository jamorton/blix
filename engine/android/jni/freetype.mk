
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

SOURCES := \
        base/ftbbox.c \
        base/ftbitmap.c \
        base/ftfstype.c \
        base/ftglyph.c \
        base/ftlcdfil.c \
        base/ftstroke.c \
        base/fttype1.c \
        base/ftxf86.c \
        base/ftbase.c \
        base/ftsystem.c \
        base/ftinit.c \
        base/ftgasp.c \
        raster/raster.c \
        sfnt/sfnt.c \
        smooth/smooth.c \
        autofit/autofit.c \
        truetype/truetype.c \
        cff/cff.c \
        psnames/psnames.c \
        pshinter/pshinter.c

SOURCES := $(addprefix ../$(DEPS_PATH)/freetype/src/,$(SOURCES))

LOCAL_MODULE     := libfreetype
LOCAL_SRC_FILES  := $(SOURCES)
LOCAL_C_INCLUDES := $(DEPS_PATH)/freetype/include
LOCAL_CFLAGS     := -O2 -fPIC -DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON

include $(BUILD_STATIC_LIBRARY)
