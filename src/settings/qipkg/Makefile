#############################################################################
# Automatically generated from settings/qipkg/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = settings/qipkg/qpe-qipkg.control

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
TARGET	= qipkg
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	packagemanager.h
SOURCES =	packagemanager.cpp \
		main.cpp
OBJECTS =	$(OBJECTS_DIR)/packagemanager.o \
		$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/packagemanagerbase.o \
		$(OBJECTS_DIR)/pkdesc.o \
		$(OBJECTS_DIR)/pkfind.o \
		$(OBJECTS_DIR)/pksettings.o
INTERFACES =    packagemanagerbase.ui \
		pkdesc.ui \
		pkfind.ui \
		pksettings.ui
UICDECLS =      packagemanagerbase.h \
		pkdesc.h \
		pkfind.h \
		pksettings.h
UICIMPLS =      packagemanagerbase.cpp \
		pkdesc.cpp \
		pkfind.cpp \
		pksettings.cpp
SRCMOC	=	$(MOC_DIR)/moc_packagemanager.cpp \
		$(MOC_DIR)/moc_packagemanagerbase.cpp \
		$(MOC_DIR)/moc_pkdesc.cpp \
		$(MOC_DIR)/moc_pkfind.cpp \
		$(MOC_DIR)/moc_pksettings.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_packagemanager.o \
		$(OBJECTS_DIR)/moc_packagemanagerbase.o \
		$(OBJECTS_DIR)/moc_pkdesc.o \
		$(OBJECTS_DIR)/moc_pkfind.o \
		$(OBJECTS_DIR)/moc_pksettings.o


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
	tmake qipkg.pro

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

$(OBJECTS_DIR)/packagemanager.o: packagemanager.cpp \
		packagemanager.h \
		packagemanagerbase.h \
		pkdesc.h \
		pkfind.h \
		pksettings.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/packagemanager.o packagemanager.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		packagemanager.h \
		packagemanagerbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

packagemanagerbase.h: packagemanagerbase.ui
	$(UIC) packagemanagerbase.ui -o $(INTERFACE_DECL_PATH)/packagemanagerbase.h

packagemanagerbase.cpp: packagemanagerbase.ui
	$(UIC) packagemanagerbase.ui -i packagemanagerbase.h -o packagemanagerbase.cpp

pkdesc.h: pkdesc.ui
	$(UIC) pkdesc.ui -o $(INTERFACE_DECL_PATH)/pkdesc.h

pkdesc.cpp: pkdesc.ui
	$(UIC) pkdesc.ui -i pkdesc.h -o pkdesc.cpp

pkfind.h: pkfind.ui
	$(UIC) pkfind.ui -o $(INTERFACE_DECL_PATH)/pkfind.h

pkfind.cpp: pkfind.ui
	$(UIC) pkfind.ui -i pkfind.h -o pkfind.cpp

pksettings.h: pksettings.ui
	$(UIC) pksettings.ui -o $(INTERFACE_DECL_PATH)/pksettings.h

pksettings.cpp: pksettings.ui
	$(UIC) pksettings.ui -i pksettings.h -o pksettings.cpp

$(OBJECTS_DIR)/packagemanagerbase.o: packagemanagerbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/packagemanagerbase.o packagemanagerbase.cpp

$(OBJECTS_DIR)/pkdesc.o: pkdesc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pkdesc.o pkdesc.cpp

$(OBJECTS_DIR)/pkfind.o: pkfind.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pkfind.o pkfind.cpp

$(OBJECTS_DIR)/pksettings.o: pksettings.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pksettings.o pksettings.cpp

$(OBJECTS_DIR)/moc_packagemanager.o: $(MOC_DIR)/moc_packagemanager.cpp \
		packagemanager.h \
		packagemanagerbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_packagemanager.o $(MOC_DIR)/moc_packagemanager.cpp

$(OBJECTS_DIR)/moc_packagemanagerbase.o: $(MOC_DIR)/moc_packagemanagerbase.cpp \
		packagemanagerbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_packagemanagerbase.o $(MOC_DIR)/moc_packagemanagerbase.cpp

$(OBJECTS_DIR)/moc_pkdesc.o: $(MOC_DIR)/moc_pkdesc.cpp \
		pkdesc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_pkdesc.o $(MOC_DIR)/moc_pkdesc.cpp

$(OBJECTS_DIR)/moc_pkfind.o: $(MOC_DIR)/moc_pkfind.cpp \
		pkfind.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_pkfind.o $(MOC_DIR)/moc_pkfind.cpp

$(OBJECTS_DIR)/moc_pksettings.o: $(MOC_DIR)/moc_pksettings.cpp \
		pksettings.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_pksettings.o $(MOC_DIR)/moc_pksettings.cpp

$(MOC_DIR)/moc_packagemanager.cpp: packagemanager.h
	$(MOC) packagemanager.h -o $(MOC_DIR)/moc_packagemanager.cpp

$(MOC_DIR)/moc_packagemanagerbase.cpp: packagemanagerbase.h
	$(MOC) packagemanagerbase.h -o $(MOC_DIR)/moc_packagemanagerbase.cpp

$(MOC_DIR)/moc_pkdesc.cpp: pkdesc.h
	$(MOC) pkdesc.h -o $(MOC_DIR)/moc_pkdesc.cpp

$(MOC_DIR)/moc_pkfind.cpp: pkfind.h
	$(MOC) pkfind.h -o $(MOC_DIR)/moc_pkfind.cpp

$(MOC_DIR)/moc_pksettings.cpp: pksettings.h
	$(MOC) pksettings.h -o $(MOC_DIR)/moc_pksettings.cpp



lupdate:
	sed -e 's/$$$$QTOPIA1DIR[^ ]*//' 	    -e 's/^static:.*//' 	    <qipkg.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease qipkg.pro
	$(QPEDIR)/bin/qtopia-linstall qipkg.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: qipkg.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make settings/qipkg
