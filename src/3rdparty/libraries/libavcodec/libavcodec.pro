TEMPLATE	=   lib
CONFIG		-=  qt

CONFIG		+=  base video audio
video:CONFIG	+=  mjpeg mpeg mpeg4
audio:CONFIG	+=  adpcm pcm mp3 amr wma
# CONFIG	+=  encode misc

TARGET          =   avcodec

DESTDIR	  	=   $$(QPEDIR)/lib
INCLUDEPATH	+=  ./ ../ ../libavformat
DEPENDPATH      +=  ./ ../ ../libavformat

QMAKE_CFLAGS_RELEASE = -w -O5

TRANSLATIONS	=


# Base source files
base {
    # Defines needed by libavformat and libavcodec
    DEFINES	+=  CONFIG_ALIGN HAVE_MEMALIGN PIC CONFIG_RISKY HAVE_AV_CONFIG_H \
		    _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _GNU_SOURCE

    # Base source files to libavcodec
    SOURCES	+=  simple_idct.c mdct.c mem.c resample.c \
		    jrevdct.c jfdctint.c jfdctfst.c error_resilience.c \
		    dsputil.c eval.c common.c allcodecs.c avcodec.c \
		    utils.c opts.c rational.c imgresample.c imgconvert.c

    # Other sources we generally need for a variety of codecs
    SOURCES	+=  h263dec.c h263.c \
		    rv10.c raw.c dv.c svq1.c h264.c golomb.c mpegvideo.c
}

# Video codec source files
mjpeg {
    DEFINES	+=  CONFIG_MJPEGD
    SOURCES	+=  mjpeg.c
}
mpeg {
    DEFINES	+=  CONFIG_MPEGD
    SOURCES	+=  mpeg12.c
}
mpeg4 {
    DEFINES	+=  CONFIG_MPEG4D
    SOURCES	+=  msmpeg4.c
#   CONFIG	+=  aac
}

# Audio codec source files
adpcm {
    DEFINES	+=  CONFIG_ADPCM
    SOURCES	+=  adpcm.c
}
pcm {
    DEFINES	+=  CONFIG_PCM
    SOURCES	+=  pcm.c
}
mp3 {
    DEFINES	+=  CONFIG_MP3D
    SOURCES	+=  mpegaudiodec.c
}
wma {
    DEFINES	+=  CONFIG_WMAD
    SOURCES	+=  wmadec.c fft.c
}
aac {
    DEFINES     +=  CONFIG_FAADBIN CONFIG_FAAD
    SOURCES	+=  faad.c
    INCLUDEPATH	+= ../libfaad
    DEPENDPATH	+= ../libfaad
    PACKAGE_DEPENDS	+= faad
    LIBS	+= -lfaad
}
amr {
    DEFINES     +=  AMR_NB_FIXED AMR_NB 
    SOURCES	+=  amr.c
    INCLUDEPATH	+= ../amr
    DEPENDPATH	+= ../amr
    PACKAGE_DEPENDS	+= amr
    LIBS	+= -lamr
}


# Encoder related source files
encode {
    SOURCES	+=  ratecontrol.c motion_est.c mpegaudio.c
}

ac3enc {
    SOURCES	+=  ac3enc.c fft.c
}

# Misc other files
misc {
    SOURCES	+=  mace.c huffyuv.c
}

# Other files we don't compile, misc formats that aren't very common or test code
extra {
    SOURCES	+=  smc.c 4xm.c 8bps.c adx.c apiexample.c \
		    asv1.c cabac.c cinepak.c cljr.c \
		    cyuv.c dpcm.c fdctref.c ffv1.c \
		    flicvideo.c idcinvideo.c indeo3.c interplayvideo.c \
		    msrle.c xan.c xvmcvideo.c \
		    msvideo1.c ra144.c ra288.c roqvideo.c rpza.c \
		    truemotion1.c vcr1.c vmdav.c vp3.c vqavideo.c
}


# Architecture specific optimized functions
# Disabled for now as some compilers are having trouble with the MMX code
contains(QMAKE_ARCH,x86_)|contains(QMAKE_ARCH,mmx_) {
    DEFINES	    +=	ARCH_X86=1 HAVE_MMX=1
    SOURCES	    +=	i386/fdct_mmx.c \
			i386/cputest.c \
			i386/dsputil_mmx.c \
			i386/mpegvideo_mmx.c \
			i386/idct_mmx.c \
			i386/motion_est_mmx.c \
			i386/simple_idct_mmx.c
}
contains(QMAKE_ARCH,xscale) {
    QMAKE_CFLAGS    +=  -mcpu=xscale -mtune=xscale
    QMAKE_ARCH	    +=  arm
}
contains(QMAKE_ARCH,arm) {
    QMAKE_CFLAGS    +=	-fexpensive-optimizations -fschedule-insns -march=armv4
    DEFINES	    +=	ARCH_ARMV4L=1 CONFIG_MPEGAUDIO_HP
    SOURCES	    +=	armv4l/dsputil_arm.c \
			armv4l/mpegvideo_arm.c \
			armv4l/jrevdct_arm.S \
			armv4l/simple_idct_arm.S
}
contains(QMAKE_ARCH,alpha) {
    DEFINES	    +=	ARCH_ALPHA=1
}
contains(QMAKE_ARCH,powerpc) {
    DEFINES	    +=	ARCH_POWERPC=1 WORDS_BIGENDIAN=1 HAVEALTIVEC=0
    SOURCES	    +=	ppc/dsputil_ppc.c \
			ppc/mpegvideo_ppc.c
}
contains(QMAKE_ARCH,altivec) {
    QMAKE_CFLAGS_RELEASE    +=	-faltivec
    DEFINES	    +=	ARCH_POWERPC=1 WORDS_BIGENDIAN=1 HAVEALTIVEC=1
    SOURCES	    +=	ppc/dsputil_altivec.c \
			ppc/dsputil_ppc.c \
			ppc/idct_altivec.c \
			ppc/mpegvideo_altivec.c \
			ppc/mpegvideo_ppc.c
}
contains(QMAKE_ARCH,ps2)|contains(QMAKE_ARCH,mmi) {
    DEFINES	    +=	ARCH_MIPS=1 WORDS_BIGENDIAN=1 HAVE_MMI=1
    SOURCES	    +=	ps2/dsputil_mmi.c \
			ps2/idct_mmi.c \
			ps2/mpegvideo_mmi.c
}
contains(QMAKE_ARCH,mips) {
    DEFINES	    +=	ARCH_MIPS=1
}

