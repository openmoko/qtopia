#############################################################################
# Automatically generated from 3rdparty/libraries/freetype/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/libraries/freetype/freetype.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB)
INCPATH =	-I$(MOC_DIR) -I$(QTDIR)/src/3rdparty/freetype/src -I$(QTDIR)/src/3rdparty/freetype/include -I$(QTDIR)/src/3rdparty/freetype/builds/unix
LFLAGS	=	$(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS)  $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib$(PROJMAK)/
VER_MAJ = 6
VER_MIN = 0
VER_PATCH = 1
TARGET	= freetype
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	
SOURCES =	$(QTDIR)/src/3rdparty/freetype/builds/unix/ftsystem.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftdebug.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftinit.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftbase.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftglyph.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftmm.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftbbox.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/autohint.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcache.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/cff.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/type1cid.c \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/psaux.c \
		$(QTDIR)/src/3rdparty/freetype/src/psnames/psmodule.c \
		$(QTDIR)/src/3rdparty/freetype/src/raster/raster.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/sfnt.c \
		$(QTDIR)/src/3rdparty/freetype/src/smooth/smooth.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/truetype.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/type1.c \
		$(QTDIR)/src/3rdparty/freetype/src/winfonts/winfnt.c
OBJECTS =	$(OBJECTS_DIR)/ftsystem.o \
		$(OBJECTS_DIR)/ftdebug.o \
		$(OBJECTS_DIR)/ftinit.o \
		$(OBJECTS_DIR)/ftbase.o \
		$(OBJECTS_DIR)/ftglyph.o \
		$(OBJECTS_DIR)/ftmm.o \
		$(OBJECTS_DIR)/ftbbox.o \
		$(OBJECTS_DIR)/autohint.o \
		$(OBJECTS_DIR)/ftcache.o \
		$(OBJECTS_DIR)/cff.o \
		$(OBJECTS_DIR)/type1cid.o \
		$(OBJECTS_DIR)/psaux.o \
		$(OBJECTS_DIR)/psmodule.o \
		$(OBJECTS_DIR)/raster.o \
		$(OBJECTS_DIR)/sfnt.o \
		$(OBJECTS_DIR)/smooth.o \
		$(OBJECTS_DIR)/truetype.o \
		$(OBJECTS_DIR)/type1.o \
		$(OBJECTS_DIR)/winfnt.o
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
	tmake freetype.pro

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

$(OBJECTS_DIR)/ftsystem.o: $(QTDIR)/src/3rdparty/freetype/builds/unix/ftsystem.c \
		$(QTDIR)/src/3rdparty/freetype/builds/unix/ftconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftsystem.o $(QTDIR)/src/3rdparty/freetype/builds/unix/ftsystem.c

$(OBJECTS_DIR)/ftdebug.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftdebug.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftdebug.o $(QTDIR)/src/3rdparty/freetype/src/base/ftdebug.c

$(OBJECTS_DIR)/ftinit.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftinit.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftinit.o $(QTDIR)/src/3rdparty/freetype/src/base/ftinit.c

$(OBJECTS_DIR)/ftbase.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftbase.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftcalc.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftobjs.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftstream.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftlist.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftoutln.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftextend.c \
		$(QTDIR)/src/3rdparty/freetype/src/base/ftnames.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftbase.o $(QTDIR)/src/3rdparty/freetype/src/base/ftbase.c

$(OBJECTS_DIR)/ftglyph.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftglyph.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftglyph.o $(QTDIR)/src/3rdparty/freetype/src/base/ftglyph.c

$(OBJECTS_DIR)/ftmm.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftmm.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftmm.o $(QTDIR)/src/3rdparty/freetype/src/base/ftmm.c

$(OBJECTS_DIR)/ftbbox.o: $(QTDIR)/src/3rdparty/freetype/src/base/ftbbox.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftbbox.o $(QTDIR)/src/3rdparty/freetype/src/base/ftbbox.c

$(OBJECTS_DIR)/autohint.o: $(QTDIR)/src/3rdparty/freetype/src/autohint/autohint.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahangles.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahangles.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahtypes.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahloader.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahglyph.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahglyph.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahglobal.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahglobal.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahhint.c \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahhint.h \
		$(QTDIR)/src/3rdparty/freetype/src/autohint/ahmodule.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/autohint.o $(QTDIR)/src/3rdparty/freetype/src/autohint/autohint.c

$(OBJECTS_DIR)/ftcache.o: $(QTDIR)/src/3rdparty/freetype/src/cache/ftcache.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftlru.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcmanag.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcglyph.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcchunk.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcimage.c \
		$(QTDIR)/src/3rdparty/freetype/src/cache/ftcsbits.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ftcache.o $(QTDIR)/src/3rdparty/freetype/src/cache/ftcache.c

