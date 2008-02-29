#############################################################################
# Automatically generated from libraries/qtopiapim/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = libraries/qtopiapim/qpe-pim.control

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
INCPATH =	-I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqtopiaservices -lqpe -luuid $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib$(PROJMAK)/
VER_MAJ = 1
VER_MIN = 6
VER_PATCH = 0
TARGET	= qpepim
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	task.h \
		event.h \
		calendar.h \
		contact.h \
		pimrecord.h \
		xmlio_p.h \
		addressbookaccess.h \
		contactio_p.h \
		contactxmlio_p.h \
		todoaccess.h \
		taskio_p.h \
		todoxmlio_p.h \
		datebookaccess.h \
		eventio_p.h \
		eventxmlio_p.h
SOURCES =	task.cpp \
		event.cpp \
		calendar.cpp \
		contact.cpp \
		pimrecord.cpp \
		xmlio.cpp \
		addressbookaccess.cpp \
		contactxmlio.cpp \
		todoaccess.cpp \
		todoxmlio.cpp \
		datebookaccess.cpp \
		eventxmlio.cpp
OBJECTS =	task.o \
		event.o \
		calendar.o \
		contact.o \
		pimrecord.o \
		xmlio.o \
		addressbookaccess.o \
		contactxmlio.o \
		todoaccess.o \
		todoxmlio.o \
		datebookaccess.o \
		eventxmlio.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	moc_addressbookaccess.cpp \
		moc_contactio_p.cpp \
		moc_contactxmlio_p.cpp \
		moc_todoaccess.cpp \
		moc_taskio_p.cpp \
		moc_todoxmlio_p.cpp \
		moc_datebookaccess.cpp \
		moc_eventio_p.cpp \
		moc_eventxmlio_p.cpp
OBJMOC	=	moc_addressbookaccess.o \
		moc_contactio_p.o \
		moc_contactxmlio_p.o \
		moc_todoaccess.o \
		moc_taskio_p.o \
		moc_todoxmlio_p.o \
		moc_datebookaccess.o \
		moc_eventio_p.o \
		moc_eventxmlio_p.o


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
	tmake qtopiapim.pro

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

task.o: task.cpp \
		task.h

event.o: event.cpp \
		event.h \
		calendar.h

calendar.o: calendar.cpp \
		calendar.h

contact.o: contact.cpp \
		contact.h

pimrecord.o: pimrecord.cpp \
		pimrecord.h

xmlio.o: xmlio.cpp \
		xmlio_p.h

addressbookaccess.o: addressbookaccess.cpp \
		contact.h \
		addressbookaccess.h \
		contactxmlio_p.h \
		contactio_p.h \
		xmlio_p.h

contactxmlio.o: contactxmlio.cpp \
		contact.h \
		contactxmlio_p.h \
		contactio_p.h \
		xmlio_p.h

todoaccess.o: todoaccess.cpp \
		task.h \
		todoaccess.h \
		todoxmlio_p.h \
		taskio_p.h \
		xmlio_p.h

todoxmlio.o: todoxmlio.cpp \
		task.h \
		todoxmlio_p.h \
		taskio_p.h \
		xmlio_p.h

datebookaccess.o: datebookaccess.cpp \
		event.h \
		datebookaccess.h \
		eventxmlio_p.h \
		eventio_p.h \
		xmlio_p.h

eventxmlio.o: eventxmlio.cpp \
		eventxmlio_p.h \
		eventio_p.h \
		xmlio_p.h \
		event.h

moc_addressbookaccess.o: moc_addressbookaccess.cpp \
		addressbookaccess.h \
		contact.h

moc_contactio_p.o: moc_contactio_p.cpp \
		contactio_p.h

moc_contactxmlio_p.o: moc_contactxmlio_p.cpp \
		contactxmlio_p.h \
		contactio_p.h \
		xmlio_p.h

moc_todoaccess.o: moc_todoaccess.cpp \
		todoaccess.h \
		task.h

moc_taskio_p.o: moc_taskio_p.cpp \
		taskio_p.h

moc_todoxmlio_p.o: moc_todoxmlio_p.cpp \
		todoxmlio_p.h \
		taskio_p.h \
		xmlio_p.h

moc_datebookaccess.o: moc_datebookaccess.cpp \
		datebookaccess.h

moc_eventio_p.o: moc_eventio_p.cpp \
		eventio_p.h

moc_eventxmlio_p.o: moc_eventxmlio_p.cpp \
		eventxmlio_p.h \
		eventio_p.h \
		xmlio_p.h

moc_addressbookaccess.cpp: addressbookaccess.h
	$(MOC) addressbookaccess.h -o moc_addressbookaccess.cpp

moc_contactio_p.cpp: contactio_p.h
	$(MOC) contactio_p.h -o moc_contactio_p.cpp

moc_contactxmlio_p.cpp: contactxmlio_p.h
	$(MOC) contactxmlio_p.h -o moc_contactxmlio_p.cpp

moc_todoaccess.cpp: todoaccess.h
	$(MOC) todoaccess.h -o moc_todoaccess.cpp

moc_taskio_p.cpp: taskio_p.h
	$(MOC) taskio_p.h -o moc_taskio_p.cpp

moc_todoxmlio_p.cpp: todoxmlio_p.h
	$(MOC) todoxmlio_p.h -o moc_todoxmlio_p.cpp

moc_datebookaccess.cpp: datebookaccess.h
	$(MOC) datebookaccess.h -o moc_datebookaccess.cpp

moc_eventio_p.cpp: eventio_p.h
	$(MOC) eventio_p.h -o moc_eventio_p.cpp

moc_eventxmlio_p.cpp: eventxmlio_p.h
	$(MOC) eventxmlio_p.h -o moc_eventxmlio_p.cpp



lupdate:
	lupdate qtopiapim.pro

lrelease:
	lrelease qtopiapim.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: qtopiapim.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make libraries/qtopiapim
