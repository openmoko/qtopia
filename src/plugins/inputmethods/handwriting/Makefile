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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB)
INCPATH =	-I$(MOC_DIR)
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
OBJECTS =	$(OBJECTS_DIR)/qimpenchar.o \
		$(OBJECTS_DIR)/qimpenprofile.o \
		$(OBJECTS_DIR)/qimpencombining.o \
		$(OBJECTS_DIR)/qimpenhelp.o \
		$(OBJECTS_DIR)/qimpeninput.o \
		$(OBJECTS_DIR)/qimpenmatch.o \
		$(OBJECTS_DIR)/qimpensetup.o \
		$(OBJECTS_DIR)/qimpenstroke.o \
		$(OBJECTS_DIR)/qimpenwidget.o \
		$(OBJECTS_DIR)/qimpenwordpick.o \
		$(OBJECTS_DIR)/handwritingimpl.o \
		$(OBJECTS_DIR)/qimpenprefbase.o
INTERFACES =    qimpenprefbase.ui
UICDECLS =      qimpenprefbase.h
UICIMPLS =      qimpenprefbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_qimpenhelp.cpp \
		$(MOC_DIR)/moc_qimpeninput.cpp \
		$(MOC_DIR)/moc_qimpenmatch.cpp \
		$(MOC_DIR)/moc_qimpensetup.cpp \
		$(MOC_DIR)/moc_qimpenwidget.cpp \
		$(MOC_DIR)/moc_qimpenwordpick.cpp \
		$(MOC_DIR)/moc_qimpenprefbase.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_qimpenhelp.o \
		$(OBJECTS_DIR)/moc_qimpeninput.o \
		$(OBJECTS_DIR)/moc_qimpenmatch.o \
		$(OBJECTS_DIR)/moc_qimpensetup.o \
		$(OBJECTS_DIR)/moc_qimpenwidget.o \
		$(OBJECTS_DIR)/moc_qimpenwordpick.o \
		$(OBJECTS_DIR)/moc_qimpenprefbase.o


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

$(OBJECTS_DIR)/qimpenchar.o: qimpenchar.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenchar.o qimpenchar.cpp

$(OBJECTS_DIR)/qimpenprofile.o: qimpenprofile.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprofile.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenprofile.o qimpenprofile.cpp

$(OBJECTS_DIR)/qimpencombining.o: qimpencombining.cpp \
		qimpencombining.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpencombining.o qimpencombining.cpp

$(OBJECTS_DIR)/qimpenhelp.o: qimpenhelp.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpencombining.h \
		qimpenmatch.h \
		qimpenhelp.h \
		qimpenprofile.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenhelp.o qimpenhelp.cpp

$(OBJECTS_DIR)/qimpeninput.o: qimpeninput.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpensetup.h \
		qimpenprofile.h \
		qimpeninput.h \
		qimpencombining.h \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenhelp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpeninput.o qimpeninput.cpp

$(OBJECTS_DIR)/qimpenmatch.o: qimpenmatch.cpp \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenmatch.o qimpenmatch.cpp

$(OBJECTS_DIR)/qimpensetup.o: qimpensetup.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprefbase.h \
		qimpensetup.h \
		qimpenprofile.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpensetup.o qimpensetup.cpp

$(OBJECTS_DIR)/qimpenstroke.o: qimpenstroke.cpp \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenstroke.o qimpenstroke.cpp

$(OBJECTS_DIR)/qimpenwidget.o: qimpenwidget.cpp \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenwidget.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenwidget.o qimpenwidget.cpp

$(OBJECTS_DIR)/qimpenwordpick.o: qimpenwordpick.cpp \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenwordpick.o qimpenwordpick.cpp

$(OBJECTS_DIR)/handwritingimpl.o: handwritingimpl.cpp \
		qimpeninput.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h \
		handwritingimpl.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/handwritingimpl.o handwritingimpl.cpp

qimpenprefbase.h: qimpenprefbase.ui
	$(UIC) qimpenprefbase.ui -o $(INTERFACE_DECL_PATH)/qimpenprefbase.h

