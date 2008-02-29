#############################################################################
# Automatically generated from 3rdparty/libraries/libavformat/Makefile.in
# Build options from 
#############################################################################

default: all # 

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -w -O5
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -w -O5
INCPATH =	-I$(MOC_DIR) -I. -I.. -I../libavcodec
LFLAGS	=	$(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS)  $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= avformat
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	
SOURCES =	allformats.c \
		asf.c \
		au.c \
		avidec.c \
		avio.c \
		aviobuf.c \
		crc.c \
		cutils.c \
		ffm.c \
		file.c \
		gif.c \
		http.c \
		img.c \
		jpeg.c \
		mov.c \
		mpeg.c \
		mpegts.c \
		raw.c \
		rm.c \
		rtp.c \
		rtpproto.c \
		rtsp.c \
		swf.c \
		tcp.c \
		udp.c \
		utils.c \
		wav.c \
		avienc.c \
		dv.c
OBJECTS =	$(OBJECTS_DIR)/allformats.o \
		$(OBJECTS_DIR)/asf.o \
		$(OBJECTS_DIR)/au.o \
		$(OBJECTS_DIR)/avidec.o \
		$(OBJECTS_DIR)/avio.o \
		$(OBJECTS_DIR)/aviobuf.o \
		$(OBJECTS_DIR)/crc.o \
		$(OBJECTS_DIR)/cutils.o \
		$(OBJECTS_DIR)/ffm.o \
		$(OBJECTS_DIR)/file.o \
		$(OBJECTS_DIR)/gif.o \
		$(OBJECTS_DIR)/http.o \
		$(OBJECTS_DIR)/img.o \
		$(OBJECTS_DIR)/jpeg.o \
		$(OBJECTS_DIR)/mov.o \
		$(OBJECTS_DIR)/mpeg.o \
		$(OBJECTS_DIR)/mpegts.o \
		$(OBJECTS_DIR)/raw.o \
		$(OBJECTS_DIR)/rm.o \
		$(OBJECTS_DIR)/rtp.o \
		$(OBJECTS_DIR)/rtpproto.o \
		$(OBJECTS_DIR)/rtsp.o \
		$(OBJECTS_DIR)/swf.o \
		$(OBJECTS_DIR)/tcp.o \
		$(OBJECTS_DIR)/udp.o \
		$(OBJECTS_DIR)/utils.o \
		$(OBJECTS_DIR)/wav.o \
		$(OBJECTS_DIR)/avienc.o \
		$(OBJECTS_DIR)/dv.o
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
	tmake libavformat.pro

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

$(OBJECTS_DIR)/allformats.o: allformats.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/allformats.o allformats.c

$(OBJECTS_DIR)/asf.o: asf.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h \
		../libavcodec/mpegaudio.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/asf.o asf.c

$(OBJECTS_DIR)/au.o: au.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/au.o au.c

$(OBJECTS_DIR)/avidec.o: avidec.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/avidec.o avidec.c

$(OBJECTS_DIR)/avio.o: avio.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/avio.o avio.c

$(OBJECTS_DIR)/aviobuf.o: aviobuf.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/aviobuf.o aviobuf.c

$(OBJECTS_DIR)/crc.o: crc.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/crc.o crc.c

$(OBJECTS_DIR)/cutils.o: cutils.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/cutils.o cutils.c

$(OBJECTS_DIR)/ffm.o: ffm.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ffm.o ffm.c

$(OBJECTS_DIR)/file.o: file.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/file.o file.c

$(OBJECTS_DIR)/gif.o: gif.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gif.o gif.c

$(OBJECTS_DIR)/http.o: http.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/http.o http.c

$(OBJECTS_DIR)/img.o: img.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/img.o img.c

$(OBJECTS_DIR)/jpeg.o: jpeg.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/jpeg.o jpeg.c

$(OBJECTS_DIR)/mov.o: mov.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mov.o mov.c

$(OBJECTS_DIR)/mpeg.o: mpeg.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpeg.o mpeg.c

$(OBJECTS_DIR)/mpegts.o: mpegts.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mpegts.o mpegts.c

$(OBJECTS_DIR)/raw.o: raw.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/raw.o raw.c

$(OBJECTS_DIR)/rm.o: rm.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rm.o rm.c

$(OBJECTS_DIR)/rtp.o: rtp.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rtp.o rtp.c

$(OBJECTS_DIR)/rtpproto.o: rtpproto.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rtpproto.o rtpproto.c

$(OBJECTS_DIR)/rtsp.o: rtsp.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rtsp.o rtsp.c

$(OBJECTS_DIR)/swf.o: swf.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/swf.o swf.c

$(OBJECTS_DIR)/tcp.o: tcp.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/tcp.o tcp.c

$(OBJECTS_DIR)/udp.o: udp.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		barpainet.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/udp.o udp.c

$(OBJECTS_DIR)/utils.o: utils.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		strptime.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/utils.o utils.c

$(OBJECTS_DIR)/wav.o: wav.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/wav.o wav.c

$(OBJECTS_DIR)/avienc.o: avienc.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h \
		avi.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/avienc.o avienc.c

$(OBJECTS_DIR)/dv.o: dv.c \
		avformat.h \
		../libavcodec/avcodec.h \
		../libavcodec/common.h \
		config.h \
		../libavcodec/fastmemcpy.h \
		../libavcodec/bswap.h \
		avio.h \
		rtp.h \
		rtsp.h \
		rtspcodes.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dv.o dv.c



lupdate:
	sed -e 's/$$$$QTOPIA1DIR[^ ]*//' 	    -e 's/^static:.*//' 	    <libavformat.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease libavformat.pro
	$(QPEDIR)/bin/qtopia-linstall libavformat.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: libavformat.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/libraries/libavformat
