#############################################################################
# Automatically generated from 3rdparty/plugins/codecs/libmpeg3/Makefile.in
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
SYSCONF_LIBS_QTOPIA	= -lqpe
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
#############################################################################

####### Compiler, tools and options

CXX	=	$(SYSCONF_CXX) $(QT_CXX_MT)
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB)
INCPATH =	-I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lpthread -lm $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/codecs/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= mpeg3plugin
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	libmpeg3plugin.h \
		libmpeg3pluginimpl.h
SOURCES =	libmpeg3plugin.cpp \
		libmpeg3pluginimpl.cpp \
		bitstream.c \
		libmpeg3.c \
		mpeg3atrack.c \
		mpeg3css.c \
		mpeg3demux.c \
		mpeg3io.c \
		mpeg3title.c \
		mpeg3vtrack.c \
		audio/ac3.c \
		audio/bit_allocation.c \
		audio/dct.c \
		audio/exponents.c \
		audio/header.c \
		audio/layer2.c \
		audio/layer3.c \
		audio/mantissa.c \
		audio/mpeg3audio.c \
		audio/pcm.c \
		audio/synthesizers.c \
		audio/tables.c \
		video/getpicture.c \
		video/headers.c \
		video/idct.c \
		video/macroblocks.c \
		video/mmxtest.c \
		video/motion.c \
		video/mpeg3video.c \
		video/output.c \
		video/reconstruct.c \
		video/seek.c \
		video/slice.c \
		video/vlc.c
OBJECTS =	libmpeg3plugin.o \
		libmpeg3pluginimpl.o \
		bitstream.o \
		libmpeg3.o \
		mpeg3atrack.o \
		mpeg3css.o \
		mpeg3demux.o \
		mpeg3io.o \
		mpeg3title.o \
		mpeg3vtrack.o \
		audio/ac3.o \
		audio/bit_allocation.o \
		audio/dct.o \
		audio/exponents.o \
		audio/header.o \
		audio/layer2.o \
		audio/layer3.o \
		audio/mantissa.o \
		audio/mpeg3audio.o \
		audio/pcm.o \
		audio/synthesizers.o \
		audio/tables.o \
		video/getpicture.o \
		video/headers.o \
		video/idct.o \
		video/macroblocks.o \
		video/mmxtest.o \
		video/motion.o \
		video/mpeg3video.o \
		video/output.o \
		video/reconstruct.o \
		video/seek.o \
		video/slice.o \
		video/vlc.o
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
	$(CXX) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules


all: $(DESTDIR)$(SYSCONF_LINK_TARGET)

$(DESTDIR)$(SYSCONF_LINK_TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) $(SUBLIBS)
	$(SYSCONF_LINK_LIB)

moc: $(SRCMOC)

tmake:
	tmake libmpeg3.pro

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

libmpeg3plugin.o: libmpeg3plugin.cpp \
		libmpeg3plugin.h \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h

libmpeg3pluginimpl.o: libmpeg3pluginimpl.cpp \
		libmpeg3plugin.h \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		libmpeg3pluginimpl.h

bitstream.o: bitstream.c \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

libmpeg3.o: libmpeg3.c \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

mpeg3atrack.o: mpeg3atrack.c \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

mpeg3css.o: mpeg3css.c \
		mpeg3css.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h

mpeg3demux.o: mpeg3demux.c \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

mpeg3io.o: mpeg3io.c \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

mpeg3title.o: mpeg3title.c \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

mpeg3vtrack.o: mpeg3vtrack.c \
		libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		mpeg3protos.h

audio/ac3.o: audio/ac3.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/bit_allocation.o: audio/bit_allocation.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/dct.o: audio/dct.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/tables.h \
		audio/fptables.h

audio/exponents.o: audio/exponents.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/header.o: audio/header.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/tables.h \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/layer2.o: audio/layer2.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/tables.h

audio/layer3.o: audio/layer3.c \
		audio/huffman.h \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/tables.h

audio/mantissa.o: audio/mantissa.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/mpeg3audio.o: audio/mpeg3audio.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/mpeg3audio.h \
		audio/tables.h

audio/pcm.o: audio/pcm.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h

audio/synthesizers.o: audio/synthesizers.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/tables.h

audio/tables.o: audio/tables.c \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		audio/../mpeg3protos.h \
		audio/tables.h \
		audio/fptables.h

video/getpicture.o: video/getpicture.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h \
		video/vlc.h

video/headers.o: video/headers.c \
		video/../mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h

video/idct.o: video/idct.c \
		video/idct.h

video/macroblocks.o: video/macroblocks.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h \
		video/vlc.h

video/mmxtest.o: video/mmxtest.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h

video/motion.o: video/motion.c \
		video/mpeg3video.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3protos.h \
		video/vlc.h

video/mpeg3video.o: video/mpeg3video.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h \
		video/mpeg3videoprotos.h

video/output.o: video/output.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h

video/reconstruct.o: video/reconstruct.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h

video/seek.o: video/seek.c \
		video/../mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h

video/slice.o: video/slice.c \
		audio/../libmpeg3.h \
		mpeg3private.h \
		mpeg3atrack.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		audio/mpeg3audio.h \
		audio/ac3.h \
		audio/mpeg3real.h \
		audio/../bitstream.h \
		mpeg3vtrack.h \
		video/mpeg3video.h \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/../mpeg3protos.h \
		video/mpeg3video.h \
		video/mpeg3videoprotos.h

video/vlc.o: video/vlc.c \
		video/mpeg3video.h \
		audio/../bitstream.h \
		mpeg3demux.h \
		mpeg3title.h \
		mpeg3io.h \
		mpeg3css.h \
		mpeg3private.inc \
		video/../mpeg3private.inc \
		video/idct.h \
		video/slice.h \
		video/../timecode.h \
		video/vlc.h



lupdate:
	lupdate libmpeg3.pro

lrelease:
	lrelease libmpeg3.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: libmpeg3.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/plugins/codecs/libmpeg3
