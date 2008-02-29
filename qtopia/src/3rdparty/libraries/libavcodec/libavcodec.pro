TEMPLATE	=   lib
CONFIG		-=  qt

TARGET          =   avcodec

DESTDIR	  	=   $$(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavformat
DEPENDPATH      +=  ./ ../ ../libavformat

include(libavcodec.pri)

CONFIG		-=  warn_on

DEFINES         += HAVE_AV_CONFIG_H \
                   _FILE_OFFSET_BITS=64 \
                   _LARGEFILE_SOURCE \
                   _GNU_SOURCE

SOURCES         += ac3enc.c \
                   adpcm.c \
                   adx.c \
                   allcodecs.c \
                   bitstream.c \
                   cabac.c \
                   dpcm.c \
                   dsputil.c \
                   dvbsub.c \
                   dvdsub.c \
                   error_resilience.c \
                   eval.c \
                   faandct.c \
                   fft.c \
                   g726.c \
                   golomb.c \
                   h263dec.c \
                   h263.c \
                   h264idct.c \
                   imgconvert.c \
                   imgresample.c \
                   integer.c \
                   jfdctfst.c \
                   jfdctint.c \
                   jrevdct.c \
                   mdct.c \
                   mem.c \
                   mjpeg.c \
                   motion_est.c \
                   mpeg12.c \
                   mpegaudiodec.c \
                   mpegaudio.c \
                   mpegvideo.c \
                   msmpeg4.c \
                   parser.c \
                   pcm.c \
                   pnm.c \
                   rangecoder.c \
                   ratecontrol.c \
                   rational.c \
                   raw.c \
                   resample2.c \
                   resample.c \
                   simple_idct.c \
                   utils.c \
                   vp3dsp.c \

contains(DEFINES,CONFIG_AMR_NB_DECODER) {
    DEFINES     += AMR_NB AMR_NB_FIXED
    SOURCES     += amr.c
    INCLUDEPATH	+= ../amr
    DEPENDPATH	+= ../amr
    PACKAGE_DEPENDS	+= amr
    LIBS	+= -lamr
}

contains(DEFINES,CONFIG_H261_DECODER) {
    SOURCES     += h261.c
}

contains(DEFINES,CONFIG_H264_DECODER) {
    SOURCES     += h264.c
}

contains(DEFINES,CONFIG_VORBIS_DECODER) {
    SOURCES     += vorbis.c
}

contains(QMAKE_ARCH,armv5e) {
    QMAKE_ARCH      +=  armv4l
    DEFINES         +=  ARCH_ARMV5E
    SOURCES         +=  armv4l/dsputil_arm_s.S
}

contains(QMAKE_ARCH,armv4l) {
    DEFINES         +=  ARCH_ARMV4L
    SOURCES         +=  armv4l/dsputil_arm.c \
                        armv4l/mpegvideo_arm.c \
                        armv4l/jrevdct_arm.S \
                        armv4l/simple_idct_arm.S
}

contains(QMAKE_ARCH,armv4l):contains(QMAKE_ARCH,iwmmxt) {
    DEFINES         += HAVE_IWMMXT
    SOURCES         += armv4l/dsputil_iwmmxt.c \
                       armv4l/mpegvideo_iwmmxt.c
}

# No tr() calls here
TRANSLATIONS=