qimpenprefbase.cpp: qimpenprefbase.ui
	$(UIC) qimpenprefbase.ui -i qimpenprefbase.h -o qimpenprefbase.cpp

$(OBJECTS_DIR)/qimpenprefbase.o: qimpenprefbase.cpp \
		qimpenprefbase.h \
		qimpenprefbase.ui
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qimpenprefbase.o qimpenprefbase.cpp

$(OBJECTS_DIR)/moc_qimpenhelp.o: $(MOC_DIR)/moc_qimpenhelp.cpp \
		qimpenhelp.h \
		qimpenchar.h \
		qimpenstroke.h \
		qimpenprofile.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpenhelp.o $(MOC_DIR)/moc_qimpenhelp.cpp

$(OBJECTS_DIR)/moc_qimpeninput.o: $(MOC_DIR)/moc_qimpeninput.cpp \
		qimpeninput.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpeninput.o $(MOC_DIR)/moc_qimpeninput.cpp

$(OBJECTS_DIR)/moc_qimpenmatch.o: $(MOC_DIR)/moc_qimpenmatch.cpp \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpenmatch.o $(MOC_DIR)/moc_qimpenmatch.cpp

$(OBJECTS_DIR)/moc_qimpensetup.o: $(MOC_DIR)/moc_qimpensetup.cpp \
		qimpensetup.h \
		qimpenprofile.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpensetup.o $(MOC_DIR)/moc_qimpensetup.cpp

$(OBJECTS_DIR)/moc_qimpenwidget.o: $(MOC_DIR)/moc_qimpenwidget.cpp \
		qimpenwidget.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpenwidget.o $(MOC_DIR)/moc_qimpenwidget.cpp

$(OBJECTS_DIR)/moc_qimpenwordpick.o: $(MOC_DIR)/moc_qimpenwordpick.cpp \
		qimpenwordpick.h \
		qimpenmatch.h \
		qimpenchar.h \
		qimpenstroke.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpenwordpick.o $(MOC_DIR)/moc_qimpenwordpick.cpp

$(OBJECTS_DIR)/moc_qimpenprefbase.o: $(MOC_DIR)/moc_qimpenprefbase.cpp \
		qimpenprefbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qimpenprefbase.o $(MOC_DIR)/moc_qimpenprefbase.cpp

$(MOC_DIR)/moc_qimpenhelp.cpp: qimpenhelp.h
	$(MOC) qimpenhelp.h -o $(MOC_DIR)/moc_qimpenhelp.cpp

$(MOC_DIR)/moc_qimpeninput.cpp: qimpeninput.h
	$(MOC) qimpeninput.h -o $(MOC_DIR)/moc_qimpeninput.cpp

$(MOC_DIR)/moc_qimpenmatch.cpp: qimpenmatch.h
	$(MOC) qimpenmatch.h -o $(MOC_DIR)/moc_qimpenmatch.cpp

$(MOC_DIR)/moc_qimpensetup.cpp: qimpensetup.h
	$(MOC) qimpensetup.h -o $(MOC_DIR)/moc_qimpensetup.cpp

$(MOC_DIR)/moc_qimpenwidget.cpp: qimpenwidget.h
	$(MOC) qimpenwidget.h -o $(MOC_DIR)/moc_qimpenwidget.cpp

$(MOC_DIR)/moc_qimpenwordpick.cpp: qimpenwordpick.h
	$(MOC) qimpenwordpick.h -o $(MOC_DIR)/moc_qimpenwordpick.cpp

$(MOC_DIR)/moc_qimpenprefbase.cpp: qimpenprefbase.h
	$(MOC) qimpenprefbase.h -o $(MOC_DIR)/moc_qimpenprefbase.cpp



lupdate:
	sed -e 's/$$$$QTOPIA1DIR[^ ]*//' 	    -e 's/^static:.*//' 	    <handwriting.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease handwriting.pro
	$(QPEDIR)/bin/qtopia-linstall handwriting.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: handwriting.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make plugins/inputmethods/handwriting
