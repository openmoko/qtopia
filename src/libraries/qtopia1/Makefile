#############################################################################
# Automatically generated from libraries/qtopia1/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = libraries/qtopia1/qpe-libqtopia.control

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
INCPATH =	-I$(MOC_DIR) -I$(QPEDIR)/src/server
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -luuid -lqpe $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib$(PROJMAK)/
VER_MAJ = 1
VER_MIN = 6
VER_PATCH = 2
TARGET	= qtopia
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	accessory.h \
		datepicker.h \
		datetimeedit.h \
		fieldmapimpl.h \
		qprocess.h \
		timezone.h \
		services.h \
		devicebuttonmanager.h \
		devicebutton.h \
		qwizard.h \
		locationcombo.h \
		docproperties.h \
		pluginloader.h
SOURCES =	applnk1.cpp \
		categories1.cpp \
		categoryselect1.cpp \
		config1.cpp \
		qpeapplication1.cpp \
		storage1.cpp \
		accessory.cpp \
		calendar1.cpp \
		resource1.cpp \
		datepicker.cpp \
		datetimeedit.cpp \
		timeconversion1.cpp \
		timestring1.cpp \
		global1.cpp \
		fieldmapimpl.cpp \
		qprocess.cpp \
		timezone.cpp \
		services.cpp \
		devicebuttonmanager.cpp \
		devicebutton.cpp \
		fileselector1.cpp \
		qwizard.cpp \
		locationcombo.cpp \
		docproperties.cpp \
		pluginloader.cpp \
		pluginloaderlib.cpp \
		qprocess_unix.cpp \
		quuid1.cpp
OBJECTS =	$(OBJECTS_DIR)/applnk1.o \
		$(OBJECTS_DIR)/categories1.o \
		$(OBJECTS_DIR)/categoryselect1.o \
		$(OBJECTS_DIR)/config1.o \
		$(OBJECTS_DIR)/qpeapplication1.o \
		$(OBJECTS_DIR)/storage1.o \
		$(OBJECTS_DIR)/accessory.o \
		$(OBJECTS_DIR)/calendar1.o \
		$(OBJECTS_DIR)/resource1.o \
		$(OBJECTS_DIR)/datepicker.o \
		$(OBJECTS_DIR)/datetimeedit.o \
		$(OBJECTS_DIR)/timeconversion1.o \
		$(OBJECTS_DIR)/timestring1.o \
		$(OBJECTS_DIR)/global1.o \
		$(OBJECTS_DIR)/fieldmapimpl.o \
		$(OBJECTS_DIR)/qprocess.o \
		$(OBJECTS_DIR)/timezone.o \
		$(OBJECTS_DIR)/services.o \
		$(OBJECTS_DIR)/devicebuttonmanager.o \
		$(OBJECTS_DIR)/devicebutton.o \
		$(OBJECTS_DIR)/fileselector1.o \
		$(OBJECTS_DIR)/qwizard.o \
		$(OBJECTS_DIR)/locationcombo.o \
		$(OBJECTS_DIR)/docproperties.o \
		$(OBJECTS_DIR)/pluginloader.o \
		$(OBJECTS_DIR)/pluginloaderlib.o \
		$(OBJECTS_DIR)/qprocess_unix.o \
		$(OBJECTS_DIR)/quuid1.o
INTERFACES =    
UICDECLS =      
UICIMPLS =      
SRCMOC	=	$(MOC_DIR)/moc_accessory.cpp \
		$(MOC_DIR)/moc_datepicker.cpp \
		$(MOC_DIR)/moc_datetimeedit.cpp \
		$(MOC_DIR)/moc_fieldmapimpl.cpp \
		$(MOC_DIR)/moc_qprocess.cpp \
		$(MOC_DIR)/moc_devicebuttonmanager.cpp \
		$(MOC_DIR)/moc_qwizard.cpp \
		$(MOC_DIR)/moc_locationcombo.cpp \
		$(MOC_DIR)/moc_docproperties.cpp \
		$(MOC_DIR)/datepicker.moc \
		$(MOC_DIR)/timestring1.moc \
		$(MOC_DIR)/pluginloaderlib.moc \
		$(MOC_DIR)/qprocess_unix.moc
