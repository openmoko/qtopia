#############################################################################
# Automatically generated from 3rdparty/libraries/libavcodec/Makefile.in
# Build options from 
#############################################################################

default: all  # 

CONTROL = 

# Compiling
INTERFACE_DECL_PATH 	= .
SYSCONF_CXX		= g++
SYSCONF_CC		= gcc
DASHCROSS		= 

# Compiling with support libraries
SYSCONF_CXXFLAGS_X11	= 
SYSCONF_CXXFLAGS_QT	= -I$(QTDIR)/include
SYSCONF_CXXFLAGS_QTOPIA	= -I$(QPEDIR)/include
SYSCONF_CFLAGS_QTOPIA	= -I$(QPEDIR)/include
SYSCONF_CXXFLAGS_OPENGL	= -I/usr/X11R6/include

# Compiling YACC output
SYSCONF_CXXFLAGS_YACC     = -Wno-unused -Wno-parentheses

# Linking with support libraries
SYSCONF_RPATH_X11	= 
SYSCONF_RPATH_QT	= -Wl,-rpath,$(QTDIR)/lib
SYSCONF_RPATH_QTOPIA	= -Wl,-rpath,$(QPEDIR)/lib
SYSCONF_RPATH_OPENGL	= -Wl,-rpath,/usr/X11R6/lib

# Linking with support libraries
# X11
SYSCONF_LFLAGS_X11	= 
SYSCONF_LIBS_X11	= 
# Qt, Qt+OpenGL
SYSCONF_LFLAGS_QT	= -L$(QTDIR)/lib
SYSCONF_LFLAGS_QTOPIA	= -L$(QPEDIR)/lib
SYSCONF_LIBS_QT		= -lqte$(QT_THREAD_SUFFIX)
SYSCONF_LIBS_QT_OPENGL	= 
SYSCONF_LIBS_QTOPIA	= -lqtopia
# OpenGL
SYSCONF_LFLAGS_OPENGL	= -L/usr/X11R6/lib
SYSCONF_LIBS_OPENGL	= 
# Yacc
SYSCONF_LIBS_YACC	= 

# Linking applications
SYSCONF_LINK		= gcc
SYSCONF_LFLAGS		= 
SYSCONF_LIBS		= 

# Link flags for shared objects
SYSCONF_LFLAGS_SHOBJ	= -shared

# Flags for threading
SYSCONF_CFLAGS_THREAD	= -D_REENTRANT
SYSCONF_CXXFLAGS_THREAD	= -D_REENTRANT
SYSCONF_LFLAGS_THREAD	= 
SYSCONF_LIBS_THREAD	=  -lpthread

# Meta-object compiler
SYSCONF_MOC		= $(QTDIR)/bin/moc

# UI compiler
SYSCONF_UIC		= $(QTDIR)/bin/uic

# Linking shared libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).so.2.2.2
#   - Place target in $(DESTDIR) - which has a trailing /
#   - Usually needs to incorporate $(VER_MAJ), $(VER_MIN) and $(VER_PATCH)
#
SYSCONF_LINK_SHLIB	= gcc
SYSCONF_LINK_TARGET_SHARED	= lib$(TARGET).so.$(VER_MAJ).$(VER_MIN).$(VER_PATCH)
SYSCONF_LINK_LIB_SHARED	=  $(SYSCONF_LINK_SHLIB) -shared -Wl,-soname,lib$(TARGET).so.$(VER_MAJ) \
				     $(LFLAGS) -o $(SYSCONF_LINK_TARGET_SHARED) \
				     $(OBJECTS) $(OBJMOC) $(LIBS) && \
				 mv $(SYSCONF_LINK_TARGET_SHARED) $(DESTDIR); \
				 cd $(DESTDIR) && \
				 rm -f lib$(TARGET).so lib$(TARGET).so.$(VER_MAJ) lib$(TARGET).so.$(VER_MAJ).$(VER_MIN); \
				 ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so; \
				 ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ); \
				 ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ).$(VER_MIN)

# Linking static libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).a
#   - Place target in $(DESTDIR) - which has a trailing /
#
SYSCONF_AR		= ar cqs
SYSCONF_LINK_TARGET_STATIC = lib$(TARGET).a
SYSCONF_LINK_LIB_STATIC	= rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) ; \
				 $(SYSCONF_AR) $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS) $(OBJMOC) 
