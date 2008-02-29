#############################################################################
# Automatically generated from 3rdparty/plugins/codecs/libmad/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/plugins/codecs/libmad/qpe-libmadplugin.control

# Compiling
INTERFACE_DECL_PATH 	= .
SYSCONF_CXX		= arm-linux-g++
SYSCONF_CC		= arm-linux-gcc
DASHCROSS		= -arm

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
SYSCONF_LINK		= arm-linux-gcc
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
SYSCONF_LINK_SHLIB	= arm-linux-gcc
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
SYSCONF_AR		= arm-linux-ar cqs
SYSCONF_LINK_TARGET_STATIC = lib$(TARGET).a
SYSCONF_LINK_LIB_STATIC	= rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) ; \
				 $(SYSCONF_AR) $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS) $(OBJMOC) 
# Compiling application source
SYSCONF_CXXFLAGS	= -pipe -DQT_QWS_SL5XXX -DQT_QWS_CUSTOM -DQWS -fno-exceptions -fno-rtti -O2 -Wall -W -DNO_DEBUG
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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DQCONFIG=\"qconfig-qpe.h\" -DFPM_64BIT
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DQCONFIG=\"qconfig-qpe.h\" -DFPM_64BIT
INCPATH =	-I.moc/linux-sharp-g++ -I$(QPEDIR)/include -I..
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lm $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/codecs/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= madplugin
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	libmad_version.h \
		fixed.h \
		bit.h \
		timer.h \
		stream.h \
		frame.h \
		synth.h \
		decoder.h \
		layer12.h \
		layer3.h \
		huffman.h \
		libmad_global.h \
		mad.h \
		libmadplugin.h \
		libmadpluginimpl.h
SOURCES =	version.c \
		fixed.c \
		bit.c \
		timer.c \
		stream.c \
		frame.c \
		synth.c \
		decoder.c \
		layer12.c \
		layer3.c \
		huffman.c \
		libmadplugin.cpp \
		libmadpluginimpl.cpp
OBJECTS =	.obj/linux-sharp-g++/version.o \
		.obj/linux-sharp-g++/fixed.o \
		.obj/linux-sharp-g++/bit.o \
		.obj/linux-sharp-g++/timer.o \
		.obj/linux-sharp-g++/stream.o \
		.obj/linux-sharp-g++/frame.o \
		.obj/linux-sharp-g++/synth.o \
		.obj/linux-sharp-g++/decoder.o \
		.obj/linux-sharp-g++/layer12.o \
		.obj/linux-sharp-g++/layer3.o \
		.obj/linux-sharp-g++/huffman.o \
		.obj/linux-sharp-g++/libmadplugin.o \
		.obj/linux-sharp-g++/libmadpluginimpl.o
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
	tmake libmad.pro

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

.obj/linux-sharp-g++/version.o: version.c \
		libmad_global.h \
		libmad_version.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/version.o version.c

.obj/linux-sharp-g++/fixed.o: fixed.c \
		libmad_global.h \
		fixed.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/fixed.o fixed.c

.obj/linux-sharp-g++/bit.o: bit.c \
		libmad_global.h \
		bit.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/bit.o bit.c

.obj/linux-sharp-g++/timer.o: timer.c \
		libmad_global.h \
		timer.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/timer.o timer.c

.obj/linux-sharp-g++/stream.o: stream.c \
		libmad_global.h \
		bit.h \
		stream.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/stream.o stream.c

.obj/linux-sharp-g++/frame.o: frame.c \
		libmad_global.h \
		bit.h \
		stream.h \
		frame.h \
		fixed.h \
		timer.h \
		layer12.h \
		layer3.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/frame.o frame.c

.obj/linux-sharp-g++/synth.o: synth.c \
		libmad_global.h \
		fixed.h \
		frame.h \
		timer.h \
		stream.h \
		bit.h \
		synth.h \
		D.dat
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/synth.o synth.c

.obj/linux-sharp-g++/decoder.o: decoder.c \
		libmad_global.h \
		stream.h \
		bit.h \
		frame.h \
		fixed.h \
		timer.h \
		synth.h \
		decoder.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/decoder.o decoder.c

.obj/linux-sharp-g++/layer12.o: layer12.c \
		libmad_global.h \
		fixed.h \
		bit.h \
		stream.h \
		frame.h \
		timer.h \
		layer12.h \
		sf_table.dat \
		qc_table.dat
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/layer12.o layer12.c

.obj/linux-sharp-g++/layer3.o: layer3.c \
		libmad_global.h \
		fixed.h \
		bit.h \
		stream.h \
		frame.h \
		timer.h \
		huffman.h \
		layer3.h \
		rq_table.dat \
		imdct_s.dat
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/layer3.o layer3.c

.obj/linux-sharp-g++/huffman.o: huffman.c \
		libmad_global.h \
		huffman.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/huffman.o huffman.c

.obj/linux-sharp-g++/libmadplugin.o: libmadplugin.cpp \
		libmadplugin.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		mad.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/libmadplugin.o libmadplugin.cpp

.obj/linux-sharp-g++/libmadpluginimpl.o: libmadpluginimpl.cpp \
		libmadplugin.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		libmadpluginimpl.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/linux-sharp-g++/libmadpluginimpl.o libmadpluginimpl.cpp



lupdate:
	lupdate libmad.pro

lrelease:
	lrelease libmad.pro
	$(QPEDIR)/bin/qtopia-linstall libmad.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: libmad.pro
	cd $(QPEDIR)/src; 	./configure  -qconfig qpe -platform sharp -make 3rdparty/plugins/codecs/libmad
