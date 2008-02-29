singleprocess:singleprocess=true

TEMPLATE	=   lib
CONFIG		+=  qtopia warn_on release 
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Defines useded by libav and libavcodec
DEFINES		+=  HAVE_AV_CONFIG_H _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

HEADERS		=   yuv2rgb.h libffmpegplugin.h libffmpegpluginimpl.h

SOURCES		=   yuv2rgb.cpp libffmpegplugin.cpp libffmpegpluginimpl.cpp

TARGET          =   ffmpegplugin

DESTDIR	  	=   $(QPEDIR)/plugins/codecs
INCLUDEPATH	+=  ./ $(QPEDIR)/src/3rdparty/libraries/libavformat $(QPEDIR)/src/3rdparty/libraries/libavcodec
DEPENDPATH      +=  ./ $(QPEDIR)/src/3rdparty/libraries/libavformat $(QPEDIR)/src/3rdparty/libraries/libavcodec
LIBS            +=  -lpthread -lm -lavcodec -lavformat
VERSION         =   1.0.0

TRANSLATIONS = libffmpegplugin-en_GB.ts libffmpegplugin-de.ts libffmpegplugin-ja.ts libffmpegplugin-no.ts
