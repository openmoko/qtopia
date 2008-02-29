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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB)
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB)
INCPATH =	-I$(MOC_DIR) -I./openobex
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lopenobex -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
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
		sendwindow.h
SOURCES =	qir.cpp \
		qirserver.cpp \
		qobex.cpp \
		receivedialog.cpp \
		sendwindow.cpp
OBJECTS =	$(OBJECTS_DIR)/qir.o \
		$(OBJECTS_DIR)/qirserver.o \
		$(OBJECTS_DIR)/qobex.o \
		$(OBJECTS_DIR)/receivedialog.o \
		$(OBJECTS_DIR)/sendwindow.o \
		$(OBJECTS_DIR)/receivedialogbase.o \
		$(OBJECTS_DIR)/sendwidgetbase.o
INTERFACES =    receivedialogbase.ui \
		sendwidgetbase.ui
UICDECLS =      receivedialogbase.h \
		sendwidgetbase.h
UICIMPLS =      receivedialogbase.cpp \
		sendwidgetbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_qir.cpp \
		$(MOC_DIR)/moc_qirserver.cpp \
		$(MOC_DIR)/moc_receivedialog.cpp \
		$(MOC_DIR)/moc_sendwindow.cpp \
		$(MOC_DIR)/moc_receivedialogbase.cpp \
		$(MOC_DIR)/moc_sendwidgetbase.cpp \
		$(MOC_DIR)/qirserver.moc
OBJMOC	=	$(OBJECTS_DIR)/moc_qir.o \
		$(OBJECTS_DIR)/moc_qirserver.o \
		$(OBJECTS_DIR)/moc_receivedialog.o \
		$(OBJECTS_DIR)/moc_sendwindow.o \
		$(OBJECTS_DIR)/moc_receivedialogbase.o \
		$(OBJECTS_DIR)/moc_sendwidgetbase.o


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

$(OBJECTS_DIR)/qir.o: qir.cpp \
		qir.h \
		qirserver.h \
		receivedialog.h \
		receivedialogbase.h \
		sendwindow.h \
		sendwidgetbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qir.o qir.cpp

$(OBJECTS_DIR)/qirserver.o: qirserver.cpp \
		$(MOC_DIR)/qirserver.moc \
		qirserver.h \
		openobex/obex.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qirserver.o qirserver.cpp

$(OBJECTS_DIR)/qobex.o: qobex.cpp \
		qobex.h \
		../../../server/obexinterface.h \
		qir.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qobex.o qobex.cpp

$(OBJECTS_DIR)/receivedialog.o: receivedialog.cpp \
		receivedialog.h \
		receivedialogbase.h \
		qirserver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/receivedialog.o receivedialog.cpp

$(OBJECTS_DIR)/sendwindow.o: sendwindow.cpp \
		sendwindow.h \
		sendwidgetbase.h \
		qirserver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/sendwindow.o sendwindow.cpp

receivedialogbase.h: receivedialogbase.ui
	$(UIC) receivedialogbase.ui -o $(INTERFACE_DECL_PATH)/receivedialogbase.h

receivedialogbase.cpp: receivedialogbase.ui
	$(UIC) receivedialogbase.ui -i receivedialogbase.h -o receivedialogbase.cpp

sendwidgetbase.h: sendwidgetbase.ui
	$(UIC) sendwidgetbase.ui -o $(INTERFACE_DECL_PATH)/sendwidgetbase.h

sendwidgetbase.cpp: sendwidgetbase.ui
	$(UIC) sendwidgetbase.ui -i sendwidgetbase.h -o sendwidgetbase.cpp

$(OBJECTS_DIR)/receivedialogbase.o: receivedialogbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/receivedialogbase.o receivedialogbase.cpp

$(OBJECTS_DIR)/sendwidgetbase.o: sendwidgetbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/sendwidgetbase.o sendwidgetbase.cpp

$(OBJECTS_DIR)/moc_qir.o: $(MOC_DIR)/moc_qir.cpp \
		qir.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qir.o $(MOC_DIR)/moc_qir.cpp

$(OBJECTS_DIR)/moc_qirserver.o: $(MOC_DIR)/moc_qirserver.cpp \
		qirserver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qirserver.o $(MOC_DIR)/moc_qirserver.cpp

$(OBJECTS_DIR)/moc_receivedialog.o: $(MOC_DIR)/moc_receivedialog.cpp \
		receivedialog.h \
		receivedialogbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_receivedialog.o $(MOC_DIR)/moc_receivedialog.cpp

$(OBJECTS_DIR)/moc_sendwindow.o: $(MOC_DIR)/moc_sendwindow.cpp \
		sendwindow.h \
		sendwidgetbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_sendwindow.o $(MOC_DIR)/moc_sendwindow.cpp

$(OBJECTS_DIR)/moc_receivedialogbase.o: $(MOC_DIR)/moc_receivedialogbase.cpp \
		receivedialogbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_receivedialogbase.o $(MOC_DIR)/moc_receivedialogbase.cpp

$(OBJECTS_DIR)/moc_sendwidgetbase.o: $(MOC_DIR)/moc_sendwidgetbase.cpp \
		sendwidgetbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_sendwidgetbase.o $(MOC_DIR)/moc_sendwidgetbase.cpp

$(MOC_DIR)/moc_qir.cpp: qir.h
	$(MOC) qir.h -o $(MOC_DIR)/moc_qir.cpp

$(MOC_DIR)/moc_qirserver.cpp: qirserver.h
	$(MOC) qirserver.h -o $(MOC_DIR)/moc_qirserver.cpp

$(MOC_DIR)/moc_receivedialog.cpp: receivedialog.h
	$(MOC) receivedialog.h -o $(MOC_DIR)/moc_receivedialog.cpp

$(MOC_DIR)/moc_sendwindow.cpp: sendwindow.h
	$(MOC) sendwindow.h -o $(MOC_DIR)/moc_sendwindow.cpp

$(MOC_DIR)/qirserver.moc: qirserver.cpp
	$(MOC) qirserver.cpp -o $(MOC_DIR)/qirserver.moc

$(MOC_DIR)/moc_receivedialogbase.cpp: receivedialogbase.h
	$(MOC) receivedialogbase.h -o $(MOC_DIR)/moc_receivedialogbase.cpp

$(MOC_DIR)/moc_sendwidgetbase.cpp: sendwidgetbase.h
	$(MOC) sendwidgetbase.h -o $(MOC_DIR)/moc_sendwidgetbase.cpp



lupdate:
	sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' 	    -e '/$$(.*/ d' 	    -e 's/^static:.*//' 	    <obex.pro >t$$$$.pro ;	lupdate  t$$$$.pro ;	rm t$$$$.pro

lrelease:
	lrelease obex.pro
	$(QPEDIR)/bin/qtopia-linstall obex.pro

ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: obex.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/plugins/obex
