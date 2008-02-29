#############################################################################
# Automatically generated from applications/mediaplayer/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = applications/mediaplayer/qpe-mpegplayer.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS)
INCPATH =	-I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lpthread $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= mpegplayer
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	controlwidgetbase.h \
		imageutil.h \
		documentlist.h \
		loopcontrol.h \
		mediaplayerplugininterface.h \
		playlistselection.h \
		id3tag.h \
		action.h \
		fileinfo.h \
		tabbedselector.h \
		mediaplayerstate.h \
		videowidget.h \
		audiowidget.h \
		playlistwidget.h \
		mediaplayer.h \
		audiodevice.h
SOURCES =	controlwidgetbase.cpp \
		imageutil.cpp \
		documentlist.cpp \
		main.cpp \
		loopcontrol.cpp \
		playlistselection.cpp \
		mediaplayerstate.cpp \
		id3tag.cpp \
		fileinfo.cpp \
		tabbedselector.cpp \
		videowidget.cpp \
		audiowidget.cpp \
		playlistwidget.cpp \
		mediaplayer.cpp \
		audiodevice.cpp
OBJECTS =	controlwidgetbase.o \
		imageutil.o \
		documentlist.o \
		main.o \
		loopcontrol.o \
		playlistselection.o \
		mediaplayerstate.o \
		id3tag.o \
		fileinfo.o \
		tabbedselector.o \
		videowidget.o \
		audiowidget.o \
		playlistwidget.o \
		mediaplayer.o \
		audiodevice.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	moc_controlwidgetbase.cpp \
		moc_documentlist.cpp \
		moc_loopcontrol.cpp \
		moc_playlistselection.cpp \
		moc_action.cpp \
		moc_fileinfo.cpp \
		moc_tabbedselector.cpp \
		moc_mediaplayerstate.cpp \
		moc_videowidget.cpp \
		moc_audiowidget.cpp \
		moc_playlistwidget.cpp \
		moc_mediaplayer.cpp \
		moc_audiodevice.cpp
OBJMOC	=	moc_controlwidgetbase.o \
		moc_documentlist.o \
		moc_loopcontrol.o \
		moc_playlistselection.o \
		moc_action.o \
		moc_fileinfo.o \
		moc_tabbedselector.o \
		moc_mediaplayerstate.o \
		moc_videowidget.o \
		moc_audiowidget.o \
		moc_playlistwidget.o \
		moc_mediaplayer.o \
		moc_audiodevice.o


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
	tmake mediaplayer.pro

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

controlwidgetbase.o: controlwidgetbase.cpp \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		imageutil.h \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

imageutil.o: imageutil.cpp \
		imageutil.h

documentlist.o: documentlist.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/storage.h \
		playlistselection.h \
		documentlist.h

main.o: main.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		mediaplayerstate.h \
		playlistwidget.h \
		$(QPEDIR)/include/qpe/applnk.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		videowidget.h \
		loopcontrol.h \
		mediaplayer.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h

loopcontrol.o: loopcontrol.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		loopcontrol.h \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		audiodevice.h

playlistselection.o: playlistselection.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/storage.h \
		$(QPEDIR)/include/qpe/categoryselect.h \
		mediaplayerstate.h \
		playlistselection.h \
		fileinfo.h \
		id3tag.h

mediaplayerstate.o: mediaplayerstate.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h \
		audiodevice.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		videowidget.h

id3tag.o: id3tag.cpp \
		id3tag.h

fileinfo.o: fileinfo.cpp \
		fileinfo.h \
		$(QPEDIR)/include/qpe/applnk.h \
		id3tag.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		mediaplayerstate.h \
		$(QPEDIR)/include/qpe/lnkproperties.h

tabbedselector.o: tabbedselector.cpp \
		$(QPEDIR)/include/qpe/fileselector.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/categoryselect.h \
		tabbedselector.h \
		$(QPEDIR)/include/qpe/applnk.h \
		playlistselection.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		documentlist.h \
		$(QPEDIR)/include/qpe/storage.h

videowidget.o: videowidget.cpp \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		imageutil.h \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

