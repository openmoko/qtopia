#############################################################################
# Automatically generated from 3rdparty/applications/keypebble/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/applications/keypebble/qpe-keypebble.control

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
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= keypebble
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	d3des.h \
		krfbbuffer.h \
		krfbcanvas.h \
		krfbconnection.h \
		krfbdecoder.h \
		krfblogin.h \
		krfboptions.h \
		krfbserverinfo.h \
		kvnc.h \
		kvncconnectdlg.h \
		kvncoptionsdlg.h \
		version.h \
		vncauth.h
SOURCES =	d3des.c \
		vncauth.c \
		krfbbuffer.cpp \
		krfbcanvas.cpp \
		krfbconnection.cpp \
		krfbdecoder.cpp \
		krfblogin.cpp \
		krfboptions.cpp \
		kvnc.cpp \
		kvncconnectdlg.cpp \
		kvncoptionsdlg.cpp \
		main.cpp
OBJECTS =	d3des.o \
		vncauth.o \
		krfbbuffer.o \
		krfbcanvas.o \
		krfbconnection.o \
		krfbdecoder.o \
		krfblogin.o \
		krfboptions.o \
		kvnc.o \
		kvncconnectdlg.o \
		kvncoptionsdlg.o \
		main.o \
		vncoptionsbase.o
INTERFACES =    vncoptionsbase.ui
UICDECLS =      vncoptionsbase.h
UICIMPLS =      vncoptionsbase.cpp
SRCMOC	=	moc_krfbbuffer.cpp \
		moc_krfbcanvas.cpp \
		moc_krfbconnection.cpp \
		moc_krfbdecoder.cpp \
		moc_krfblogin.cpp \
		moc_kvnc.cpp \
		moc_kvncconnectdlg.cpp \
		moc_kvncoptionsdlg.cpp \
		moc_vncoptionsbase.cpp
OBJMOC	=	moc_krfbbuffer.o \
		moc_krfbcanvas.o \
		moc_krfbconnection.o \
		moc_krfbdecoder.o \
		moc_krfblogin.o \
		moc_kvnc.o \
		moc_kvncconnectdlg.o \
		moc_kvncoptionsdlg.o \
		moc_vncoptionsbase.o


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
	tmake keypebble.pro

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

d3des.o: d3des.c \
		d3des.h

vncauth.o: vncauth.c \
		vncauth.h \
		d3des.h

krfbbuffer.o: krfbbuffer.cpp \
		krfbdecoder.h \
		krfbbuffer.h \
		krfbserverinfo.h

krfbcanvas.o: krfbcanvas.cpp \
		kvncconnectdlg.h \
		krfbconnection.h \
		krfbcanvas.h \
		krfboptions.h \
		krfbbuffer.h \
		$(QPEDIR)/include/qpe/config.h

krfbconnection.o: krfbconnection.cpp \
		krfbconnection.h \
		krfblogin.h \
		krfboptions.h \
		krfbdecoder.h \
		krfbbuffer.h

krfbdecoder.o: krfbdecoder.cpp \
		krfbconnection.h \
		krfboptions.h \
		krfbserverinfo.h \
		krfbdecoder.h \
		krfbbuffer.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

krfblogin.o: krfblogin.cpp \
		vncauth.h \
		krfblogin.h \
		krfbconnection.h

krfboptions.o: krfboptions.cpp \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		krfboptions.h

kvnc.o: kvnc.cpp \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/global.h \
		kvnc.h \
		krfbcanvas.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h \
		krfbconnection.h \
		krfboptions.h

kvncconnectdlg.o: kvncconnectdlg.cpp \
		$(QPEDIR)/include/qpe/config.h \
		krfbconnection.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h \
		kvncconnectdlg.h

kvncoptionsdlg.o: kvncoptionsdlg.cpp \
		krfboptions.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h

main.o: main.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		kvnc.h

vncoptionsbase.h: vncoptionsbase.ui
	$(UIC) vncoptionsbase.ui -o $(INTERFACE_DECL_PATH)/vncoptionsbase.h

vncoptionsbase.cpp: vncoptionsbase.ui
	$(UIC) vncoptionsbase.ui -i vncoptionsbase.h -o vncoptionsbase.cpp

vncoptionsbase.o: vncoptionsbase.cpp \
		vncoptionsbase.h \
		vncoptionsbase.ui

moc_krfbbuffer.o: moc_krfbbuffer.cpp \
		krfbbuffer.h

moc_krfbcanvas.o: moc_krfbcanvas.cpp \
		krfbcanvas.h

moc_krfbconnection.o: moc_krfbconnection.cpp \
		krfbconnection.h

moc_krfbdecoder.o: moc_krfbdecoder.cpp \
		krfbdecoder.h

moc_krfblogin.o: moc_krfblogin.cpp \
		krfblogin.h

moc_kvnc.o: moc_kvnc.cpp \
		kvnc.h

moc_kvncconnectdlg.o: moc_kvncconnectdlg.cpp \
		kvncconnectdlg.h

moc_kvncoptionsdlg.o: moc_kvncoptionsdlg.cpp \
		kvncoptionsdlg.h \
		vncoptionsbase.h

moc_vncoptionsbase.o: moc_vncoptionsbase.cpp \
		vncoptionsbase.h

moc_krfbbuffer.cpp: krfbbuffer.h
	$(MOC) krfbbuffer.h -o moc_krfbbuffer.cpp

moc_krfbcanvas.cpp: krfbcanvas.h
	$(MOC) krfbcanvas.h -o moc_krfbcanvas.cpp

moc_krfbconnection.cpp: krfbconnection.h
	$(MOC) krfbconnection.h -o moc_krfbconnection.cpp

moc_krfbdecoder.cpp: krfbdecoder.h
	$(MOC) krfbdecoder.h -o moc_krfbdecoder.cpp

moc_krfblogin.cpp: krfblogin.h
	$(MOC) krfblogin.h -o moc_krfblogin.cpp

moc_kvnc.cpp: kvnc.h
	$(MOC) kvnc.h -o moc_kvnc.cpp

moc_kvncconnectdlg.cpp: kvncconnectdlg.h
	$(MOC) kvncconnectdlg.h -o moc_kvncconnectdlg.cpp

moc_kvncoptionsdlg.cpp: kvncoptionsdlg.h
	$(MOC) kvncoptionsdlg.h -o moc_kvncoptionsdlg.cpp

moc_vncoptionsbase.cpp: vncoptionsbase.h
	$(MOC) vncoptionsbase.h -o moc_vncoptionsbase.cpp



lupdate:
	lupdate keypebble.pro

lrelease:
	lrelease keypebble.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: keypebble.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/applications/keypebble
