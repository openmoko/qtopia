#############################################################################
# Automatically generated from applications/mediarecorder/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = applications/mediarecorder/qpe-mediarecorder.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS)
INCPATH =	-I$(MOC_DIR)
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lpthread -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= mediarecorder
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	audiodevice.h \
		audioinput.h \
		mediarecorder.h \
		pluginlist.h \
		samplebuffer.h \
		timeprogressbar.h \
		confrecorder.h \
		waveform.h
SOURCES =	audiodevice.cpp \
		audioinput.cpp \
		main.cpp \
		mediarecorder.cpp \
		pluginlist.cpp \
		samplebuffer.cpp \
		timeprogressbar.cpp \
		confrecorder.cpp \
		waveform.cpp
OBJECTS =	$(OBJECTS_DIR)/audiodevice.o \
		$(OBJECTS_DIR)/audioinput.o \
		$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/mediarecorder.o \
		$(OBJECTS_DIR)/pluginlist.o \
		$(OBJECTS_DIR)/samplebuffer.o \
		$(OBJECTS_DIR)/timeprogressbar.o \
		$(OBJECTS_DIR)/confrecorder.o \
		$(OBJECTS_DIR)/waveform.o \
		$(OBJECTS_DIR)/mediarecorderbase.o \
		$(OBJECTS_DIR)/confrecorderbase.o
INTERFACES =    mediarecorderbase.ui \
		confrecorderbase.ui
UICDECLS =      mediarecorderbase.h \
		confrecorderbase.h
UICIMPLS =      mediarecorderbase.cpp \
		confrecorderbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_audiodevice.cpp \
		$(MOC_DIR)/moc_audioinput.cpp \
		$(MOC_DIR)/moc_mediarecorder.cpp \
		$(MOC_DIR)/moc_timeprogressbar.cpp \
		$(MOC_DIR)/moc_confrecorder.cpp \
		$(MOC_DIR)/moc_mediarecorderbase.cpp \
		$(MOC_DIR)/moc_confrecorderbase.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_audiodevice.o \
		$(OBJECTS_DIR)/moc_audioinput.o \
		$(OBJECTS_DIR)/moc_mediarecorder.o \
		$(OBJECTS_DIR)/moc_timeprogressbar.o \
		$(OBJECTS_DIR)/moc_confrecorder.o \
		$(OBJECTS_DIR)/moc_mediarecorderbase.o \
		$(OBJECTS_DIR)/moc_confrecorderbase.o


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


