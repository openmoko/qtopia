#############################################################################
# Automatically generated from settings/pluginmgr/Makefile.in
# Build options from 
#############################################################################

default: all  # package

CONTROL = settings/pluginmgr/qpe-pluginmanager.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) -DQTOPIA_APP_INTERFACE
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) -DQTOPIA_APP_INTERFACE
INCPATH =	-I$(MOC_DIR) -I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= pluginmanager
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	pluginmanager.h
SOURCES =	pluginmanager.cpp \
		main.cpp
OBJECTS =	$(OBJECTS_DIR)/pluginmanager.o \
		$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/pluginmanagerbase.o
INTERFACES =    pluginmanagerbase.ui
UICDECLS =      pluginmanagerbase.h
UICIMPLS =      pluginmanagerbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_pluginmanager.cpp \
		$(MOC_DIR)/moc_pluginmanagerbase.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_pluginmanager.o \
		$(OBJECTS_DIR)/moc_pluginmanagerbase.o


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
	tmake pluginmgr.pro

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

$(OBJECTS_DIR)/pluginmanager.o: pluginmanager.cpp \
		pluginmanager.h \
		pluginmanagerbase.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/qtopiawinexport.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		$(QPEDIR)/include/qtopia/applicationinterface.h \
		$(QPEDIR)/include/qtopia/pluginloader.h \
		$(QPEDIR)/include/qtopia/global.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/resource.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginmanager.o pluginmanager.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		pluginmanager.h \
		pluginmanagerbase.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/qtopiawinexport.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		$(QPEDIR)/include/qtopia/applicationinterface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

pluginmanagerbase.h: pluginmanagerbase.ui
	$(UIC) pluginmanagerbase.ui -o $(INTERFACE_DECL_PATH)/pluginmanagerbase.h

pluginmanagerbase.cpp: pluginmanagerbase.ui
	$(UIC) pluginmanagerbase.ui -i pluginmanagerbase.h -o pluginmanagerbase.cpp

$(OBJECTS_DIR)/pluginmanagerbase.o: pluginmanagerbase.cpp \
		pluginmanagerbase.h \
		pluginmanagerbase.ui
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginmanagerbase.o pluginmanagerbase.cpp

$(OBJECTS_DIR)/moc_pluginmanager.o: $(MOC_DIR)/moc_pluginmanager.cpp \
		pluginmanager.h \
		pluginmanagerbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_pluginmanager.o $(MOC_DIR)/moc_pluginmanager.cpp

$(OBJECTS_DIR)/moc_pluginmanagerbase.o: $(MOC_DIR)/moc_pluginmanagerbase.cpp \
		pluginmanagerbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_pluginmanagerbase.o $(MOC_DIR)/moc_pluginmanagerbase.cpp

$(MOC_DIR)/moc_pluginmanager.cpp: pluginmanager.h
	$(MOC) pluginmanager.h -o $(MOC_DIR)/moc_pluginmanager.cpp

$(MOC_DIR)/moc_pluginmanagerbase.cpp: pluginmanagerbase.h
	$(MOC) pluginmanagerbase.h -o $(MOC_DIR)/moc_pluginmanagerbase.cpp



lupdate-check:
	@sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <pluginmgr.pro >t$$$$.pro && \
	rm -rf t$$$$&& \
	mkdir t$$$$ && \
	cp -a *.ts t$$$$ && \
	chmod +w *.ts && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro && \
	for i in *.ts; \
	do \
	    if cmp -s $$i t$$$$/$$i; \
	    then \
		true; \
	    else \
		echo out-of-date $$i; \
	    fi; \
	    rm $$i; \
	    mv t$$$$/$$i $$i; \
	done && \
	rmdir t$$$$

lupdate:
	sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <pluginmgr.pro >t$$$$.pro && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro

lrelease:
	lrelease pluginmgr.pro
	$(QPEDIR)/bin/qtopia-linstall pluginmgr.pro


ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic  $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: pluginmgr.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make settings/pluginmgr
