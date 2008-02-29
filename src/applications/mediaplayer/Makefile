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
TARGET	= mpegplayer
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	maindocumentwidgetstack.h \
		controlwidgetbase.h \
		imageutil.h \
		documentlist.h \
		loopcontrol.h \
		playlistselection.h \
		framebuffer.h \
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
SOURCES =	maindocumentwidgetstack.cpp \
		controlwidgetbase.cpp \
		imageutil.cpp \
		documentlist.cpp \
		framebuffer.cpp \
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
		audiodevice.cpp \
		main.cpp
OBJECTS =	$(OBJECTS_DIR)/maindocumentwidgetstack.o \
		$(OBJECTS_DIR)/controlwidgetbase.o \
		$(OBJECTS_DIR)/imageutil.o \
		$(OBJECTS_DIR)/documentlist.o \
		$(OBJECTS_DIR)/framebuffer.o \
		$(OBJECTS_DIR)/loopcontrol.o \
		$(OBJECTS_DIR)/playlistselection.o \
		$(OBJECTS_DIR)/mediaplayerstate.o \
		$(OBJECTS_DIR)/id3tag.o \
		$(OBJECTS_DIR)/fileinfo.o \
		$(OBJECTS_DIR)/tabbedselector.o \
		$(OBJECTS_DIR)/videowidget.o \
		$(OBJECTS_DIR)/audiowidget.o \
		$(OBJECTS_DIR)/playlistwidget.o \
		$(OBJECTS_DIR)/mediaplayer.o \
		$(OBJECTS_DIR)/audiodevice.o \
		$(OBJECTS_DIR)/main.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	$(MOC_DIR)/moc_maindocumentwidgetstack.cpp \
		$(MOC_DIR)/moc_controlwidgetbase.cpp \
		$(MOC_DIR)/moc_documentlist.cpp \
		$(MOC_DIR)/moc_loopcontrol.cpp \
		$(MOC_DIR)/moc_playlistselection.cpp \
		$(MOC_DIR)/moc_id3tag.cpp \
		$(MOC_DIR)/moc_action.cpp \
		$(MOC_DIR)/moc_fileinfo.cpp \
		$(MOC_DIR)/moc_tabbedselector.cpp \
		$(MOC_DIR)/moc_mediaplayerstate.cpp \
		$(MOC_DIR)/moc_videowidget.cpp \
		$(MOC_DIR)/moc_audiowidget.cpp \
		$(MOC_DIR)/moc_playlistwidget.cpp \
		$(MOC_DIR)/moc_mediaplayer.cpp \
		$(MOC_DIR)/moc_audiodevice.cpp \
		$(MOC_DIR)/fileinfo.moc
OBJMOC	=	$(OBJECTS_DIR)/moc_maindocumentwidgetstack.o \
		$(OBJECTS_DIR)/moc_controlwidgetbase.o \
		$(OBJECTS_DIR)/moc_documentlist.o \
		$(OBJECTS_DIR)/moc_loopcontrol.o \
		$(OBJECTS_DIR)/moc_playlistselection.o \
		$(OBJECTS_DIR)/moc_id3tag.o \
		$(OBJECTS_DIR)/moc_action.o \
		$(OBJECTS_DIR)/moc_fileinfo.o \
		$(OBJECTS_DIR)/moc_tabbedselector.o \
		$(OBJECTS_DIR)/moc_mediaplayerstate.o \
		$(OBJECTS_DIR)/moc_videowidget.o \
		$(OBJECTS_DIR)/moc_audiowidget.o \
		$(OBJECTS_DIR)/moc_playlistwidget.o \
		$(OBJECTS_DIR)/moc_mediaplayer.o \
		$(OBJECTS_DIR)/moc_audiodevice.o


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

$(OBJECTS_DIR)/maindocumentwidgetstack.o: maindocumentwidgetstack.cpp \
		mediaplayerstate.h \
		playlistwidget.h \
		loopcontrol.h \
		mediaplayer.h \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/maindocumentwidgetstack.o maindocumentwidgetstack.cpp

$(OBJECTS_DIR)/controlwidgetbase.o: controlwidgetbase.cpp \
		imageutil.h \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/controlwidgetbase.o controlwidgetbase.cpp

$(OBJECTS_DIR)/imageutil.o: imageutil.cpp \
		imageutil.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/imageutil.o imageutil.cpp

$(OBJECTS_DIR)/documentlist.o: documentlist.cpp \
		playlistselection.h \
		documentlist.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/documentlist.o documentlist.cpp

$(OBJECTS_DIR)/framebuffer.o: framebuffer.cpp \
		framebuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/framebuffer.o framebuffer.cpp

