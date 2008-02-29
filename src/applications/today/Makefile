#############################################################################
# Automatically generated from applications/today/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = applications/today/qpe-today.control

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
INCPATH =	-I$(QPEDIR)/include -Ilibrary
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lqtopiaservices -Wl,-export-dynamic $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= today
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	today.h \
		todayoptions.h
SOURCES =	today.cpp \
		todayoptions.cpp \
		main.cpp
OBJECTS =	today.o \
		todayoptions.o \
		main.o \
		todayoptionsbase.o
INTERFACES =    todayoptionsbase.ui
UICDECLS =      todayoptionsbase.h
UICIMPLS =      todayoptionsbase.cpp
SRCMOC	=	moc_today.cpp \
		moc_todayoptions.cpp \
		moc_todayoptionsbase.cpp
OBJMOC	=	moc_today.o \
		moc_todayoptions.o \
		moc_todayoptionsbase.o


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
	tmake today.pro

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

today.o: today.cpp \
		today.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		todayoptions.h \
		todayoptionsbase.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/global.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		$(QPEDIR)/include/qtopia/datebookmonth.h \
		$(QPEDIR)/include/qtopia/private/event.h \
		$(QPEDIR)/include/qtopia/private/palmtoprecord.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

todayoptions.o: todayoptions.cpp \
		todayoptions.h \
		todayoptionsbase.h \
		today.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/config.h

main.o: main.cpp \
		today.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h \
		$(QPEDIR)/include/qpe/qpeapplication.h

todayoptionsbase.h: todayoptionsbase.ui
	$(UIC) todayoptionsbase.ui -o $(INTERFACE_DECL_PATH)/todayoptionsbase.h

todayoptionsbase.cpp: todayoptionsbase.ui
	$(UIC) todayoptionsbase.ui -i todayoptionsbase.h -o todayoptionsbase.cpp

todayoptionsbase.o: todayoptionsbase.cpp \
		todayoptionsbase.h \
		todayoptionsbase.ui

moc_today.o: moc_today.cpp \
		today.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h

moc_todayoptions.o: moc_todayoptions.cpp \
		todayoptions.h \
		todayoptionsbase.h \
		today.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/todayplugininterface.h

moc_todayoptionsbase.o: moc_todayoptionsbase.cpp \
		todayoptionsbase.h

moc_today.cpp: today.h
	$(MOC) today.h -o moc_today.cpp

moc_todayoptions.cpp: todayoptions.h
	$(MOC) todayoptions.h -o moc_todayoptions.cpp

moc_todayoptionsbase.cpp: todayoptionsbase.h
	$(MOC) todayoptionsbase.h -o moc_todayoptionsbase.cpp



lupdate:
	lupdate today.pro

lrelease:
	lrelease today.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: today.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/today