all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) $(SUBLIBS)
	$(SYSCONF_LINK) $(LFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

tmake:
	tmake mediarecorder.pro

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

$(OBJECTS_DIR)/audiodevice.o: audiodevice.cpp \
		audiodevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/audiodevice.o audiodevice.cpp

$(OBJECTS_DIR)/audioinput.o: audioinput.cpp \
		audioinput.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/audioinput.o audioinput.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		mediarecorder.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

$(OBJECTS_DIR)/mediarecorder.o: mediarecorder.cpp \
		mediarecorder.h \
		mediarecorderbase.h \
		audioinput.h \
		audiodevice.h \
		samplebuffer.h \
		pluginlist.h \
		timeprogressbar.h \
		confrecorder.h \
		confrecorderbase.h \
		waveform.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mediarecorder.o mediarecorder.cpp

$(OBJECTS_DIR)/pluginlist.o: pluginlist.cpp \
		pluginlist.h \
		../../plugins/codecs/wavrecord/wavrecordimpl.h \
		../../plugins/codecs/wavplugin/wavpluginimpl.h \
		../../3rdparty/plugins/codecs/libffmpeg/libffmpegpluginimpl.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginlist.o pluginlist.cpp

$(OBJECTS_DIR)/samplebuffer.o: samplebuffer.cpp \
		samplebuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/samplebuffer.o samplebuffer.cpp

$(OBJECTS_DIR)/timeprogressbar.o: timeprogressbar.cpp \
		timeprogressbar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timeprogressbar.o timeprogressbar.cpp

$(OBJECTS_DIR)/confrecorder.o: confrecorder.cpp \
		confrecorder.h \
		confrecorderbase.h \
		mediarecorder.h \
		pluginlist.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/confrecorder.o confrecorder.cpp

$(OBJECTS_DIR)/waveform.o: waveform.cpp \
		waveform.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/waveform.o waveform.cpp

mediarecorderbase.h: mediarecorderbase.ui
	$(UIC) mediarecorderbase.ui -o $(INTERFACE_DECL_PATH)/mediarecorderbase.h

mediarecorderbase.cpp: mediarecorderbase.ui
	$(UIC) mediarecorderbase.ui -i mediarecorderbase.h -o mediarecorderbase.cpp

confrecorderbase.h: confrecorderbase.ui
	$(UIC) confrecorderbase.ui -o $(INTERFACE_DECL_PATH)/confrecorderbase.h

confrecorderbase.cpp: confrecorderbase.ui
	$(UIC) confrecorderbase.ui -i confrecorderbase.h -o confrecorderbase.cpp

$(OBJECTS_DIR)/mediarecorderbase.o: mediarecorderbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mediarecorderbase.o mediarecorderbase.cpp

$(OBJECTS_DIR)/confrecorderbase.o: confrecorderbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/confrecorderbase.o confrecorderbase.cpp

$(OBJECTS_DIR)/moc_audiodevice.o: $(MOC_DIR)/moc_audiodevice.cpp \
		audiodevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_audiodevice.o $(MOC_DIR)/moc_audiodevice.cpp

$(OBJECTS_DIR)/moc_audioinput.o: $(MOC_DIR)/moc_audioinput.cpp \
		audioinput.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_audioinput.o $(MOC_DIR)/moc_audioinput.cpp

$(OBJECTS_DIR)/moc_mediarecorder.o: $(MOC_DIR)/moc_mediarecorder.cpp \
		mediarecorder.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_mediarecorder.o $(MOC_DIR)/moc_mediarecorder.cpp

$(OBJECTS_DIR)/moc_timeprogressbar.o: $(MOC_DIR)/moc_timeprogressbar.cpp \
		timeprogressbar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_timeprogressbar.o $(MOC_DIR)/moc_timeprogressbar.cpp

$(OBJECTS_DIR)/moc_confrecorder.o: $(MOC_DIR)/moc_confrecorder.cpp \
		confrecorder.h \
		confrecorderbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_confrecorder.o $(MOC_DIR)/moc_confrecorder.cpp

$(OBJECTS_DIR)/moc_mediarecorderbase.o: $(MOC_DIR)/moc_mediarecorderbase.cpp \
		mediarecorderbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_mediarecorderbase.o $(MOC_DIR)/moc_mediarecorderbase.cpp

$(OBJECTS_DIR)/moc_confrecorderbase.o: $(MOC_DIR)/moc_confrecorderbase.cpp \
		confrecorderbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_confrecorderbase.o $(MOC_DIR)/moc_confrecorderbase.cpp

$(MOC_DIR)/moc_audiodevice.cpp: audiodevice.h
	$(MOC) audiodevice.h -o $(MOC_DIR)/moc_audiodevice.cpp

$(MOC_DIR)/moc_audioinput.cpp: audioinput.h
	$(MOC) audioinput.h -o $(MOC_DIR)/moc_audioinput.cpp

$(MOC_DIR)/moc_mediarecorder.cpp: mediarecorder.h
	$(MOC) mediarecorder.h -o $(MOC_DIR)/moc_mediarecorder.cpp

$(MOC_DIR)/moc_timeprogressbar.cpp: timeprogressbar.h
	$(MOC) timeprogressbar.h -o $(MOC_DIR)/moc_timeprogressbar.cpp

$(MOC_DIR)/moc_confrecorder.cpp: confrecorder.h
	$(MOC) confrecorder.h -o $(MOC_DIR)/moc_confrecorder.cpp

$(MOC_DIR)/moc_mediarecorderbase.cpp: mediarecorderbase.h
	$(MOC) mediarecorderbase.h -o $(MOC_DIR)/moc_mediarecorderbase.cpp

$(MOC_DIR)/moc_confrecorderbase.cpp: confrecorderbase.h
	$(MOC) confrecorderbase.h -o $(MOC_DIR)/moc_confrecorderbase.cpp



lupdate:
	sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' 	    -e '/$$(.*/ d' 	    -e 's/^static:.*//' 	    <mediarecorder.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease mediarecorder.pro
	$(QPEDIR)/bin/qtopia-linstall mediarecorder.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: mediarecorder.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/mediarecorder
