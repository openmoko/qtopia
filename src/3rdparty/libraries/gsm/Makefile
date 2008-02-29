#############################################################################
# Automatically generated from 3rdparty/libraries/gsm/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/libraries/gsm/gsm.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DFAST -DSASR -DWAV49
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DFAST -DSASR -DWAV49
INCPATH =	-I$(MOC_DIR) -I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS)  $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= gsm
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	gsm.h
SOURCES =	add.c \
		code.c \
		decode.c \
		gsm_create.c \
		gsm_decode.c \
		gsm_destroy.c \
		gsm_encode.c \
		gsm_option.c \
		long_term.c \
		lpc.c \
		preprocess.c \
		rpe.c \
		short_term.c \
		table.c
OBJECTS =	$(OBJECTS_DIR)/add.o \
		$(OBJECTS_DIR)/code.o \
		$(OBJECTS_DIR)/decode.o \
		$(OBJECTS_DIR)/gsm_create.o \
		$(OBJECTS_DIR)/gsm_decode.o \
		$(OBJECTS_DIR)/gsm_destroy.o \
		$(OBJECTS_DIR)/gsm_encode.o \
		$(OBJECTS_DIR)/gsm_option.o \
		$(OBJECTS_DIR)/long_term.o \
		$(OBJECTS_DIR)/lpc.o \
		$(OBJECTS_DIR)/preprocess.o \
		$(OBJECTS_DIR)/rpe.o \
		$(OBJECTS_DIR)/short_term.o \
		$(OBJECTS_DIR)/table.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	
OBJMOC	=	


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
	tmake gsm.pro

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

$(OBJECTS_DIR)/add.o: add.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/add.o add.c

$(OBJECTS_DIR)/code.o: code.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/code.o code.c

$(OBJECTS_DIR)/decode.o: decode.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/decode.o decode.c

$(OBJECTS_DIR)/gsm_create.o: gsm_create.c \
		gsm.h \
		private.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gsm_create.o gsm_create.c

$(OBJECTS_DIR)/gsm_decode.o: gsm_decode.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gsm_decode.o gsm_decode.c

$(OBJECTS_DIR)/gsm_destroy.o: gsm_destroy.c \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gsm_destroy.o gsm_destroy.c

$(OBJECTS_DIR)/gsm_encode.o: gsm_encode.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gsm_encode.o gsm_encode.c

$(OBJECTS_DIR)/gsm_option.o: gsm_option.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/gsm_option.o gsm_option.c

$(OBJECTS_DIR)/long_term.o: long_term.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/long_term.o long_term.c

$(OBJECTS_DIR)/lpc.o: lpc.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/lpc.o lpc.c

$(OBJECTS_DIR)/preprocess.o: preprocess.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/preprocess.o preprocess.c

$(OBJECTS_DIR)/rpe.o: rpe.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/rpe.o rpe.c

$(OBJECTS_DIR)/short_term.o: short_term.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/short_term.o short_term.c

$(OBJECTS_DIR)/table.o: table.c \
		private.h \
		gsm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/table.o table.c



lupdate:
	sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' 	    -e '/$$(.*/ d' 	    -e 's/^static:.*//' 	    <gsm.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease gsm.pro
	$(QPEDIR)/bin/qtopia-linstall gsm.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: gsm.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/libraries/gsm