OBJMOC	=	$(OBJECTS_DIR)/moc_accessory.o \
		$(OBJECTS_DIR)/moc_datepicker.o \
		$(OBJECTS_DIR)/moc_datetimeedit.o \
		$(OBJECTS_DIR)/moc_fieldmapimpl.o \
		$(OBJECTS_DIR)/moc_qprocess.o \
		$(OBJECTS_DIR)/moc_devicebuttonmanager.o \
		$(OBJECTS_DIR)/moc_qwizard.o \
		$(OBJECTS_DIR)/moc_locationcombo.o \
		$(OBJECTS_DIR)/moc_docproperties.o


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
	tmake qtopia1.pro

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

$(OBJECTS_DIR)/applnk1.o: applnk1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/applnk1.o applnk1.cpp

$(OBJECTS_DIR)/categories1.o: categories1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categories1.o categories1.cpp

$(OBJECTS_DIR)/categoryselect1.o: categoryselect1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categoryselect1.o categoryselect1.cpp

$(OBJECTS_DIR)/config1.o: config1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/config1.o config1.cpp

$(OBJECTS_DIR)/qpeapplication1.o: qpeapplication1.cpp \
		qpe_show_dialog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpeapplication1.o qpeapplication1.cpp

$(OBJECTS_DIR)/storage1.o: storage1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/storage1.o storage1.cpp

$(OBJECTS_DIR)/accessory.o: accessory.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/accessory.o accessory.cpp

$(OBJECTS_DIR)/calendar1.o: calendar1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/calendar1.o calendar1.cpp

$(OBJECTS_DIR)/resource1.o: resource1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/resource1.o resource1.cpp

$(OBJECTS_DIR)/datepicker.o: datepicker.cpp \
		$(MOC_DIR)/datepicker.moc \
		datepicker.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datepicker.o datepicker.cpp

$(OBJECTS_DIR)/datetimeedit.o: datetimeedit.cpp \
		datetimeedit.h \
		datepicker.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datetimeedit.o datetimeedit.cpp

$(OBJECTS_DIR)/timeconversion1.o: timeconversion1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timeconversion1.o timeconversion1.cpp

$(OBJECTS_DIR)/timestring1.o: timestring1.cpp \
		$(MOC_DIR)/timestring1.moc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timestring1.o timestring1.cpp

$(OBJECTS_DIR)/global1.o: global1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/global1.o global1.cpp

$(OBJECTS_DIR)/fieldmapimpl.o: fieldmapimpl.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fieldmapimpl.o fieldmapimpl.cpp

$(OBJECTS_DIR)/qprocess.o: qprocess.cpp \
		qprocess.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qprocess.o qprocess.cpp

$(OBJECTS_DIR)/timezone.o: timezone.cpp \
		timezone.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timezone.o timezone.cpp

$(OBJECTS_DIR)/services.o: services.cpp \
		services.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/services.o services.cpp

$(OBJECTS_DIR)/devicebuttonmanager.o: devicebuttonmanager.cpp \
		devicebutton.h \
		devicebuttonmanager.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/devicebuttonmanager.o devicebuttonmanager.cpp

$(OBJECTS_DIR)/devicebutton.o: devicebutton.cpp \
		devicebutton.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/devicebutton.o devicebutton.cpp

$(OBJECTS_DIR)/fileselector1.o: fileselector1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fileselector1.o fileselector1.cpp

$(OBJECTS_DIR)/qwizard.o: qwizard.cpp \
		qwizard.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qwizard.o qwizard.cpp

$(OBJECTS_DIR)/locationcombo.o: locationcombo.cpp \
		locationcombo.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/locationcombo.o locationcombo.cpp

$(OBJECTS_DIR)/docproperties.o: docproperties.cpp \
		docproperties.h \
		locationcombo.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/docproperties.o docproperties.cpp

$(OBJECTS_DIR)/pluginloader.o: pluginloader.cpp \
		pluginloader.h \
		pluginloaderlib_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginloader.o pluginloader.cpp

$(OBJECTS_DIR)/pluginloaderlib.o: pluginloaderlib.cpp \
		$(MOC_DIR)/pluginloaderlib.moc \
		pluginloaderlib_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginloaderlib.o pluginloaderlib.cpp

$(OBJECTS_DIR)/qprocess_unix.o: qprocess_unix.cpp \
		$(MOC_DIR)/qprocess_unix.moc \
		qprocess.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qprocess_unix.o qprocess_unix.cpp

$(OBJECTS_DIR)/quuid1.o: quuid1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/quuid1.o quuid1.cpp

$(OBJECTS_DIR)/moc_accessory.o: $(MOC_DIR)/moc_accessory.cpp \
		accessory.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_accessory.o $(MOC_DIR)/moc_accessory.cpp