$(OBJECTS_DIR)/loopcontrol.o: loopcontrol.cpp \
		loopcontrol.h \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		framebuffer.h \
		audiodevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/loopcontrol.o loopcontrol.cpp

$(OBJECTS_DIR)/playlistselection.o: playlistselection.cpp \
		mediaplayerstate.h \
		playlistselection.h \
		fileinfo.h \
		id3tag.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/playlistselection.o playlistselection.cpp

$(OBJECTS_DIR)/mediaplayerstate.o: mediaplayerstate.cpp \
		audiodevice.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		videowidget.h \
		framebuffer.h \
		maindocumentwidgetstack.h \
		../../3rdparty/plugins/codecs/libffmpeg/libffmpegpluginimpl.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mediaplayerstate.o mediaplayerstate.cpp

$(OBJECTS_DIR)/id3tag.o: id3tag.cpp \
		id3tag.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/id3tag.o id3tag.cpp

$(OBJECTS_DIR)/fileinfo.o: fileinfo.cpp \
		$(MOC_DIR)/fileinfo.moc \
		fileinfo.h \
		id3tag.h \
		mediaplayerstate.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fileinfo.o fileinfo.cpp

$(OBJECTS_DIR)/tabbedselector.o: tabbedselector.cpp \
		tabbedselector.h \
		playlistselection.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		documentlist.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/tabbedselector.o tabbedselector.cpp

$(OBJECTS_DIR)/videowidget.o: videowidget.cpp \
		imageutil.h \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		framebuffer.h \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/videowidget.o videowidget.cpp

$(OBJECTS_DIR)/audiowidget.o: audiowidget.cpp \
		imageutil.h \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/audiowidget.o audiowidget.cpp

$(OBJECTS_DIR)/playlistwidget.o: playlistwidget.cpp \
		playlistselection.h \
		playlistwidget.h \
		mediaplayerstate.h \
		tabbedselector.h \
		action.h \
		fileinfo.h \
		id3tag.h \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/playlistwidget.o playlistwidget.cpp

$(OBJECTS_DIR)/mediaplayer.o: mediaplayer.cpp \
		mediaplayer.h \
		playlistwidget.h \
		mediaplayerstate.h \
		audiowidget.h \
		controlwidgetbase.h \
		action.h \
		loopcontrol.h \
		audiodevice.h \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mediaplayer.o mediaplayer.cpp

$(OBJECTS_DIR)/audiodevice.o: audiodevice.cpp \
		audiodevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/audiodevice.o audiodevice.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

$(OBJECTS_DIR)/moc_maindocumentwidgetstack.o: $(MOC_DIR)/moc_maindocumentwidgetstack.cpp \
		maindocumentwidgetstack.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_maindocumentwidgetstack.o $(MOC_DIR)/moc_maindocumentwidgetstack.cpp

$(OBJECTS_DIR)/moc_controlwidgetbase.o: $(MOC_DIR)/moc_controlwidgetbase.cpp \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_controlwidgetbase.o $(MOC_DIR)/moc_controlwidgetbase.cpp

$(OBJECTS_DIR)/moc_documentlist.o: $(MOC_DIR)/moc_documentlist.cpp \
		documentlist.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_documentlist.o $(MOC_DIR)/moc_documentlist.cpp

$(OBJECTS_DIR)/moc_loopcontrol.o: $(MOC_DIR)/moc_loopcontrol.cpp \
		loopcontrol.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_loopcontrol.o $(MOC_DIR)/moc_loopcontrol.cpp

$(OBJECTS_DIR)/moc_playlistselection.o: $(MOC_DIR)/moc_playlistselection.cpp \
		playlistselection.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_playlistselection.o $(MOC_DIR)/moc_playlistselection.cpp

$(OBJECTS_DIR)/moc_id3tag.o: $(MOC_DIR)/moc_id3tag.cpp \
		id3tag.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_id3tag.o $(MOC_DIR)/moc_id3tag.cpp

$(OBJECTS_DIR)/moc_action.o: $(MOC_DIR)/moc_action.cpp \
		action.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_action.o $(MOC_DIR)/moc_action.cpp

$(OBJECTS_DIR)/moc_fileinfo.o: $(MOC_DIR)/moc_fileinfo.cpp \
		fileinfo.h \
		id3tag.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_fileinfo.o $(MOC_DIR)/moc_fileinfo.cpp

$(OBJECTS_DIR)/moc_tabbedselector.o: $(MOC_DIR)/moc_tabbedselector.cpp \
		tabbedselector.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_tabbedselector.o $(MOC_DIR)/moc_tabbedselector.cpp

