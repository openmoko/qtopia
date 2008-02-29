CONFIG		+=  qtopiaplugin 

# Defines useded by libav and libavcodec
DEFINES		+=  HAVE_AV_CONFIG_H _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

HEADERS		=   yuv2rgb.h \
		    videocodeccontext.h \
		    videoscalecontext.h \
		    audiocodeccontext.h \
		    mediapacket.h \
		    mediapacketbuffer.h \
		    ffmutex.h \
		    libffmpegplugin.h \
		    libffmpegpluginimpl.h

SOURCES		=   yuv2rgb.cpp \
		    videocodeccontext.cpp \
		    audiocodeccontext.cpp \
		    mediapacketbuffer.cpp \
		    libffmpegplugin.cpp \
		    libffmpegpluginimpl.cpp

TRANSLATABLES += $${HEADERS} $${SOURCES}

contains(QMAKE_ARCH,xscale) {
    QMAKE_ARCH      +=   arm
    DEFINES	    +=   ARCH_XSCALE
    SOURCES	    +=   yuv2rgb_xscale.S
}

contains(QMAKE_ARCH,arm) {
    SOURCES	+=   yuv2rgb_arm4l.S
}

TARGET          =   ffmpegplugin

INCLUDEPATH	+=  $${QTOPIA_DEPOT_PATH}/src/3rdparty/libraries/libavformat $${QTOPIA_DEPOT_PATH}/src/3rdparty/libraries/libavcodec
#DEPENDPATH      +=  $$(QPEDIR)/src/3rdparty/libraries/libavformat $$(QPEDIR)/src/3rdparty/libraries/libavcodec

LIBS            +=  -lpthread -lm -lavcodec -lavformat

QMAKE_CXXFLAGS_RELEASE = -w -O5

# handled by src/ipk_groups.pri

