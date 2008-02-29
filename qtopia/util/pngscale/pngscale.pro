include($$(QPEDIR)/src/config.pri)
TEMPLATE	= app
CONFIG          -= qt debug
CONFIG		+= release
DEFINES		-= QT_DLL
TARGET		= pngscale
DESTDIR		= $$(QPEDIR)/bin

DEFINES		+= QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_COMPONENT \
		   QT_NO_STL QT_NO_COMPRESS HAVE_QCONFIG_CPP \
		   QT_NO_IMAGE_TEXT QT_NO_MIME QT_NO_IMAGEIO_BMP \
		   QT_NO_IMAGEIO_PPM QT_NO_IMAGEIO_XBM QT_NO_IMAGEIO_MNG \
		   QT_NO_IMAGEIO_JPEG QT_NO_IMAGE_TRANSFORMATION \
		   QT_NO_ASYNC_IMAGE_IO QT_NO_IMAGEIO_XPM

INCLUDEPATH	+= $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools \
		   $${QTOPIA_DEPOT_PATH}/qmake/qt/src/kernel \
		   $$(QTDIR)/src/3rdparty/libpng \
		   zlib

SOURCES		= main.cpp qimage.cpp qasyncimageio.cpp qpngio.cpp \
		  qrect.cpp qsize.cpp qpoint.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qstring.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qtextstream.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qiodevice.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qglobal.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgdict.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qcstring.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdatastream.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgarray.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qbuffer.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qglist.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qptrcollection.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfile.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qregexp.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgvector.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgcache.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qbitarray.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdir.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/quuid.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfileinfo.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdatetime.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qstringlist.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qmap.cpp \
		  $$(QPEDIR)/qmake/qt/src/tools/qconfig.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qunicodetables.cpp \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qsettings.cpp

unix:!mac:SOURCES+= $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfile_unix.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdir_unix.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfileinfo_unix.cpp

mac:SOURCES	+=  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfile_unix.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdir_unix.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfileinfo_unix.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qsettings_mac.cpp

win32:SOURCES	+=  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfile_win.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdir_win.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfileinfo_win.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qsettings_win.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qlibrary.cpp \
		    $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qlibrary_win.cpp \

PNG_SOURCES	= $$(QTDIR)/src/3rdparty/libpng/png.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngerror.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngget.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngmem.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngpread.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngread.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngrio.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngrtran.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngrutil.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngset.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngtrans.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngwio.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngwrite.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngwtran.c \
		  $$(QTDIR)/src/3rdparty/libpng/pngwutil.c

ZLIB_SOURCES	= zlib/adler32.c \
		  zlib/compress.c \
		  zlib/crc32.c \
		  zlib/deflate.c \
		  zlib/gzio.c \
		  zlib/infblock.c \
		  zlib/infcodes.c \
		  zlib/inffast.c \
		  zlib/inflate.c \
		  zlib/inftrees.c \
		  zlib/infutil.c \
		  zlib/trees.c \
		  zlib/uncompr.c \
		  zlib/zutil.c

HEADERS		= qimage.h qasyncimageio.cpp qpngio.cpp \
		  qrect.h qsize.h qpoint.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qstring.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qtextstream.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qiodevice.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qglobal.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgdict.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qcstring.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdatastream.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgarray.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qbuffer.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qglist.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qptrcollection.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfile.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qregexp.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgcache.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qgvector.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qbitarray.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdir.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/quuid.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qfileinfo.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qdatetime.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qstringlist.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qmap.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/kernel/qconfig.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qunicodetables_p.h \
		  $${QTOPIA_DEPOT_PATH}/qmake/qt/src/tools/qsettings.h

# Do one of the following depending upon whether we want to use system libs
SOURCES		+= $$PNG_SOURCES $$ZLIB_SOURCES
#LIBS		+= -lpng -lz

mac:LIBS	+= -framework Carbon
mac:CONFIG	-= resource_fork
win32:LIBS	+= ole32.lib advapi32.lib

