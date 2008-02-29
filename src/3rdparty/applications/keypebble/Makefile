#############################################################################
# Automatically generated from 3rdparty/applications/keypebble/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = 3rdparty/applications/keypebble/qpe-keypebble.control

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
TARGET	= keypebble
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	d3des.h \
		krfbbuffer.h \
		krfbcanvas.h \
		krfbconnection.h \
		krfbdecoder.h \
		krfblogin.h \
		krfboptions.h \
		krfbserverinfo.h \
		kvnc.h \
		kvncconnectdlg.h \
		kvncoptionsdlg.h \
		vncauth.h
SOURCES =	d3des.c \
		vncauth.c \
		krfbbuffer.cpp \
		krfbcanvas.cpp \
		krfbconnection.cpp \
		krfbdecoder.cpp \
		krfblogin.cpp \
		krfboptions.cpp \
		kvnc.cpp \
		kvncconnectdlg.cpp \
		kvncoptionsdlg.cpp \
		main.cpp
OBJECTS =	$(OBJECTS_DIR)/d3des.o \
		$(OBJECTS_DIR)/vncauth.o \
		$(OBJECTS_DIR)/krfbbuffer.o \
		$(OBJECTS_DIR)/krfbcanvas.o \
		$(OBJECTS_DIR)/krfbconnection.o \
		$(OBJECTS_DIR)/krfbdecoder.o \
		$(OBJECTS_DIR)/krfblogin.o \
		$(OBJECTS_DIR)/krfboptions.o \
		$(OBJECTS_DIR)/kvnc.o \
		$(OBJECTS_DIR)/kvncconnectdlg.o \
		$(OBJECTS_DIR)/kvncoptionsdlg.o \
		$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/vncoptionsbase.o
INTERFACES =    vncoptionsbase.ui
UICDECLS =      vncoptionsbase.h
UICIMPLS =      vncoptionsbase.cpp
SRCMOC	=	$(MOC_DIR)/moc_krfbbuffer.cpp \
		$(MOC_DIR)/moc_krfbcanvas.cpp \
		$(MOC_DIR)/moc_krfbconnection.cpp \
		$(MOC_DIR)/moc_krfbdecoder.cpp \
		$(MOC_DIR)/moc_krfblogin.cpp \
		$(MOC_DIR)/moc_kvnc.cpp \
		$(MOC_DIR)/moc_kvncconnectdlg.cpp \
		$(MOC_DIR)/moc_kvncoptionsdlg.cpp \
		$(MOC_DIR)/moc_vncoptionsbase.cpp
OBJMOC	=	$(OBJECTS_DIR)/moc_krfbbuffer.o \
		$(OBJECTS_DIR)/moc_krfbcanvas.o \
		$(OBJECTS_DIR)/moc_krfbconnection.o \
		$(OBJECTS_DIR)/moc_krfbdecoder.o \
		$(OBJECTS_DIR)/moc_krfblogin.o \
		$(OBJECTS_DIR)/moc_kvnc.o \
		$(OBJECTS_DIR)/moc_kvncconnectdlg.o \
		$(OBJECTS_DIR)/moc_kvncoptionsdlg.o \
		$(OBJECTS_DIR)/moc_vncoptionsbase.o


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
	tmake keypebble.pro

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

$(OBJECTS_DIR)/d3des.o: d3des.c \
		d3des.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/d3des.o d3des.c

$(OBJECTS_DIR)/vncauth.o: vncauth.c \
		vncauth.h \
		d3des.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/vncauth.o vncauth.c

$(OBJECTS_DIR)/krfbbuffer.o: krfbbuffer.cpp \
		krfbdecoder.h \
		krfbbuffer.h \
		krfbserverinfo.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfbbuffer.o krfbbuffer.cpp

$(OBJECTS_DIR)/krfbcanvas.o: krfbcanvas.cpp \
		kvncconnectdlg.h \
		krfbconnection.h \
		krfbcanvas.h \
		krfboptions.h \
		krfbbuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfbcanvas.o krfbcanvas.cpp

$(OBJECTS_DIR)/krfbconnection.o: krfbconnection.cpp \
		krfbconnection.h \
		krfblogin.h \
		krfboptions.h \
		krfbdecoder.h \
		krfbbuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfbconnection.o krfbconnection.cpp

$(OBJECTS_DIR)/krfbdecoder.o: krfbdecoder.cpp \
		krfbconnection.h \
		krfboptions.h \
		krfbserverinfo.h \
		krfbdecoder.h \
		krfbbuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfbdecoder.o krfbdecoder.cpp

$(OBJECTS_DIR)/krfblogin.o: krfblogin.cpp \
		vncauth.h \
		krfblogin.h \
		krfbconnection.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfblogin.o krfblogin.cpp

$(OBJECTS_DIR)/krfboptions.o: krfboptions.cpp \
		krfboptions.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/krfboptions.o krfboptions.cpp

$(OBJECTS_DIR)/kvnc.o: kvnc.cpp \
		kvnc.h \
		krfbcanvas.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h \
		krfbconnection.h \
		krfboptions.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/kvnc.o kvnc.cpp

$(OBJECTS_DIR)/kvncconnectdlg.o: kvncconnectdlg.cpp \
		krfbconnection.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h \
		kvncconnectdlg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/kvncconnectdlg.o kvncconnectdlg.cpp

