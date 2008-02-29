#############################################################################
# Automatically generated from plugins/inputmethods/handwriting/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = plugins/inputmethods/handwriting/qpe-handwriting.control

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
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/inputmethods/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= qhandwriting
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	qimpenchar.h \
		qimpenprofile.h \
		qimpencombining.h \
		qimpenhelp.h \
		qimpeninput.h \
		qimpenmatch.h \
		qimpensetup.h \
		qimpenstroke.h \
		qimpenwidget.h \
		qimpenwordpick.h \
		handwritingimpl.h
SOURCES =	qimpenchar.cpp \
		qimpenprofile.cpp \
		qimpencombining.cpp \
		qimpenhelp.cpp \
		qimpeninput.cpp \
		qimpenmatch.cpp \
		qimpensetup.cpp \
		qimpenstroke.cpp \
		qimpenwidget.cpp \
		qimpenwordpick.cpp \
		handwritingimpl.cpp
OBJECTS =	qimpenchar.o \
		qimpenprofile.o \
		qimpencombining.o \
		qimpenhelp.o \
		qimpeninput.o \
		qimpenmatch.o \
		qimpensetup.o \
		qimpenstroke.o \
		qimpenwidget.o \
		qimpenwordpick.o \
		handwritingimpl.o \
		qimpenprefbase.o
INTERFACES =    qimpenprefbase.ui
UICDECLS =      qimpenprefbase.h
UICIMPLS =      qimpenprefbase.cpp
SRCMOC	=	moc_qimpenhelp.cpp \
		moc_qimpeninput.cpp \
		moc_qimpenmatch.cpp \
		moc_qimpensetup.cpp \
		moc_qimpenwidget.cpp \
		moc_qimpenwordpick.cpp \
		moc_qimpenprefbase.cpp
OBJMOC	=	moc_qimpenhelp.o \
		moc_qimpeninput.o \
		moc_qimpenmatch.o \
		moc_qimpensetup.o \
		moc_qimpenwidget.o \
		moc_qimpenwordpick.o \
		moc_qimpenprefbase.o


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
	tmake handwriting.pro

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

qimpenchar.o: qimpenchar.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h

qimpenprofile.o: qimpenprofile.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprofile.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h

qimpencombining.o: qimpencombining.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h

qimpenhelp.o: qimpenhelp.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpencombining.h \
		qimpenmatch.h \
		$(QPEDIR)/include/qpe/qdawg.h \
		qimpenhelp.h \
		qimpenprofile.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/stringutil.h

qimpeninput.o: qimpeninput.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpensetup.h \
		qimpenprofile.h \
		qimpeninput.h \
		$(QPEDIR)/include/qpe/qdawg.h \
		qimpencombining.h \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenhelp.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h

qimpenmatch.o: qimpenmatch.cpp \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

qimpensetup.o: qimpensetup.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprefbase.h \
		qimpensetup.h \
		qimpenprofile.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/config.h

qimpenstroke.o: qimpenstroke.cpp \
		qimpenstroke.h

qimpenwidget.o: qimpenwidget.cpp \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenwidget.h

qimpenwordpick.o: qimpenwordpick.cpp \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h

handwritingimpl.o: handwritingimpl.cpp \
		qimpeninput.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h \
		handwritingimpl.h \
		$(QPEDIR)/include/qpe/inputmethodinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h

qimpenprefbase.h: qimpenprefbase.ui
	$(UIC) qimpenprefbase.ui -o $(INTERFACE_DECL_PATH)/qimpenprefbase.h

qimpenprefbase.cpp: qimpenprefbase.ui
	$(UIC) qimpenprefbase.ui -i qimpenprefbase.h -o qimpenprefbase.cpp

qimpenprefbase.o: qimpenprefbase.cpp \
		qimpenprefbase.h \
		qimpenprefbase.ui

moc_qimpenhelp.o: moc_qimpenhelp.cpp \
		qimpenhelp.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprofile.h

moc_qimpeninput.o: moc_qimpeninput.cpp \
		qimpeninput.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h

moc_qimpenmatch.o: moc_qimpenmatch.cpp \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h

moc_qimpensetup.o: moc_qimpensetup.cpp \
		qimpensetup.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h

moc_qimpenwidget.o: moc_qimpenwidget.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h

moc_qimpenwordpick.o: moc_qimpenwordpick.cpp \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h \
		$(QPEDIR)/include/qpe/qdawg.h

moc_qimpenprefbase.o: moc_qimpenprefbase.cpp \
		qimpenprefbase.h

moc_qimpenhelp.cpp: qimpenhelp.h
	$(MOC) qimpenhelp.h -o moc_qimpenhelp.cpp

moc_qimpeninput.cpp: qimpeninput.h
	$(MOC) qimpeninput.h -o moc_qimpeninput.cpp

moc_qimpenmatch.cpp: qimpenmatch.h
	$(MOC) qimpenmatch.h -o moc_qimpenmatch.cpp

moc_qimpensetup.cpp: qimpensetup.h
	$(MOC) qimpensetup.h -o moc_qimpensetup.cpp

moc_qimpenwidget.cpp: qimpenwidget.h
	$(MOC) qimpenwidget.h -o moc_qimpenwidget.cpp

moc_qimpenwordpick.cpp: qimpenwordpick.h
	$(MOC) qimpenwordpick.h -o moc_qimpenwordpick.cpp

moc_qimpenprefbase.cpp: qimpenprefbase.h
	$(MOC) qimpenprefbase.h -o moc_qimpenprefbase.cpp



lupdate:
	lupdate handwriting.pro

lrelease:
	lrelease handwriting.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: handwriting.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make plugins/inputmethods/handwriting
