TEMPLATE	= lib
CONFIG		= qtopia warn_on release
HEADERS		= 
SOURCES		= $(QTDIR)/src/3rdparty/freetype/builds/unix/ftsystem.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftdebug.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftinit.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftbase.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftglyph.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftmm.c \
		  $(QTDIR)/src/3rdparty/freetype/src/base/ftbbox.c \
		  $(QTDIR)/src/3rdparty/freetype/src/autohint/autohint.c \
		  $(QTDIR)/src/3rdparty/freetype/src/cache/ftcache.c \
		  $(QTDIR)/src/3rdparty/freetype/src/cff/cff.c \
		  $(QTDIR)/src/3rdparty/freetype/src/cid/type1cid.c \
		  $(QTDIR)/src/3rdparty/freetype/src/psaux/psaux.c \
		  $(QTDIR)/src/3rdparty/freetype/src/psnames/psmodule.c \
		  $(QTDIR)/src/3rdparty/freetype/src/raster/raster.c \
		  $(QTDIR)/src/3rdparty/freetype/src/sfnt/sfnt.c \
		  $(QTDIR)/src/3rdparty/freetype/src/smooth/smooth.c \
		  $(QTDIR)/src/3rdparty/freetype/src/truetype/truetype.c \
		  $(QTDIR)/src/3rdparty/freetype/src/type1/type1.c \
		  $(QTDIR)/src/3rdparty/freetype/src/winfonts/winfnt.c

TARGET		= freetype
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
INCLUDEPATH	+= $(QTDIR)/src/3rdparty/freetype/src $(QTDIR)/src/3rdparty/freetype/include $(QTDIR)/src/3rdparty/freetype/builds/unix
VERSION		= 6.0.1

