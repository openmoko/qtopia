#############################################################################
# Automatically generated from libraries/qtopiacalc/Makefile.in
# Build options from 
#############################################################################

default: all  # 

CONTROL = 

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DQTOPIA_APP_INTERFACE
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DQTOPIA_APP_INTERFACE
INCPATH =	-I$(MOC_DIR)
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib$(PROJMAK)/
VER_MAJ = 1
VER_MIN = 7
VER_PATCH = 0
TARGET	= qtopiacalc
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	calculator.h \
		engine.h \
		data.h \
		instruction.h \
		stdinputwidgets.h \
		doubledata.h \
		doubleinstruction.h
SOURCES =	calculator.cpp \
		engine.cpp \
		data.cpp \
		instruction.cpp \
		stdinputwidgets.cpp \
		doubledata.cpp \
		doubleinstruction.cpp
OBJECTS =	$(OBJECTS_DIR)/calculator.o \
		$(OBJECTS_DIR)/engine.o \
		$(OBJECTS_DIR)/data.o \
		$(OBJECTS_DIR)/instruction.o \
		$(OBJECTS_DIR)/stdinputwidgets.o \
		$(OBJECTS_DIR)/doubledata.o \
		$(OBJECTS_DIR)/doubleinstruction.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	$(MOC_DIR)/moc_calculator.cpp \
		$(MOC_DIR)/moc_stdinputwidgets.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_calculator.o \
		$(OBJECTS_DIR)/moc_stdinputwidgets.o


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
	tmake qtopiacalc.pro

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

$(OBJECTS_DIR)/calculator.o: calculator.cpp \
		../../plugins/calculator/simple/simple.h \
		doubleinstruction.h \
		instruction.h \
		data.h \
		doubledata.h \
		engine.h \
		calculator.h \
		plugininterface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/calculator.o calculator.cpp

$(OBJECTS_DIR)/engine.o: engine.cpp \
		engine.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/engine.o engine.cpp

$(OBJECTS_DIR)/data.o: data.cpp \
		data.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/data.o data.cpp

$(OBJECTS_DIR)/instruction.o: instruction.cpp \
		instruction.h \
		data.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/instruction.o instruction.cpp

$(OBJECTS_DIR)/stdinputwidgets.o: stdinputwidgets.cpp \
		stdinputwidgets.h \
		engine.h \
		doubleinstruction.h \
		instruction.h \
		data.h \
		doubledata.h \
		fractioninstruction.h \
		fractiondata.h \
		calculator.h \
		plugininterface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/stdinputwidgets.o stdinputwidgets.cpp

$(OBJECTS_DIR)/doubledata.o: doubledata.cpp \
		doubledata.h \
		data.h \
		engine.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/doubledata.o doubledata.cpp

$(OBJECTS_DIR)/doubleinstruction.o: doubleinstruction.cpp \
		doubleinstruction.h \
		instruction.h \
		data.h \
		doubledata.h \
		engine.h \
		integerdata.h \
		fractiondata.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/doubleinstruction.o doubleinstruction.cpp

$(OBJECTS_DIR)/moc_calculator.o: $(MOC_DIR)/moc_calculator.cpp \
		calculator.h \
		plugininterface.h \
		instruction.h \
		data.h \
		engine.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_calculator.o $(MOC_DIR)/moc_calculator.cpp

$(OBJECTS_DIR)/moc_stdinputwidgets.o: $(MOC_DIR)/moc_stdinputwidgets.cpp \
		stdinputwidgets.h \
		engine.h \
		doubleinstruction.h \
		instruction.h \
		data.h \
		doubledata.h \
		fractioninstruction.h \
		fractiondata.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_stdinputwidgets.o $(MOC_DIR)/moc_stdinputwidgets.cpp

$(MOC_DIR)/moc_calculator.cpp: calculator.h
	$(MOC) calculator.h -o $(MOC_DIR)/moc_calculator.cpp

$(MOC_DIR)/moc_stdinputwidgets.cpp: stdinputwidgets.h
	$(MOC) stdinputwidgets.h -o $(MOC_DIR)/moc_stdinputwidgets.cpp



lupdate-check:
	@true

lupdate:
	@true

lrelease:
	@echo "No TRANSLATIONS"


ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic  $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: qtopiacalc.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make libraries/qtopiacalc
