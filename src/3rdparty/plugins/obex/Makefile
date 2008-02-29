#############################################################################
# Automatically generated from 3rdparty/plugins/obex/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/plugins/obex/qpe-obex.control

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
INCPATH =	-I./openobex -I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lopenobex -lqtopiaservices -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/obex/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= qobex
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	qir.h \
		qirserver.h \
		qobex.h \
		receivedialog.h \
		senddialog.h
SOURCES =	qir.cpp \
		qirserver.cpp \
		qobex.cpp \
		receivedialog.cpp \
		senddialog.cpp
OBJECTS =	qir.o \
		qirserver.o \
		qobex.o \
		receivedialog.o \
		senddialog.o \
		receivedialogbase.o \
		senddialogbase.o
INTERFACES =    receivedialogbase.ui \
		senddialogbase.ui
UICDECLS =      receivedialogbase.h \
		senddialogbase.h
UICIMPLS =      receivedialogbase.cpp \
		senddialogbase.cpp
SRCMOC	=	moc_qir.cpp \
		moc_qirserver.cpp \
		moc_receivedialog.cpp \
		moc_senddialog.cpp \
		qirserver.moc \
		moc_receivedialogbase.cpp \
		moc_senddialogbase.cpp
OBJMOC	=	moc_qir.o \
		moc_qirserver.o \
		moc_receivedialog.o \
		moc_senddialog.o \
		moc_receivedialogbase.o \
		moc_senddialogbase.o


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
	tmake obex.pro

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

qir.o: qir.cpp \
		qir.h \
		qirserver.h \
		receivedialog.h \
		receivedialogbase.h \
		senddialog.h \
		senddialogbase.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

qirserver.o: qirserver.cpp \
		qirserver.moc \
		qirserver.h \
		$(QPEDIR)/include/qtopia/mimetype.h \
		openobex/obex.h

qobex.o: qobex.cpp \
		qobex.h \
		../../../server/obexinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		qir.h

receivedialog.o: receivedialog.cpp \
		receivedialog.h \
		receivedialogbase.h \
		qirserver.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		$(QPEDIR)/include/qtopia/mimetype.h \
		$(QPEDIR)/include/qtopia/filemanager.h \
		$(QPEDIR)/include/qtopia/global.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/private/contact.h \
		$(QPEDIR)/include/qtopia/private/palmtoprecord.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		$(QPEDIR)/include/qtopia/private/event.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		$(QPEDIR)/include/qtopia/private/task.h \
		$(QPEDIR)/include/qtopia/stringutil.h \
		$(QPEDIR)/include/qtopia/services/services.h

senddialog.o: senddialog.cpp \
		senddialog.h \
		senddialogbase.h \
		qirserver.h

receivedialogbase.h: receivedialogbase.ui
	$(UIC) receivedialogbase.ui -o $(INTERFACE_DECL_PATH)/receivedialogbase.h

receivedialogbase.cpp: receivedialogbase.ui
	$(UIC) receivedialogbase.ui -i receivedialogbase.h -o receivedialogbase.cpp

senddialogbase.h: senddialogbase.ui
	$(UIC) senddialogbase.ui -o $(INTERFACE_DECL_PATH)/senddialogbase.h

senddialogbase.cpp: senddialogbase.ui
	$(UIC) senddialogbase.ui -i senddialogbase.h -o senddialogbase.cpp

receivedialogbase.o: receivedialogbase.cpp

senddialogbase.o: senddialogbase.cpp

moc_qir.o: moc_qir.cpp \
		qir.h

moc_qirserver.o: moc_qirserver.cpp \
		qirserver.h

moc_receivedialog.o: moc_receivedialog.cpp \
		receivedialog.h \
		receivedialogbase.h

moc_senddialog.o: moc_senddialog.cpp \
		senddialog.h \
		senddialogbase.h

moc_receivedialogbase.o: qirserver.moc \
		qirserver.cpp \
		qirserver.h \
		$(QPEDIR)/include/qtopia/mimetype.h \
		openobex/obex.h

moc_senddialogbase.o: moc_receivedialogbase.cpp \
		receivedialogbase.h

moc_qir.cpp: qir.h
	$(MOC) qir.h -o moc_qir.cpp

moc_qirserver.cpp: qirserver.h
	$(MOC) qirserver.h -o moc_qirserver.cpp

moc_receivedialog.cpp: receivedialog.h
	$(MOC) receivedialog.h -o moc_receivedialog.cpp

moc_senddialog.cpp: senddialog.h
	$(MOC) senddialog.h -o moc_senddialog.cpp

qirserver.moc: qirserver.cpp
	$(MOC) qirserver.cpp -o qirserver.moc

moc_receivedialogbase.cpp: receivedialogbase.h
	$(MOC) receivedialogbase.h -o moc_receivedialogbase.cpp

moc_senddialogbase.cpp: senddialogbase.h
	$(MOC) senddialogbase.h -o moc_senddialogbase.cpp



lupdate:
	lupdate obex.pro

lrelease:
	lrelease obex.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: obex.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/plugins/obex
