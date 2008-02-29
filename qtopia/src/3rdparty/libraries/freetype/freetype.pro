TEMPLATE	= lib

TARGET		= qt-freetype
DESTDIR		= $$(QPEDIR)/lib
INCLUDEPATH	+= $${QTEDIR}/src/3rdparty/freetype/src $${QTEDIR}/src/3rdparty/freetype/include

VERSION		= 6.0.1

CONFIG		-= qt
CONFIG          -= warn_on

unix {
    SOURCES	= $${QTEDIR}/src/3rdparty/freetype/builds/unix/ftsystem.c
    INCLUDEPATH += $${QTEDIR}/src/3rdparty/freetype/builds/unix
}

# We must get the right Win32 runtime ie -MD or -MDd
win32 {
    CONFIG += qtopia #??
    SOURCES = $${QTEDIR}/src/3rdparty/freetype/src/base/ftsystem.c 
}

SOURCES += \ 	  
            $${QTEDIR}/src/3rdparty/freetype/src/autohint/autohint.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftbase.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftbbox.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftdebug.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftglyph.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftinit.c \
            $${QTEDIR}/src/3rdparty/freetype/src/base/ftmm.c \
            $${QTEDIR}/src/3rdparty/freetype/src/bdf/bdf.c \
            $${QTEDIR}/src/3rdparty/freetype/src/cache/ftcache.c \
            $${QTEDIR}/src/3rdparty/freetype/src/cff/cff.c \
            $${QTEDIR}/src/3rdparty/freetype/src/cid/type1cid.c \
            $${QTEDIR}/src/3rdparty/freetype/src/gzip/ftgzip.c \
            $${QTEDIR}/src/3rdparty/freetype/src/pcf/pcf.c \
            $${QTEDIR}/src/3rdparty/freetype/src/pfr/pfr.c \
            $${QTEDIR}/src/3rdparty/freetype/src/psaux/psaux.c \
            $${QTEDIR}/src/3rdparty/freetype/src/pshinter/pshinter.c \
            $${QTEDIR}/src/3rdparty/freetype/src/psnames/psmodule.c \
            $${QTEDIR}/src/3rdparty/freetype/src/raster/raster.c \
            $${QTEDIR}/src/3rdparty/freetype/src/sfnt/sfnt.c \
            $${QTEDIR}/src/3rdparty/freetype/src/smooth/smooth.c \
            $${QTEDIR}/src/3rdparty/freetype/src/truetype/truetype.c \
            $${QTEDIR}/src/3rdparty/freetype/src/type1/type1.c \
            $${QTEDIR}/src/3rdparty/freetype/src/type42/type42.c \
            $${QTEDIR}/src/3rdparty/freetype/src/winfonts/winfnt.c \
            $${QTEDIR}/src/3rdparty/freetype/src/lzw/ftlzw.c


PACKAGE_NAME=freetype

TRANSLATIONS=
