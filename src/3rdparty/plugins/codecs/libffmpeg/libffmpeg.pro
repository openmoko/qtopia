TEMPLATE  =   lib
CONFIG    +=  qtopia warn_on release 
HEADERS   =   yuv2rgb.h libffmpegplugin.h libffmpegpluginimpl.h
SOURCES   =   yuv2rgb_arm4l.S yuv2rgb.cpp libffmpegplugin.cpp libffmpegpluginimpl.cpp
TARGET          =  ffmpegplugin
TMAKE_CFLAGS    =  $(OPTFLAGS) -DHAVE_AV_CONFIG_H
DESTDIR	  	=  $(QPEDIR)/plugins/codecs 
INCLUDEPATH	+= $(QPEDIR)/include ./ ./libavcodec ./libav 
DEPENDPATH      += $(QPEDIR)/include ./ ./libavcodec ./libav
LIBS            += -lqpe -lpthread -lm ./libavcodec/libavcodec.a ./libav/libavformat.a
VERSION         =  1.0.0

