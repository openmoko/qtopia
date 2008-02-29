#############################################################################
# Automatically generated from plugins/today/todo/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = plugins/today/todo/qpe-today-todoplugin.control

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
INCPATH =	-I$(QPEDIR)/include -I.. -I../library
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lqpepim $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/today/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= todoplugin
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	todoplugin.h \
		todopluginimpl.h \
		todopluginoptions.h
SOURCES =	todoplugin.cpp \
		todopluginimpl.cpp \
		todopluginoptions.cpp
OBJECTS =	todoplugin.o \
		todopluginimpl.o \
		todopluginoptions.o \
		todooptionsbase.o
INTERFACES =    todooptionsbase.ui
UICDECLS =      todooptionsbase.h
UICIMPLS =      todooptionsbase.cpp
SRCMOC	=	moc_todoplugin.cpp \
		moc_todooptionsbase.cpp
OBJMOC	=	moc_todoplugin.o \
		moc_todooptionsbase.o


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
	tmake todo.pro

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

todoplugin.o: todoplugin.cpp \
		todoplugin.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/pim/task.h \
		$(QPEDIR)/include/qtopia/stringutil.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		todopluginoptions.h \
		todooptionsbase.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/pim/todoaccess.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		$(QPEDIR)/include/qtopia/resource.h

todopluginimpl.o: todopluginimpl.cpp \
		todoplugin.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/pim/task.h \
		$(QPEDIR)/include/qtopia/stringutil.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		todopluginimpl.h \
		$(QPEDIR)/include/qpe/todayplugininterface.h

todopluginoptions.o: todopluginoptions.cpp \
		todopluginoptions.h \
		todooptionsbase.h \
		$(QPEDIR)/include/qtopia/config.h

todooptionsbase.h: todooptionsbase.ui
	$(UIC) todooptionsbase.ui -o $(INTERFACE_DECL_PATH)/todooptionsbase.h

todooptionsbase.cpp: todooptionsbase.ui
	$(UIC) todooptionsbase.ui -i todooptionsbase.h -o todooptionsbase.cpp

todooptionsbase.o: todooptionsbase.cpp \
		todooptionsbase.h \
		todooptionsbase.ui

moc_todoplugin.o: moc_todoplugin.cpp \
		todoplugin.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/pim/task.h \
		$(QPEDIR)/include/qtopia/stringutil.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h

moc_todooptionsbase.o: moc_todooptionsbase.cpp \
		todooptionsbase.h

moc_todoplugin.cpp: todoplugin.h
	$(MOC) todoplugin.h -o moc_todoplugin.cpp

moc_todooptionsbase.cpp: todooptionsbase.h
	$(MOC) todooptionsbase.h -o moc_todooptionsbase.cpp



lupdate:
	lupdate todo.pro

lrelease:
	lrelease todo.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: todo.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make plugins/today/todo
