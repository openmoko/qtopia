singleprocess:singleprocess=true

TEMPLATE	=   lib
CONFIG		+=  warn_on release 
CONFIG		-=  qt

# Defines needed by libav 
DEFINES		+=  HAVE_AV_CONFIG_H _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

# Source files to libav
SOURCES		+=  allformats.c asf.c au.c avidec.c avio.c \
		    aviobuf.c crc.c cutils.c ffm.c file.c gif.c \
		    http.c img.c jpeg.c mov.c mpeg.c mpegts.c raw.c \
		    rm.c rtp.c rtpproto.c rtsp.c swf.c tcp.c udp.c \
		    utils.c wav.c avienc.c dv.c

TARGET          =   avformat

DESTDIR	  	=   $(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavcodec
DEPENDPATH      +=  ./ ../ ../libavcodec
VERSION         =   1.0.0

