#############################################################################
# Automatically generated from 3rdparty/applications/embeddedkonsole/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/applications/embeddedkonsole/qpe-embeddedkonsole.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) -DHAVE_OPENPTY
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) -DHAVE_OPENPTY
INCPATH =	-I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP) -lutil
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= embeddedkonsole
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	TEWidget.h \
		TEScreen.h \
		TECommon.h \
		TEHistory.h \
		TEmulation.h \
		TEmuVt102.h \
		session.h \
		keytrans.h \
		konsole.h \
		MyPty.h
SOURCES =	TEScreen.cpp \
		TEWidget.cpp \
		TEHistory.cpp \
		TEmulation.cpp \
		TEmuVt102.cpp \
		session.cpp \
		keytrans.cpp \
		konsole.cpp \
		main.cpp \
		MyPty.cpp
OBJECTS =	TEScreen.o \
		TEWidget.o \
		TEHistory.o \
		TEmulation.o \
		TEmuVt102.o \
		session.o \
		keytrans.o \
		konsole.o \
		main.o \
		MyPty.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	moc_TEWidget.cpp \
		moc_TEmulation.cpp \
		moc_TEmuVt102.cpp \
		moc_session.cpp \
		moc_konsole.cpp \
		moc_MyPty.cpp
OBJMOC	=	moc_TEWidget.o \
		moc_TEmulation.o \
		moc_TEmuVt102.o \
		moc_session.o \
		moc_konsole.o \
		moc_MyPty.o


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
	tmake embeddedkonsole.pro

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


REQUIRES=embeddedkonsole

####### Sub-libraries


###### Combined headers



####### Compile

TEScreen.o: TEScreen.cpp \
		TEScreen.h \
		TECommon.h \
		TEHistory.h

TEWidget.o: TEWidget.cpp \
		TEWidget.h \
		TECommon.h \
		session.h \
		MyPty.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h

TEHistory.o: TEHistory.cpp \
		TEHistory.h \
		TECommon.h

TEmulation.o: TEmulation.cpp \
		TEmulation.h \
		TEWidget.h \
		TECommon.h \
		TEScreen.h \
		TEHistory.h \
		keytrans.h

TEmuVt102.o: TEmuVt102.cpp \
		TEmuVt102.h \
		TEWidget.h \
		TECommon.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h

session.o: session.cpp \
		session.h \
		MyPty.h \
		TEWidget.h \
		TECommon.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h

keytrans.o: keytrans.cpp \
		keytrans.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		default.keytab.h

konsole.o: konsole.cpp \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		$(QPEDIR)/include/qpe/qpemenubar.h \
		$(QPEDIR)/include/qpe/config.h \
		konsole.h \
		MyPty.h \
		TEWidget.h \
		TECommon.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h \
		session.h

main.o: main.cpp \
		konsole.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		MyPty.h \
		TEWidget.h \
		TECommon.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h \
		session.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

MyPty.o: MyPty.cpp \
		MyPty.h

moc_TEWidget.o: moc_TEWidget.cpp \
		TEWidget.h \
		TECommon.h

moc_TEmulation.o: moc_TEmulation.cpp \
		TEmulation.h \
		TEWidget.h \
		TECommon.h \
		TEScreen.h \
		TEHistory.h \
		keytrans.h

moc_TEmuVt102.o: moc_TEmuVt102.cpp \
		TEmuVt102.h \
		TEWidget.h \
		TECommon.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h

moc_session.o: moc_session.cpp \
		session.h \
		MyPty.h \
		TEWidget.h \
		TECommon.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h

moc_konsole.o: moc_konsole.cpp \
		konsole.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h \
		MyPty.h \
		TEWidget.h \
		TECommon.h \
		TEmuVt102.h \
		TEScreen.h \
		TEHistory.h \
		TEmulation.h \
		keytrans.h \
		session.h

moc_MyPty.o: moc_MyPty.cpp \
		MyPty.h

moc_TEWidget.cpp: TEWidget.h
	$(MOC) TEWidget.h -o moc_TEWidget.cpp

moc_TEmulation.cpp: TEmulation.h
	$(MOC) TEmulation.h -o moc_TEmulation.cpp

moc_TEmuVt102.cpp: TEmuVt102.h
	$(MOC) TEmuVt102.h -o moc_TEmuVt102.cpp

moc_session.cpp: session.h
	$(MOC) session.h -o moc_session.cpp

moc_konsole.cpp: konsole.h
	$(MOC) konsole.h -o moc_konsole.cpp

moc_MyPty.cpp: MyPty.h
	$(MOC) MyPty.h -o moc_MyPty.cpp



lupdate:
	lupdate embeddedkonsole.pro

lrelease:
	lrelease embeddedkonsole.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: embeddedkonsole.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/applications/embeddedkonsole