$(OBJECTS_DIR)/moc_datepicker.o: $(MOC_DIR)/moc_datepicker.cpp \
		datepicker.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datepicker.o $(MOC_DIR)/moc_datepicker.cpp

$(OBJECTS_DIR)/moc_datetimeedit.o: $(MOC_DIR)/moc_datetimeedit.cpp \
		datetimeedit.h \
		datepicker.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datetimeedit.o $(MOC_DIR)/moc_datetimeedit.cpp

$(OBJECTS_DIR)/moc_fieldmapimpl.o: $(MOC_DIR)/moc_fieldmapimpl.cpp \
		fieldmapimpl.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_fieldmapimpl.o $(MOC_DIR)/moc_fieldmapimpl.cpp

$(OBJECTS_DIR)/moc_qprocess.o: $(MOC_DIR)/moc_qprocess.cpp \
		qprocess.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qprocess.o $(MOC_DIR)/moc_qprocess.cpp

$(OBJECTS_DIR)/moc_devicebuttonmanager.o: $(MOC_DIR)/moc_devicebuttonmanager.cpp \
		devicebuttonmanager.h \
		devicebutton.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_devicebuttonmanager.o $(MOC_DIR)/moc_devicebuttonmanager.cpp

$(OBJECTS_DIR)/moc_qwizard.o: $(MOC_DIR)/moc_qwizard.cpp \
		qwizard.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qwizard.o $(MOC_DIR)/moc_qwizard.cpp

$(OBJECTS_DIR)/moc_locationcombo.o: $(MOC_DIR)/moc_locationcombo.cpp \
		locationcombo.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_locationcombo.o $(MOC_DIR)/moc_locationcombo.cpp

$(OBJECTS_DIR)/moc_docproperties.o: $(MOC_DIR)/moc_docproperties.cpp \
		docproperties.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_docproperties.o $(MOC_DIR)/moc_docproperties.cpp

$(MOC_DIR)/moc_accessory.cpp: accessory.h
	$(MOC) accessory.h -o $(MOC_DIR)/moc_accessory.cpp

$(MOC_DIR)/moc_datepicker.cpp: datepicker.h
	$(MOC) datepicker.h -o $(MOC_DIR)/moc_datepicker.cpp

$(MOC_DIR)/moc_datetimeedit.cpp: datetimeedit.h
	$(MOC) datetimeedit.h -o $(MOC_DIR)/moc_datetimeedit.cpp

$(MOC_DIR)/moc_fieldmapimpl.cpp: fieldmapimpl.h
	$(MOC) fieldmapimpl.h -o $(MOC_DIR)/moc_fieldmapimpl.cpp

$(MOC_DIR)/moc_qprocess.cpp: qprocess.h
	$(MOC) qprocess.h -o $(MOC_DIR)/moc_qprocess.cpp

$(MOC_DIR)/moc_devicebuttonmanager.cpp: devicebuttonmanager.h
	$(MOC) devicebuttonmanager.h -o $(MOC_DIR)/moc_devicebuttonmanager.cpp

$(MOC_DIR)/moc_qwizard.cpp: qwizard.h
	$(MOC) qwizard.h -o $(MOC_DIR)/moc_qwizard.cpp

$(MOC_DIR)/moc_locationcombo.cpp: locationcombo.h
	$(MOC) locationcombo.h -o $(MOC_DIR)/moc_locationcombo.cpp

$(MOC_DIR)/moc_docproperties.cpp: docproperties.h
	$(MOC) docproperties.h -o $(MOC_DIR)/moc_docproperties.cpp

$(MOC_DIR)/datepicker.moc: datepicker.cpp
	$(MOC) datepicker.cpp -o $(MOC_DIR)/datepicker.moc

$(MOC_DIR)/timestring1.moc: timestring1.cpp
	$(MOC) timestring1.cpp -o $(MOC_DIR)/timestring1.moc

$(MOC_DIR)/pluginloaderlib.moc: pluginloaderlib.cpp
	$(MOC) pluginloaderlib.cpp -o $(MOC_DIR)/pluginloaderlib.moc

$(MOC_DIR)/qprocess_unix.moc: qprocess_unix.cpp
	$(MOC) qprocess_unix.cpp -o $(MOC_DIR)/qprocess_unix.moc



lupdate-check:
	@sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <qtopia1.pro >t$$$$.pro && \
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
	    <qtopia1.pro >t$$$$.pro && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro

lrelease:
	lrelease qtopia1.pro
	$(QPEDIR)/bin/qtopia-linstall qtopia1.pro


ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: qtopia1.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make libraries/qtopia1
