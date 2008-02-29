TEMPLATE	=   lib
CONFIG		-=  qt
CONFIG		+=  base amr
CONFIG		+=  network

TARGET          =   avformat

DESTDIR	  	=   $$(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavcodec
DEPENDPATH      +=  ./ ../ ../libavcodec

QMAKE_CFLAGS_RELEASE = -w -O5

TRANSLATIONS=

# handled by src/ipk_groups.pri


# Base files
base {
    # Defines needed by libavformat
    DEFINES	+=  HAVE_AV_CONFIG_H _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

    # Header files to libavformat
    HEADERS	+=  avformat.h avio.h config.h dv1394.h mpegts.h \
		    qtopiaavformat.h rtp.h rtspcodes.h avi.h \
		    barpainet.h dv.h framehook.h os_support.h \
		    qtpalette.h rtsp.h

    # Source files to libavformat
    SOURCES	+=  allformats.c asf.c au.c avidec.c avio.c \
		    aviobuf.c cutils.c ffm.c file.c dv.c \
		    img.c jpeg.c mov.c mpeg.c mpegts.c raw.c \
		    rm.c swf.c utils.c wav.c avienc.c gifdec.c \
		    mp3.c crc.c yuv4mpeg.c \
		    4xm.c audio.c dv1394.c flvdec.c mpegtsenc.c \
		    framehook.c grab.c idroq.c ipmovie.c \
		    mpjpeg.c os_support.c psxstr.c \
		    wc3movie.c
}

# Requires the AMR library in 3rdparty to also be compiled
amr {
    DEFINES	+=  AMR_NB AMR_NB_FIXED
    SOURCES	+=  amr.c
}

# For streaming support, these extra files need to be compiled
network {
    DEFINES	+=  CONFIG_NETWORK
    SOURCES	+=  tcp.c udp.c http.c rtpproto.c rtp.c rtsp.c
}

# These files are encoders, and we don't compile them
encoders {
    DEFINES	+=  CONFIG_ENCODERS
    SOURCES	+=  flvenc.c gif.c movenc.c
}

# The remaining files are also ones we don't compile
misc {
    SOURCES	+=  ogg.c barpainet.c flic.c segafilm.c sierravmd.c \
		    westwood.c idcin.c nut.c png.c pnm.c yuv.c
}

# Architecture specific defines
contains(QMAKE_ARCH,x86_)|contains(QMAKE_ARCH,mmx_) {
    DEFINES	+=  ARCH_X86=1
}
contains(QMAKE_ARCH,arm) {
    DEFINES	+=  ARCH_ARMV4L=1
}
contains(QMAKE_ARCH,alpha) {
    DEFINES	+=  ARCH_ALPHA=1
}
contains(QMAKE_ARCH,powerpc)|contains(QMAKE_ARCH,altivec) {
    DEFINES	+=  ARCH_POWERPC=1 WORDS_BIGENDIAN=1
}
contains(QMAKE_ARCH,ps2)|contains(QMAKE_ARCH,mmi) {
    DEFINES	+=  ARCH_MIPS=1 WORDS_BIGENDIAN=1
}
contains(QMAKE_ARCH,mips) {
    DEFINES	+=  ARCH_MIPS=1
}

