singleprocess:singleprocess=true
TEMPLATE	=   lib
CONFIG		+=  warn_on release 
CONFIG		-=  qt

# Defines needed by libav and libavcodec
DEFINES		+=  HAVE_AV_CONFIG_H _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

# Platform independent source files to libavcodec
SOURCES		+=  wmadec.c avcodec_utils.c svq1.c simple_idct.c rv10.c adpcm.c pcm.c mem.c \
		    mdct.c resample.c msmpeg4.c mpegvideo.c mpegaudio.c mpegaudiodec.c mpeg12.c \
		    ratecontrol.c mjpeg.c motion_est.c mace.c jrevdct.c jfdctint.c jfdctfst.c \
		    imgresample.c imgconvert.c h263dec.c h263.c error_resilience.c avcodec_dv.c \
		    fft.c dsputil.c eval.c common.c allcodecs.c huffyuv.c

TARGET          =   avcodec

DESTDIR	  	=   $(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavformat 
DEPENDPATH      +=  ./ ../ ../libavformat
VERSION         =   1.0.0