$(OBJECTS_DIR)/cff.o: $(QTDIR)/src/3rdparty/freetype/src/cff/cff.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2driver.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2driver.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2gload.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2objs.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2parse.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2parse.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2tokens.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2load.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2load.h \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2objs.c \
		$(QTDIR)/src/3rdparty/freetype/src/cff/t2gload.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/cff.o $(QTDIR)/src/3rdparty/freetype/src/cff/cff.c

$(OBJECTS_DIR)/type1cid.o: $(QTDIR)/src/3rdparty/freetype/src/cid/type1cid.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidparse.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidparse.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidload.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidload.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidtokens.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidobjs.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidgload.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidobjs.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidriver.c \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidriver.h \
		$(QTDIR)/src/3rdparty/freetype/src/cid/cidgload.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/type1cid.o $(QTDIR)/src/3rdparty/freetype/src/cid/type1cid.c

$(OBJECTS_DIR)/psaux.o: $(QTDIR)/src/3rdparty/freetype/src/psaux/psaux.c \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/psobjs.c \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/psobjs.h \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/psauxmod.c \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/t1decode.c \
		$(QTDIR)/src/3rdparty/freetype/src/psaux/t1decode.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/psaux.o $(QTDIR)/src/3rdparty/freetype/src/psaux/psaux.c

$(OBJECTS_DIR)/psmodule.o: $(QTDIR)/src/3rdparty/freetype/src/psnames/psmodule.c \
		$(QTDIR)/src/3rdparty/freetype/src/psnames/psmodule.h \
		$(QTDIR)/src/3rdparty/freetype/src/psnames/pstables.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/psmodule.o $(QTDIR)/src/3rdparty/freetype/src/psnames/psmodule.c

$(OBJECTS_DIR)/raster.o: $(QTDIR)/src/3rdparty/freetype/src/raster/raster.c \
		$(QTDIR)/src/3rdparty/freetype/src/raster/ftraster.c \
		$(QTDIR)/src/3rdparty/freetype/src/raster/ftraster.h \
		$(QTDIR)/src/3rdparty/freetype/src/raster/ftrend1.c \
		$(QTDIR)/src/3rdparty/freetype/src/raster/ftrend1.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/raster.o $(QTDIR)/src/3rdparty/freetype/src/raster/raster.c

$(OBJECTS_DIR)/sfnt.o: $(QTDIR)/src/3rdparty/freetype/src/sfnt/sfnt.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttload.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttload.h \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttcmap.h \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttcmap.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/sfobjs.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/sfobjs.h \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttsbit.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttsbit.h \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttpost.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/ttpost.h \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/sfdriver.c \
		$(QTDIR)/src/3rdparty/freetype/src/sfnt/sfdriver.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/sfnt.o $(QTDIR)/src/3rdparty/freetype/src/sfnt/sfnt.c

$(OBJECTS_DIR)/smooth.o: $(QTDIR)/src/3rdparty/freetype/src/smooth/smooth.c \
		$(QTDIR)/src/3rdparty/freetype/src/smooth/ftgrays.c \
		$(QTDIR)/src/3rdparty/freetype/src/smooth/ftgrays.h \
		$(QTDIR)/src/3rdparty/freetype/src/smooth/ftsmooth.c \
		$(QTDIR)/src/3rdparty/freetype/src/smooth/ftsmooth.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/smooth.o $(QTDIR)/src/3rdparty/freetype/src/smooth/smooth.c

$(OBJECTS_DIR)/truetype.o: $(QTDIR)/src/3rdparty/freetype/src/truetype/truetype.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttdriver.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttdriver.h \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttgload.h \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttobjs.h \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttinterp.h \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttpload.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttpload.h \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttgload.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttobjs.c \
		$(QTDIR)/src/3rdparty/freetype/src/truetype/ttinterp.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/truetype.o $(QTDIR)/src/3rdparty/freetype/src/truetype/truetype.c

$(OBJECTS_DIR)/type1.o: $(QTDIR)/src/3rdparty/freetype/src/type1/type1.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1parse.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1parse.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1load.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1load.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1tokens.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1objs.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1gload.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1objs.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1afm.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1driver.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1driver.h \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1gload.c \
		$(QTDIR)/src/3rdparty/freetype/src/type1/t1afm.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/type1.o $(QTDIR)/src/3rdparty/freetype/src/type1/type1.c

$(OBJECTS_DIR)/winfnt.o: $(QTDIR)/src/3rdparty/freetype/src/winfonts/winfnt.c \
		$(QTDIR)/src/3rdparty/freetype/src/winfonts/winfnt.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/winfnt.o $(QTDIR)/src/3rdparty/freetype/src/winfonts/winfnt.c



lupdate:
	sed -e 's/$$$$QTOPIA1DIR[^ ]*//' 	    -e 's/^static:.*//' 	    <freetype.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease freetype.pro
	$(QPEDIR)/bin/qtopia-linstall freetype.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: freetype.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/libraries/freetype
