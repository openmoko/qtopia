TEMPLATE	=   lib
CONFIG		-=  qt

TARGET          =   avformat

DESTDIR	  	=   $$(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavcodec
DEPENDPATH      +=  ./ ../ ../libavcodec

include(libavformat.pri)

CONFIG -= warn_on 

DEFINES         += HAVE_AV_CONFIG_H \
                   _FILE_OFFSET_BITS=64 \
                   _LARGEFILE_SOURCE \
                   _GNU_SOURCE

SOURCES         += allformats.c \
                   cutils.c \
                   os_support.c \
                   utils.c

SOURCES         += 4xm.c \
                   asf.c \
                   asf-enc.c \
                   au.c \
                   avidec.c \
                   avienc.c \
                   crc.c \
                   dv.c \
                   electronicarts.c \
                   ffm.c \
                   flic.c \
                   flvdec.c \
                   flvenc.c \
                   gif.c \
                   idcin.c \
                   idroq.c \
                   img2.c \
                   img.c \
                   ipmovie.c \
                   matroska.c \
                   mov.c \
                   movenc.c \
                   mp3.c \
                   mpeg.c \
                   mpegts.c \
                   mpegtsenc.c \
                   mpjpeg.c \
                   nsvdec.c \
                   nut.c \
                   ogg2.c \
                   oggparseflac.c \
                   oggparsetheora.c \
                   oggparsevorbis.c \
                   psxstr.c \
                   raw.c \
                   rm.c \
                   segafilm.c \
                   sierravmd.c \
                   sol.c \
                   swf.c \
                   wav.c \
                   wc3movie.c \
                   westwood.c \
                   yuv4mpeg.c

SOURCES         += gifdec.c

SOURCES         += avio.c \
                   aviobuf.c \
                   file.c

SOURCES         += framehook.c

contains(DEFINES,AMR_NB) {
    SOURCES     += amr.c
}

contains(DEFINES,CONFIG_NETWORK) {
    SOURCES     += udp.c \
                   tcp.c \
                   http.c \
                   rtsp.c \
                   rtp.c \
                   rtpproto.c
}

# No tr() calls here
TRANSLATIONS=
