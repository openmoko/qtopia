#############################################################################
# Automatically generated from settings/launcher/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = settings/launcher/qpe-launcher-settings.control

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
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= launchersettings
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	launchersettings.h \
		wallpaperselector.h \
		colorselector.h
SOURCES =	main.cpp \
		launchersettings.cpp \
		wallpaperselector.cpp \
		colorselector.cpp
OBJECTS =	$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/launchersettings.o \
		$(OBJECTS_DIR)/wallpaperselector.o \
		$(OBJECTS_DIR)/colorselector.o \
		$(OBJECTS_DIR)/launchersettingsbase.o \
		$(OBJECTS_DIR)/wallpaperselectorbase.o
INTERFACES =    launchersettingsbase.ui \
		wallpaperselectorbase.ui
UICDECLS =      launchersettingsbase.h \
		wallpaperselectorbase.h
UICIMPLS =      launchersettingsbase.cpp \
		wallpaperselectorbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_launchersettings.cpp \
		$(MOC_DIR)/moc_wallpaperselector.cpp \
		$(MOC_DIR)/moc_colorselector.cpp \
		$(MOC_DIR)/moc_launchersettingsbase.cpp \
		$(MOC_DIR)/moc_wallpaperselectorbase.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_launchersettings.o \
		$(OBJECTS_DIR)/moc_wallpaperselector.o \
		$(OBJECTS_DIR)/moc_colorselector.o \
		$(OBJECTS_DIR)/moc_launchersettingsbase.o \
		$(OBJECTS_DIR)/moc_wallpaperselectorbase.o


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
	tmake launcher.pro

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

$(OBJECTS_DIR)/main.o: main.cpp \
		launchersettings.h \
		launchersettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

$(OBJECTS_DIR)/launchersettings.o: launchersettings.cpp \
		launchersettings.h \
		launchersettingsbase.h \
		wallpaperselector.h \
		wallpaperselectorbase.h \
		colorselector.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/launchersettings.o launchersettings.cpp

$(OBJECTS_DIR)/wallpaperselector.o: wallpaperselector.cpp \
		wallpaperselector.h \
		wallpaperselectorbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/wallpaperselector.o wallpaperselector.cpp

$(OBJECTS_DIR)/colorselector.o: colorselector.cpp \
		colorselector.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/colorselector.o colorselector.cpp

launchersettingsbase.h: launchersettingsbase.ui
	$(UIC) launchersettingsbase.ui -o $(INTERFACE_DECL_PATH)/launchersettingsbase.h

launchersettingsbase.cpp: launchersettingsbase.ui
	$(UIC) launchersettingsbase.ui -i launchersettingsbase.h -o launchersettingsbase.cpp

wallpaperselectorbase.h: wallpaperselectorbase.ui
	$(UIC) wallpaperselectorbase.ui -o $(INTERFACE_DECL_PATH)/wallpaperselectorbase.h

wallpaperselectorbase.cpp: wallpaperselectorbase.ui
	$(UIC) wallpaperselectorbase.ui -i wallpaperselectorbase.h -o wallpaperselectorbase.cpp

$(OBJECTS_DIR)/launchersettingsbase.o: launchersettingsbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/launchersettingsbase.o launchersettingsbase.cpp

$(OBJECTS_DIR)/wallpaperselectorbase.o: wallpaperselectorbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/wallpaperselectorbase.o wallpaperselectorbase.cpp

$(OBJECTS_DIR)/moc_launchersettings.o: $(MOC_DIR)/moc_launchersettings.cpp \
		launchersettings.h \
		launchersettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_launchersettings.o $(MOC_DIR)/moc_launchersettings.cpp

$(OBJECTS_DIR)/moc_wallpaperselector.o: $(MOC_DIR)/moc_wallpaperselector.cpp \
		wallpaperselector.h \
		wallpaperselectorbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_wallpaperselector.o $(MOC_DIR)/moc_wallpaperselector.cpp

$(OBJECTS_DIR)/moc_colorselector.o: $(MOC_DIR)/moc_colorselector.cpp \
		colorselector.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_colorselector.o $(MOC_DIR)/moc_colorselector.cpp

$(OBJECTS_DIR)/moc_launchersettingsbase.o: $(MOC_DIR)/moc_launchersettingsbase.cpp \
		launchersettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_launchersettingsbase.o $(MOC_DIR)/moc_launchersettingsbase.cpp

$(OBJECTS_DIR)/moc_wallpaperselectorbase.o: $(MOC_DIR)/moc_wallpaperselectorbase.cpp \
		wallpaperselectorbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_wallpaperselectorbase.o $(MOC_DIR)/moc_wallpaperselectorbase.cpp

$(MOC_DIR)/moc_launchersettings.cpp: launchersettings.h
	$(MOC) launchersettings.h -o $(MOC_DIR)/moc_launchersettings.cpp

$(MOC_DIR)/moc_wallpaperselector.cpp: wallpaperselector.h
	$(MOC) wallpaperselector.h -o $(MOC_DIR)/moc_wallpaperselector.cpp

$(MOC_DIR)/moc_colorselector.cpp: colorselector.h
	$(MOC) colorselector.h -o $(MOC_DIR)/moc_colorselector.cpp

$(MOC_DIR)/moc_launchersettingsbase.cpp: launchersettingsbase.h
	$(MOC) launchersettingsbase.h -o $(MOC_DIR)/moc_launchersettingsbase.cpp

$(MOC_DIR)/moc_wallpaperselectorbase.cpp: wallpaperselectorbase.h
	$(MOC) wallpaperselectorbase.h -o $(MOC_DIR)/moc_wallpaperselectorbase.cpp



lupdate:
	sed -e 's/$$$$QTOPIA1DIR[^ ]*//' 	    -e 's/^static:.*//' 	    <launcher.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease launcher.pro
	$(QPEDIR)/bin/qtopia-linstall launcher.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: launcher.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make settings/launcher
