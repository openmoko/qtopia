!qbuild{
qtopia_project(embedded lib)
TARGET = qtopiagfx
CONFIG += qtopia_visibility
CONFIG += no_tr
VERSION = 4.0.0
depends(3rdparty/libraries/easing)
}

#DEFINES+=QT_GREENPHONE_OPT

HEADERS=\
    routines.h\
    gfx.h\
    def_blur.h\
    def_color.h\
    def_blend.h\
    gfxpainter.h\
    def_memory.h\
    gfxparticles.h\
    gfximage.h\
    def_transform.h\
    def_blendhelper.h\
    gfxtimeline.h\
    gfxmipimage.h\
    gfxeasing.h

SOURCES=\
    routines.cpp\
    gfx.cpp\
    def_blur.cpp\
    def_color.cpp\
    def_blend.cpp\
    def_memory.cpp\
    gfxpainter.cpp\
    gfxparticles.cpp\
    gfximage.cpp\
    def_transform.cpp\
    gfxtimeline.cpp\
    gfxmipimage.cpp\
    gfxeasing.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopiagfx
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
