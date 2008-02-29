#############################################################################
# Automatically generated from applications/addressbook/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = applications/addressbook/qpe-addressbook.control

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
INCPATH =	-I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpe -lqpepim $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= addressbook
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	addressbook.h \
		abeditor.h \
		ablabel.h \
		abtable.h \
		addresssettings.h
SOURCES =	main.cpp \
		addressbook.cpp \
		abeditor.cpp \
		ablabel.cpp \
		abtable.cpp \
		addresssettings.cpp
OBJECTS =	main.o \
		addressbook.o \
		abeditor.o \
		ablabel.o \
		abtable.o \
		addresssettings.o \
		addresssettingsbase.o
INTERFACES =    addresssettingsbase.ui
UICDECLS =      addresssettingsbase.h
UICIMPLS =      addresssettingsbase.cpp
SRCMOC	=	moc_addressbook.cpp \
		moc_abeditor.cpp \
		moc_ablabel.cpp \
		moc_abtable.cpp \
		moc_addresssettings.cpp \
		moc_addresssettingsbase.cpp
OBJMOC	=	moc_addressbook.o \
		moc_abeditor.o \
		moc_ablabel.o \
		moc_abtable.o \
		moc_addresssettings.o \
		moc_addresssettingsbase.o


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
	tmake addressbook.pro

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

main.o: main.cpp \
		addressbook.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

addressbook.o: addressbook.cpp \
		abeditor.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		ablabel.h \
		abtable.h \
		$(QPEDIR)/include/qpe/categories.h \
		$(QPEDIR)/include/qtopia/private/palmtopuidgen.h \
		$(QPEDIR)/include/qpe/pim/private/contactxmlio_p.h \
		$(QPEDIR)/include/qtopia/pim/contact.h \
		addresssettings.h \
		addresssettingsbase.h \
		addressbook.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/global.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/ir.h \
		$(QPEDIR)/include/qtopia/qpemessagebox.h \
		$(QPEDIR)/include/qtopia/categoryselect.h \
		$(QPEDIR)/include/qtopia/categories.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/qpemenubar.h \
		$(QPEDIR)/include/qtopia/qpetoolbar.h

abeditor.o: abeditor.cpp \
		abeditor.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		addresspicker.h \
		$(QPEDIR)/include/qpe/categoryselect.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qpe/qpedialog.h

ablabel.o: ablabel.cpp \
		ablabel.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		$(QPEDIR)/include/qpe/stringutil.h

abtable.o: abtable.cpp \
		$(QPEDIR)/include/qpe/categoryselect.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/stringutil.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		abtable.h \
		$(QPEDIR)/include/qpe/categories.h \
		$(QPEDIR)/include/qtopia/private/palmtopuidgen.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		$(QPEDIR)/include/qpe/pim/private/contactxmlio_p.h \
		$(QPEDIR)/include/qtopia/pim/contact.h

addresssettings.o: addresssettings.cpp \
		addresssettings.h \
		addresssettingsbase.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h

addresssettingsbase.h: addresssettingsbase.ui
	$(UIC) addresssettingsbase.ui -o $(INTERFACE_DECL_PATH)/addresssettingsbase.h

addresssettingsbase.cpp: addresssettingsbase.ui
	$(UIC) addresssettingsbase.ui -i addresssettingsbase.h -o addresssettingsbase.cpp

addresssettingsbase.o: addresssettingsbase.cpp \
		addresssettingsbase.h \
		addresssettingsbase.ui

moc_addressbook.o: moc_addressbook.cpp \
		addressbook.h

moc_abeditor.o: moc_abeditor.cpp \
		abeditor.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h

moc_ablabel.o: moc_ablabel.cpp \
		ablabel.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h

moc_abtable.o: moc_abtable.cpp \
		abtable.h \
		$(QPEDIR)/include/qpe/categories.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/private/palmtopuidgen.h \
		$(QPEDIR)/include/qpe/pim/contact.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		$(QPEDIR)/include/qpe/pim/private/contactxmlio_p.h \
		$(QPEDIR)/include/qtopia/pim/contact.h

moc_addresssettings.o: moc_addresssettings.cpp \
		addresssettings.h \
		addresssettingsbase.h

moc_addresssettingsbase.o: moc_addresssettingsbase.cpp \
		addresssettingsbase.h

moc_addressbook.cpp: addressbook.h
	$(MOC) addressbook.h -o moc_addressbook.cpp

moc_abeditor.cpp: abeditor.h
	$(MOC) abeditor.h -o moc_abeditor.cpp

moc_ablabel.cpp: ablabel.h
	$(MOC) ablabel.h -o moc_ablabel.cpp

moc_abtable.cpp: abtable.h
	$(MOC) abtable.h -o moc_abtable.cpp

moc_addresssettings.cpp: addresssettings.h
	$(MOC) addresssettings.h -o moc_addresssettings.cpp

moc_addresssettingsbase.cpp: addresssettingsbase.h
	$(MOC) addresssettingsbase.h -o moc_addresssettingsbase.cpp



lupdate:
	lupdate addressbook.pro

lrelease:
	lrelease addressbook.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: addressbook.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/addressbook