$(OBJECTS_DIR)/moc_mediaplayerstate.o: $(MOC_DIR)/moc_mediaplayerstate.cpp \
		mediaplayerstate.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_mediaplayerstate.o $(MOC_DIR)/moc_mediaplayerstate.cpp

$(OBJECTS_DIR)/moc_videowidget.o: $(MOC_DIR)/moc_videowidget.cpp \
		videowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h \
		framebuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_videowidget.o $(MOC_DIR)/moc_videowidget.cpp

$(OBJECTS_DIR)/moc_audiowidget.o: $(MOC_DIR)/moc_audiowidget.cpp \
		audiowidget.h \
		controlwidgetbase.h \
		mediaplayerstate.h \
		action.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_audiowidget.o $(MOC_DIR)/moc_audiowidget.cpp

$(OBJECTS_DIR)/moc_playlistwidget.o: $(MOC_DIR)/moc_playlistwidget.cpp \
		playlistwidget.h \
		mediaplayerstate.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_playlistwidget.o $(MOC_DIR)/moc_playlistwidget.cpp

$(OBJECTS_DIR)/moc_mediaplayer.o: $(MOC_DIR)/moc_mediaplayer.cpp \
		mediaplayer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_mediaplayer.o $(MOC_DIR)/moc_mediaplayer.cpp

$(OBJECTS_DIR)/moc_audiodevice.o: $(MOC_DIR)/moc_audiodevice.cpp \
		audiodevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_audiodevice.o $(MOC_DIR)/moc_audiodevice.cpp

$(MOC_DIR)/moc_maindocumentwidgetstack.cpp: maindocumentwidgetstack.h
	$(MOC) maindocumentwidgetstack.h -o $(MOC_DIR)/moc_maindocumentwidgetstack.cpp

$(MOC_DIR)/moc_controlwidgetbase.cpp: controlwidgetbase.h
	$(MOC) controlwidgetbase.h -o $(MOC_DIR)/moc_controlwidgetbase.cpp

$(MOC_DIR)/moc_documentlist.cpp: documentlist.h
	$(MOC) documentlist.h -o $(MOC_DIR)/moc_documentlist.cpp

$(MOC_DIR)/moc_loopcontrol.cpp: loopcontrol.h
	$(MOC) loopcontrol.h -o $(MOC_DIR)/moc_loopcontrol.cpp

$(MOC_DIR)/moc_playlistselection.cpp: playlistselection.h
	$(MOC) playlistselection.h -o $(MOC_DIR)/moc_playlistselection.cpp

$(MOC_DIR)/moc_id3tag.cpp: id3tag.h
	$(MOC) id3tag.h -o $(MOC_DIR)/moc_id3tag.cpp

$(MOC_DIR)/moc_action.cpp: action.h
	$(MOC) action.h -o $(MOC_DIR)/moc_action.cpp

$(MOC_DIR)/moc_fileinfo.cpp: fileinfo.h
	$(MOC) fileinfo.h -o $(MOC_DIR)/moc_fileinfo.cpp

$(MOC_DIR)/moc_tabbedselector.cpp: tabbedselector.h
	$(MOC) tabbedselector.h -o $(MOC_DIR)/moc_tabbedselector.cpp

$(MOC_DIR)/moc_mediaplayerstate.cpp: mediaplayerstate.h
	$(MOC) mediaplayerstate.h -o $(MOC_DIR)/moc_mediaplayerstate.cpp

$(MOC_DIR)/moc_videowidget.cpp: videowidget.h
	$(MOC) videowidget.h -o $(MOC_DIR)/moc_videowidget.cpp

$(MOC_DIR)/moc_audiowidget.cpp: audiowidget.h
	$(MOC) audiowidget.h -o $(MOC_DIR)/moc_audiowidget.cpp

$(MOC_DIR)/moc_playlistwidget.cpp: playlistwidget.h
	$(MOC) playlistwidget.h -o $(MOC_DIR)/moc_playlistwidget.cpp

$(MOC_DIR)/moc_mediaplayer.cpp: mediaplayer.h
	$(MOC) mediaplayer.h -o $(MOC_DIR)/moc_mediaplayer.cpp

$(MOC_DIR)/moc_audiodevice.cpp: audiodevice.h
	$(MOC) audiodevice.h -o $(MOC_DIR)/moc_audiodevice.cpp

$(MOC_DIR)/fileinfo.moc: fileinfo.cpp
	$(MOC) fileinfo.cpp -o $(MOC_DIR)/fileinfo.moc



lupdate:
	sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' 	    -e '/$$(.*/ d' 	    -e 's/^static:.*//' 	    <mediaplayer.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease mediaplayer.pro
	$(QPEDIR)/bin/qtopia-linstall mediaplayer.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: mediaplayer.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/mediaplayer