# Compiling application source
SYSCONF_CXXFLAGS	= -pipe -DQWS -fno-exceptions -fno-rtti -O2 -Wall -W -DNO_DEBUG
SYSCONF_CFLAGS		= -pipe -O2 -Wall -W
# Default link type (static linking is still be used where required)
SYSCONF_LINK_LIB	= $(SYSCONF_LINK_LIB_SHARED)
SYSCONF_LINK_TARGET	= $(SYSCONF_LINK_TARGET_SHARED)
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= -fPIC
SYSCONF_CFLAGS_LIB	= -fPIC
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= -fPIC
SYSCONF_CFLAGS_SHOBJ	= -fPIC
# Linking Qt
SYSCONF_LIBS_QTLIB	= $(SYSCONF_LFLAGS_X11) $(QT_LIBS_MT) $(QT_LIBS_OPT)
# Linking Qt applications
SYSCONF_LIBS_QTAPP	= 

# Platform output directories
MOC_DIR = .moc/linux-generic-g++/
OBJECTS_DIR = .obj/linux-generic-g++/

#############################################################################

####### Compiler, tools and options

CXX	=	$(SYSCONF_CXX) $(QT_CXX_MT)
CXXFLAGS=	$(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DQTOPIA_APP_INTERFACE -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -w -O5
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DQTOPIA_APP_INTERFACE -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -w -O5
INCPATH =	-I$(MOC_DIR) -I. -I.. -I../libavformat
LFLAGS	=	$(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS)  $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= avcodec
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	
SOURCES =	wmadec.c \
		avcodec_utils.c \
		svq1.c \
		simple_idct.c \
		rv10.c \
		adpcm.c \
		pcm.c \
		mem.c \
		mdct.c \
		resample.c \
		msmpeg4.c \
		mpegvideo.c \
		mpegaudio.c \
		mpegaudiodec.c \
		mpeg12.c \
		ratecontrol.c \
		mjpeg.c \
		motion_est.c \
		mace.c \
		jrevdct.c \
		jfdctint.c \
		jfdctfst.c \
		imgresample.c \
		imgconvert.c \
		h263dec.c \
		h263.c \
		error_resilience.c \
		avcodec_dv.c \
		fft.c \
		dsputil.c \
		eval.c \
		common.c \
		allcodecs.c \
		huffyuv.c
OBJECTS =	$(OBJECTS_DIR)/wmadec.o \
		$(OBJECTS_DIR)/avcodec_utils.o \
		$(OBJECTS_DIR)/svq1.o \
		$(OBJECTS_DIR)/simple_idct.o \
		$(OBJECTS_DIR)/rv10.o \
		$(OBJECTS_DIR)/adpcm.o \
		$(OBJECTS_DIR)/pcm.o \
		$(OBJECTS_DIR)/mem.o \
		$(OBJECTS_DIR)/mdct.o \
		$(OBJECTS_DIR)/resample.o \
		$(OBJECTS_DIR)/msmpeg4.o \
		$(OBJECTS_DIR)/mpegvideo.o \
		$(OBJECTS_DIR)/mpegaudio.o \
		$(OBJECTS_DIR)/mpegaudiodec.o \
		$(OBJECTS_DIR)/mpeg12.o \
		$(OBJECTS_DIR)/ratecontrol.o \
		$(OBJECTS_DIR)/mjpeg.o \
		$(OBJECTS_DIR)/motion_est.o \
		$(OBJECTS_DIR)/mace.o \
		$(OBJECTS_DIR)/jrevdct.o \
		$(OBJECTS_DIR)/jfdctint.o \
		$(OBJECTS_DIR)/jfdctfst.o \
		$(OBJECTS_DIR)/imgresample.o \
		$(OBJECTS_DIR)/imgconvert.o \
		$(OBJECTS_DIR)/h263dec.o \
		$(OBJECTS_DIR)/h263.o \
		$(OBJECTS_DIR)/error_resilience.o \
		$(OBJECTS_DIR)/avcodec_dv.o \
		$(OBJECTS_DIR)/fft.o \
		$(OBJECTS_DIR)/dsputil.o \
		$(OBJECTS_DIR)/eval.o \
		$(OBJECTS_DIR)/common.o \
		$(OBJECTS_DIR)/allcodecs.o \
		$(OBJECTS_DIR)/huffyuv.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	
OBJMOC	=	


####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules


all: $(DESTDIR)$(SYSCONF_LINK_TARGET)

$(DESTDIR)$(SYSCONF_LINK_TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) $(SUBLIBS)
	$(SYSCONF_LINK_LIB)

moc: $(SRCMOC)

tmake:
	tmake libavcodec.pro

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(UICIMPLS) $(UICDECLS)
	-rm -f *~ core
	-rm -f allmoc.cpp

####### Extension Modules

listpromodules:
	@echo 

listallmodules:
	@echo 

listaddonpromodules:
	@echo 

listaddonentmodules:
	@echo 


REQUIRES=

####### Sub-libraries


###### Combined headers



####### Compile

$(OBJECTS_DIR)/wmadec.o: wmadec.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		wmadata.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/wmadec.o wmadec.c

$(OBJECTS_DIR)/avcodec_utils.o: avcodec_utils.c \
		utils.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/avcodec_utils.o avcodec_utils.c

$(OBJECTS_DIR)/svq1.o: svq1.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		avcodec.h \
		dsputil.h \
		mpegvideo.h \
		svq1_cb.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/svq1.o svq1.c

$(OBJECTS_DIR)/simple_idct.o: simple_idct.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		simple_idct.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/simple_idct.o simple_idct.c

$(OBJECTS_DIR)/rv10.o: rv10.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rv10.o rv10.c

$(OBJECTS_DIR)/adpcm.o: adpcm.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/adpcm.o adpcm.c

$(OBJECTS_DIR)/pcm.o: pcm.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pcm.o pcm.c

$(OBJECTS_DIR)/mem.o: mem.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mem.o mem.c

$(OBJECTS_DIR)/mdct.o: mdct.c \
		dsputil.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		avcodec.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mdct.o mdct.c

$(OBJECTS_DIR)/resample.o: resample.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/resample.o resample.c

$(OBJECTS_DIR)/msmpeg4.o: msmpeg4.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h \
		msmpeg4data.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/msmpeg4.o msmpeg4.c

$(OBJECTS_DIR)/mpegvideo.o: mpegvideo.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h \
		simple_idct.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpegvideo.o mpegvideo.c

$(OBJECTS_DIR)/mpegaudio.o: mpegaudio.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		mpegaudio.h \
		mpegaudiotab.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpegaudio.o mpegaudio.c

$(OBJECTS_DIR)/mpegaudiodec.o: mpegaudiodec.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		mpegaudio.h \
		mpegaudiodectab.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpegaudiodec.o mpegaudiodec.c

$(OBJECTS_DIR)/mpeg12.o: mpeg12.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h \
		mpeg12data.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpeg12.o mpeg12.c

$(OBJECTS_DIR)/ratecontrol.o: ratecontrol.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		avcodec.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ratecontrol.o ratecontrol.c

$(OBJECTS_DIR)/mjpeg.o: mjpeg.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mjpeg.o mjpeg.c

$(OBJECTS_DIR)/motion_est.o: motion_est.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/motion_est.o motion_est.c

$(OBJECTS_DIR)/mace.o: mace.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mace.o mace.c

$(OBJECTS_DIR)/jrevdct.o: jrevdct.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		avcodec.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/jrevdct.o jrevdct.c

$(OBJECTS_DIR)/jfdctint.o: jfdctint.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		avcodec.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/jfdctint.o jfdctint.c

$(OBJECTS_DIR)/jfdctfst.o: jfdctfst.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		avcodec.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/jfdctfst.o jfdctfst.c

$(OBJECTS_DIR)/imgresample.o: imgresample.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		i386/mmx.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/imgresample.o imgresample.c

$(OBJECTS_DIR)/imgconvert.o: imgconvert.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		i386/mmx.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/imgconvert.o imgconvert.c

$(OBJECTS_DIR)/h263dec.o: h263dec.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/h263dec.o h263dec.c

$(OBJECTS_DIR)/h263.o: h263.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		avcodec.h \
		mpegvideo.h \
		h263data.h \
		mpeg4data.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/h263.o h263.c

$(OBJECTS_DIR)/error_resilience.o: error_resilience.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/error_resilience.o error_resilience.c

$(OBJECTS_DIR)/avcodec_dv.o: avcodec_dv.c \
		dv.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h \
		mpegvideo.h \
		simple_idct.h \
		dvdata.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/avcodec_dv.o avcodec_dv.c

$(OBJECTS_DIR)/fft.o: fft.c \
		dsputil.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		avcodec.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fft.o fft.c

$(OBJECTS_DIR)/dsputil.o: dsputil.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		dsputil.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dsputil.o dsputil.c

$(OBJECTS_DIR)/eval.o: eval.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/eval.o eval.c

$(OBJECTS_DIR)/common.o: common.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/common.o common.c

$(OBJECTS_DIR)/allcodecs.o: allcodecs.c \
		avcodec.h \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/allcodecs.o allcodecs.c

$(OBJECTS_DIR)/huffyuv.o: huffyuv.c \
		common.h \
		../libavformat/config.h \
		fastmemcpy.h \
		bswap.h \
		avcodec.h \
		dsputil.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/huffyuv.o huffyuv.c



lupdate-check:
	@true

lupdate:
	@true

lrelease:
	@echo "No TRANSLATIONS"


ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic  $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: libavcodec.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/libraries/libavcodec