$(OBJECTS_DIR)/kvncoptionsdlg.o: kvncoptionsdlg.cpp \
		krfboptions.h \
		kvncoptionsdlg.h \
		vncoptionsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/kvncoptionsdlg.o kvncoptionsdlg.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		kvnc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

vncoptionsbase.h: vncoptionsbase.ui
	$(UIC) vncoptionsbase.ui -o $(INTERFACE_DECL_PATH)/vncoptionsbase.h

vncoptionsbase.cpp: vncoptionsbase.ui
	$(UIC) vncoptionsbase.ui -i vncoptionsbase.h -o vncoptionsbase.cpp

$(OBJECTS_DIR)/vncoptionsbase.o: vncoptionsbase.cpp \
		vncoptionsbase.h \
		vncoptionsbase.ui
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/vncoptionsbase.o vncoptionsbase.cpp

$(OBJECTS_DIR)/moc_krfbbuffer.o: $(MOC_DIR)/moc_krfbbuffer.cpp \
		krfbbuffer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_krfbbuffer.o $(MOC_DIR)/moc_krfbbuffer.cpp

$(OBJECTS_DIR)/moc_krfbcanvas.o: $(MOC_DIR)/moc_krfbcanvas.cpp \
		krfbcanvas.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_krfbcanvas.o $(MOC_DIR)/moc_krfbcanvas.cpp

$(OBJECTS_DIR)/moc_krfbconnection.o: $(MOC_DIR)/moc_krfbconnection.cpp \
		krfbconnection.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_krfbconnection.o $(MOC_DIR)/moc_krfbconnection.cpp

$(OBJECTS_DIR)/moc_krfbdecoder.o: $(MOC_DIR)/moc_krfbdecoder.cpp \
		krfbdecoder.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_krfbdecoder.o $(MOC_DIR)/moc_krfbdecoder.cpp

$(OBJECTS_DIR)/moc_krfblogin.o: $(MOC_DIR)/moc_krfblogin.cpp \
		krfblogin.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_krfblogin.o $(MOC_DIR)/moc_krfblogin.cpp

$(OBJECTS_DIR)/moc_kvnc.o: $(MOC_DIR)/moc_kvnc.cpp \
		kvnc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_kvnc.o $(MOC_DIR)/moc_kvnc.cpp

$(OBJECTS_DIR)/moc_kvncconnectdlg.o: $(MOC_DIR)/moc_kvncconnectdlg.cpp \
		kvncconnectdlg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_kvncconnectdlg.o $(MOC_DIR)/moc_kvncconnectdlg.cpp

$(OBJECTS_DIR)/moc_kvncoptionsdlg.o: $(MOC_DIR)/moc_kvncoptionsdlg.cpp \
		kvncoptionsdlg.h \
		vncoptionsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_kvncoptionsdlg.o $(MOC_DIR)/moc_kvncoptionsdlg.cpp

$(OBJECTS_DIR)/moc_vncoptionsbase.o: $(MOC_DIR)/moc_vncoptionsbase.cpp \
		vncoptionsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_vncoptionsbase.o $(MOC_DIR)/moc_vncoptionsbase.cpp

$(MOC_DIR)/moc_krfbbuffer.cpp: krfbbuffer.h
	$(MOC) krfbbuffer.h -o $(MOC_DIR)/moc_krfbbuffer.cpp

$(MOC_DIR)/moc_krfbcanvas.cpp: krfbcanvas.h
	$(MOC) krfbcanvas.h -o $(MOC_DIR)/moc_krfbcanvas.cpp

$(MOC_DIR)/moc_krfbconnection.cpp: krfbconnection.h
	$(MOC) krfbconnection.h -o $(MOC_DIR)/moc_krfbconnection.cpp

$(MOC_DIR)/moc_krfbdecoder.cpp: krfbdecoder.h
	$(MOC) krfbdecoder.h -o $(MOC_DIR)/moc_krfbdecoder.cpp

$(MOC_DIR)/moc_krfblogin.cpp: krfblogin.h
	$(MOC) krfblogin.h -o $(MOC_DIR)/moc_krfblogin.cpp

$(MOC_DIR)/moc_kvnc.cpp: kvnc.h
	$(MOC) kvnc.h -o $(MOC_DIR)/moc_kvnc.cpp

$(MOC_DIR)/moc_kvncconnectdlg.cpp: kvncconnectdlg.h
	$(MOC) kvncconnectdlg.h -o $(MOC_DIR)/moc_kvncconnectdlg.cpp

$(MOC_DIR)/moc_kvncoptionsdlg.cpp: kvncoptionsdlg.h
	$(MOC) kvncoptionsdlg.h -o $(MOC_DIR)/moc_kvncoptionsdlg.cpp

$(MOC_DIR)/moc_vncoptionsbase.cpp: vncoptionsbase.h
	$(MOC) vncoptionsbase.h -o $(MOC_DIR)/moc_vncoptionsbase.cpp



lupdate-check:
	@sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <keypebble.pro >t$$$$.pro && \
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
	    <keypebble.pro >t$$$$.pro && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro

lrelease:
	lrelease keypebble.pro
	$(QPEDIR)/bin/qtopia-linstall keypebble.pro


ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: keypebble.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make 3rdparty/applications/keypebble