audiowidget.o: audiowidget.cpp \
		$(QPEDIR)/include/qpe/resource.h \
		imageutil.h \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

playlistwidget.o: playlistwidget.cpp \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		$(QPEDIR)/include/qpe/fileselector.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/resource.h \
		playlistselection.h \
		playlistwidget.h \
		mediaplayerstate.h \
		tabbedselector.h \
		action.h \
		fileinfo.h \
		id3tag.h

mediaplayer.o: mediaplayer.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/config.h \
		mediaplayer.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		playlistwidget.h \
		$(QPEDIR)/include/qpe/applnk.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		loopcontrol.h \
		audiodevice.h

audiodevice.o: audiodevice.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/config.h \
		audiodevice.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

moc_controlwidgetbase.o: moc_controlwidgetbase.cpp \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

moc_documentlist.o: moc_documentlist.cpp \
		documentlist.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/storage.h

moc_loopcontrol.o: moc_loopcontrol.cpp \
		loopcontrol.h

moc_playlistselection.o: moc_playlistselection.cpp \
		playlistselection.h \
		$(QPEDIR)/include/qpe/applnk.h

moc_action.o: moc_action.cpp \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

moc_fileinfo.o: moc_fileinfo.cpp \
		fileinfo.h \
		$(QPEDIR)/include/qpe/applnk.h \
		id3tag.h

moc_tabbedselector.o: moc_tabbedselector.cpp \
		tabbedselector.h \
		$(QPEDIR)/include/qpe/applnk.h

moc_mediaplayerstate.o: moc_mediaplayerstate.cpp \
		mediaplayerstate.h

moc_videowidget.o: moc_videowidget.cpp \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

moc_audiowidget.o: moc_audiowidget.cpp \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

moc_playlistwidget.o: moc_playlistwidget.cpp \
		playlistwidget.h \
		$(QPEDIR)/include/qpe/applnk.h \
		mediaplayerstate.h

moc_mediaplayer.o: moc_mediaplayer.cpp \
		mediaplayer.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/mediaplayerplugininterface.h \
		$(QPEDIR)/include/qpe/qcom.h

moc_audiodevice.o: moc_audiodevice.cpp \
		audiodevice.h

moc_controlwidgetbase.cpp: controlwidgetbase.h
	$(MOC) controlwidgetbase.h -o moc_controlwidgetbase.cpp

moc_documentlist.cpp: documentlist.h
	$(MOC) documentlist.h -o moc_documentlist.cpp

moc_loopcontrol.cpp: loopcontrol.h
	$(MOC) loopcontrol.h -o moc_loopcontrol.cpp

moc_playlistselection.cpp: playlistselection.h
	$(MOC) playlistselection.h -o moc_playlistselection.cpp

moc_action.cpp: action.h
	$(MOC) action.h -o moc_action.cpp

moc_fileinfo.cpp: fileinfo.h
	$(MOC) fileinfo.h -o moc_fileinfo.cpp

moc_tabbedselector.cpp: tabbedselector.h
	$(MOC) tabbedselector.h -o moc_tabbedselector.cpp

moc_mediaplayerstate.cpp: mediaplayerstate.h
	$(MOC) mediaplayerstate.h -o moc_mediaplayerstate.cpp

moc_videowidget.cpp: videowidget.h
	$(MOC) videowidget.h -o moc_videowidget.cpp

moc_audiowidget.cpp: audiowidget.h
	$(MOC) audiowidget.h -o moc_audiowidget.cpp

moc_playlistwidget.cpp: playlistwidget.h
	$(MOC) playlistwidget.h -o moc_playlistwidget.cpp

moc_mediaplayer.cpp: mediaplayer.h
	$(MOC) mediaplayer.h -o moc_mediaplayer.cpp

moc_audiodevice.cpp: audiodevice.h
	$(MOC) audiodevice.h -o moc_audiodevice.cpp



lupdate:
	lupdate mediaplayer.pro

lrelease:
	lrelease mediaplayer.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: mediaplayer.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/mediaplayer
