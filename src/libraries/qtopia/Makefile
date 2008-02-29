#############################################################################
# Automatically generated from libraries/qtopia/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = libraries/qtopia/qpe-base.control libraries/qtopia/task-qpe.control

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DQTOPIA_WIN32PROCESS_SUPPORT
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DQTOPIA_WIN32PROCESS_SUPPORT
INCPATH =	-I$(MOC_DIR) -I$(QPEDIR)/include
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -ldl -lcrypt -lm $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/lib/
VER_MAJ = 1
VER_MIN = 5
VER_PATCH = 2
TARGET	= qpe
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	calendar.h \
		global.h \
		qpeglobal.h \
		resource.h \
		xmlreader.h \
		mimetype.h \
		menubutton.h \
		filemanager.h \
		datebookmonth.h \
		fileselector.h \
		fileselector_p.h \
		imageedit.h \
		qpeapplication.h \
		qpestyle.h \
		qpedialog.h \
		config.h \
		applnk.h \
		sound.h \
		tzselect.h \
		qmath.h \
		datebookdb.h \
		alarmserver.h \
		password.h \
		timestring.h \
		fontfactoryinterface.h \
		storage.h \
		qpemessagebox.h \
		timeconversion.h \
		qpedebug.h \
		qpemenubar.h \
		qpetoolbar.h \
		categories.h \
		stringutil.h \
		backend/palmtoprecord.h \
		backend/task.h \
		backend/event.h \
		backend/contact.h \
		backend/qfiledirect_p.h \
		categorymenu.h \
		categoryedit_p.h \
		categoryselect.h \
		categorywidget.h \
		backend/vobject_p.h \
		styleinterface.h \
		windowdecorationinterface.h \
		qdawg.h \
		quuid.h \
		qcom.h \
		qlibrary.h \
		qlibrary_p.h \
		process.h \
		fontmanager.h \
		fontdatabase.h \
		qpedecoration_p.h \
		network.h \
		networkinterface.h \
		qcopenvelope_qws.h \
		power.h \
		ir.h \
		pluginloader_p.h \
		qmemoryfile_p.h
SOURCES =	calendar.cpp \
		global.cpp \
		custom.cpp \
		xmlreader.cpp \
		mimetype.cpp \
		menubutton.cpp \
		filemanager.cpp \
		datebookmonth.cpp \
		fileselector.cpp \
		imageedit.cpp \
		resource.cpp \
		qpeapplication.cpp \
		qpestyle.cpp \
		qpedialog.cpp \
		config.cpp \
		applnk.cpp \
		sound.cpp \
		tzselect.cpp \
		qmath.cpp \
		datebookdb.cpp \
		alarmserver.cpp \
		password.cpp \
		timestring.cpp \
		storage.cpp \
		qpemessagebox.cpp \
		backend/timeconversion.cpp \
		qpedebug.cpp \
		qpemenubar.cpp \
		qpetoolbar.cpp \
		backend/categories.cpp \
		backend/stringutil.cpp \
		backend/palmtoprecord.cpp \
		backend/task.cpp \
		backend/event.cpp \
		backend/contact.cpp \
		categorymenu.cpp \
		categoryedit_p.cpp \
		categoryselect.cpp \
		categorywidget.cpp \
		backend/vcc_yacc.cpp \
		backend/vobject.cpp \
		mediarecorderplugininterface.cpp \
		qdawg.cpp \
		quuid.cpp \
		qlibrary.cpp \
		process.cpp \
		process_unix.cpp \
		qlibrary_unix.cpp \
		fontmanager.cpp \
		fontdatabase.cpp \
		qpedecoration_qws.cpp \
		network.cpp \
		networkinterface.cpp \
		qcopenvelope_qws.cpp \
		power.cpp \
		ir.cpp \
		pluginloader_p.cpp \
		qmemoryfile.cpp \
		qmemoryfile_unix.cpp
OBJECTS =	$(OBJECTS_DIR)/calendar.o \
		$(OBJECTS_DIR)/global.o \
		$(OBJECTS_DIR)/custom.o \
		$(OBJECTS_DIR)/xmlreader.o \
		$(OBJECTS_DIR)/mimetype.o \
		$(OBJECTS_DIR)/menubutton.o \
		$(OBJECTS_DIR)/filemanager.o \
		$(OBJECTS_DIR)/datebookmonth.o \
		$(OBJECTS_DIR)/fileselector.o \
		$(OBJECTS_DIR)/imageedit.o \
		$(OBJECTS_DIR)/resource.o \
		$(OBJECTS_DIR)/qpeapplication.o \
		$(OBJECTS_DIR)/qpestyle.o \
		$(OBJECTS_DIR)/qpedialog.o \
		$(OBJECTS_DIR)/config.o \
		$(OBJECTS_DIR)/applnk.o \
		$(OBJECTS_DIR)/sound.o \
		$(OBJECTS_DIR)/tzselect.o \
		$(OBJECTS_DIR)/qmath.o \
		$(OBJECTS_DIR)/datebookdb.o \
		$(OBJECTS_DIR)/alarmserver.o \
		$(OBJECTS_DIR)/password.o \
		$(OBJECTS_DIR)/timestring.o \
		$(OBJECTS_DIR)/storage.o \
		$(OBJECTS_DIR)/qpemessagebox.o \
		$(OBJECTS_DIR)/timeconversion.o \
		$(OBJECTS_DIR)/qpedebug.o \
		$(OBJECTS_DIR)/qpemenubar.o \
		$(OBJECTS_DIR)/qpetoolbar.o \
		$(OBJECTS_DIR)/categories.o \
		$(OBJECTS_DIR)/stringutil.o \
		$(OBJECTS_DIR)/palmtoprecord.o \
		$(OBJECTS_DIR)/task.o \
		$(OBJECTS_DIR)/event.o \
		$(OBJECTS_DIR)/contact.o \
		$(OBJECTS_DIR)/categorymenu.o \
		$(OBJECTS_DIR)/categoryedit_p.o \
		$(OBJECTS_DIR)/categoryselect.o \
		$(OBJECTS_DIR)/categorywidget.o \
		$(OBJECTS_DIR)/vcc_yacc.o \
		$(OBJECTS_DIR)/vobject.o \
		$(OBJECTS_DIR)/mediarecorderplugininterface.o \
		$(OBJECTS_DIR)/qdawg.o \
		$(OBJECTS_DIR)/quuid.o \
		$(OBJECTS_DIR)/qlibrary.o \
		$(OBJECTS_DIR)/process.o \
		$(OBJECTS_DIR)/process_unix.o \
		$(OBJECTS_DIR)/qlibrary_unix.o \
		$(OBJECTS_DIR)/fontmanager.o \
		$(OBJECTS_DIR)/fontdatabase.o \
		$(OBJECTS_DIR)/qpedecoration_qws.o \
		$(OBJECTS_DIR)/network.o \
		$(OBJECTS_DIR)/networkinterface.o \
		$(OBJECTS_DIR)/qcopenvelope_qws.o \
		$(OBJECTS_DIR)/power.o \
		$(OBJECTS_DIR)/ir.o \
		$(OBJECTS_DIR)/pluginloader_p.o \
		$(OBJECTS_DIR)/qmemoryfile.o \
		$(OBJECTS_DIR)/qmemoryfile_unix.o \
		$(OBJECTS_DIR)/passwordbase_p.o \
		$(OBJECTS_DIR)/categoryeditbase_p.o
INTERFACES =    passwordbase_p.ui \
		categoryeditbase_p.ui
UICDECLS =      passwordbase_p.h \
		categoryeditbase_p.h
UICIMPLS =      passwordbase_p.cpp \
		categoryeditbase_p.cpp
SRCMOC	=	$(MOC_DIR)/moc_menubutton.cpp \
		$(MOC_DIR)/moc_datebookmonth.cpp \
		$(MOC_DIR)/moc_fileselector.cpp \
		$(MOC_DIR)/moc_fileselector_p.cpp \
		$(MOC_DIR)/moc_imageedit.cpp \
		$(MOC_DIR)/moc_qpeapplication.cpp \
		$(MOC_DIR)/moc_qpedialog.cpp \
		$(MOC_DIR)/moc_tzselect.cpp \
		$(MOC_DIR)/moc_storage.cpp \
		$(MOC_DIR)/moc_qpemenubar.cpp \
		$(MOC_DIR)/moc_qpetoolbar.cpp \
		$(MOC_DIR)/moc_categories.cpp \
		$(MOC_DIR)/moc_categorymenu.cpp \
		$(MOC_DIR)/moc_categoryedit_p.cpp \
		$(MOC_DIR)/moc_categoryselect.cpp \
		$(MOC_DIR)/moc_qlibrary_p.cpp \
		$(MOC_DIR)/moc_process.cpp \
		$(MOC_DIR)/moc_qpedecoration_p.cpp \
		$(MOC_DIR)/moc_ir.cpp \
		$(MOC_DIR)/moc_passwordbase_p.cpp \
		$(MOC_DIR)/moc_categoryeditbase_p.cpp \
		$(MOC_DIR)/global.moc \
		$(MOC_DIR)/fileselector.moc \
		$(MOC_DIR)/applnk.moc \
		$(MOC_DIR)/tzselect.moc \
		$(MOC_DIR)/alarmserver.moc \
		$(MOC_DIR)/password.moc \
		$(MOC_DIR)/timestring.moc \
		$(MOC_DIR)/categoryselect.moc \
		$(MOC_DIR)/process_unix.moc \
		$(MOC_DIR)/network.moc
OBJMOC	=	$(OBJECTS_DIR)/moc_menubutton.o \
		$(OBJECTS_DIR)/moc_datebookmonth.o \
		$(OBJECTS_DIR)/moc_fileselector.o \
		$(OBJECTS_DIR)/moc_fileselector_p.o \
		$(OBJECTS_DIR)/moc_imageedit.o \
		$(OBJECTS_DIR)/moc_qpeapplication.o \
		$(OBJECTS_DIR)/moc_qpedialog.o \
		$(OBJECTS_DIR)/moc_tzselect.o \
		$(OBJECTS_DIR)/moc_storage.o \
		$(OBJECTS_DIR)/moc_qpemenubar.o \
		$(OBJECTS_DIR)/moc_qpetoolbar.o \
		$(OBJECTS_DIR)/moc_categories.o \
		$(OBJECTS_DIR)/moc_categorymenu.o \
		$(OBJECTS_DIR)/moc_categoryedit_p.o \
		$(OBJECTS_DIR)/moc_categoryselect.o \
		$(OBJECTS_DIR)/moc_qlibrary_p.o \
		$(OBJECTS_DIR)/moc_process.o \
		$(OBJECTS_DIR)/moc_qpedecoration_p.o \
		$(OBJECTS_DIR)/moc_ir.o \
		$(OBJECTS_DIR)/moc_passwordbase_p.o \
		$(OBJECTS_DIR)/moc_categoryeditbase_p.o


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
	tmake qtopia.pro

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

$(OBJECTS_DIR)/calendar.o: calendar.cpp \
		calendar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/calendar.o calendar.cpp

$(OBJECTS_DIR)/global.o: global.cpp \
		$(MOC_DIR)/global.moc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/global.o global.cpp

$(OBJECTS_DIR)/custom.o: custom.cpp \
		power.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/custom.o custom.cpp

$(OBJECTS_DIR)/xmlreader.o: xmlreader.cpp \
		xmlreader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/xmlreader.o xmlreader.cpp

$(OBJECTS_DIR)/mimetype.o: mimetype.cpp \
		mimetype.h \
		applnk.h \
		resource.h \
		qpeapplication.h \
		config.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mimetype.o mimetype.cpp

$(OBJECTS_DIR)/menubutton.o: menubutton.cpp \
		menubutton.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/menubutton.o menubutton.cpp

$(OBJECTS_DIR)/filemanager.o: filemanager.cpp \
		filemanager.h \
		applnk.h \
		global.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/filemanager.o filemanager.cpp

$(OBJECTS_DIR)/datebookmonth.o: datebookmonth.cpp \
		config.h \
		datebookmonth.h \
		calendar.h \
		timestring.h \
		datebookdb.h \
		resource.h \
		qpeapplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookmonth.o datebookmonth.cpp

$(OBJECTS_DIR)/fileselector.o: fileselector.cpp \
		$(MOC_DIR)/fileselector.moc \
		global.h \
		fileselector.h \
		filemanager.h \
		applnk.h \
		fileselector_p.h \
		resource.h \
		config.h \
		storage.h \
		qpemenubar.h \
		qpeapplication.h \
		categorymenu.h \
		categoryselect.h \
		mimetype.h \
		categories.h \
		qpemessagebox.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fileselector.o fileselector.cpp

$(OBJECTS_DIR)/imageedit.o: imageedit.cpp \
		imageedit.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/imageedit.o imageedit.cpp

$(OBJECTS_DIR)/resource.o: resource.cpp \
		resource.h \
		config.h \
		mimetype.h \
		inlinepics_p.h \
		qpeapplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/resource.o resource.cpp

$(OBJECTS_DIR)/qpeapplication.o: qpeapplication.cpp \
		custom.h \
		qcopenvelope_qws.h \
		qpeapplication.h \
		global.h \
		config.h \
		network.h \
		applnk.h \
		fontmanager.h \
		fontdatabase.h \
		power.h \
		qpemenubar.h \
		imagecodecinterface.h \
		textcodecinterface.h \
		styleinterface.h \
		resource.h \
		qpestyle.h \
		qpedecoration_p.h \
		windowdecorationinterface.h \
		alarmserver.h \
		pluginloader_p.h \
		../qtopia1/pluginloader.h \
		../qtopia1/qpe_show_dialog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpeapplication.o qpeapplication.cpp

$(OBJECTS_DIR)/qpestyle.o: qpestyle.cpp \
		qpestyle.h \
		qpeapplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpestyle.o qpestyle.cpp

$(OBJECTS_DIR)/qpedialog.o: qpedialog.cpp \
		qpedialog.h \
		qpeapplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpedialog.o qpedialog.cpp

$(OBJECTS_DIR)/config.o: config.cpp \
		config.h \
		global.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/config.o config.cpp

$(OBJECTS_DIR)/applnk.o: applnk.cpp \
		$(MOC_DIR)/applnk.moc \
		applnk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/applnk.o applnk.cpp

$(OBJECTS_DIR)/sound.o: sound.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/sound.o sound.cpp

$(OBJECTS_DIR)/tzselect.o: tzselect.cpp \
		$(MOC_DIR)/tzselect.moc \
		tzselect.h \
		resource.h \
		global.h \
		config.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/tzselect.o tzselect.cpp

$(OBJECTS_DIR)/qmath.o: qmath.cpp \
		qmath.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qmath.o qmath.cpp

$(OBJECTS_DIR)/datebookdb.o: datebookdb.cpp \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookdb.o datebookdb.cpp

$(OBJECTS_DIR)/alarmserver.o: alarmserver.cpp \
		$(MOC_DIR)/alarmserver.moc \
		qlibrary.h \
		qpeapplication.h \
		global.h \
		resource.h \
		qcopenvelope_qws.h \
		alarmserver.h \
		process.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/alarmserver.o alarmserver.cpp

$(OBJECTS_DIR)/password.o: password.cpp \
		$(MOC_DIR)/password.moc \
		password.h \
		config.h \
		global.h \
		backend/contact.h \
		qpeapplication.h \
		passwordbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/password.o password.cpp

$(OBJECTS_DIR)/timestring.o: timestring.cpp \
		$(MOC_DIR)/timestring.moc \
		timestring.h \
		config.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timestring.o timestring.cpp

$(OBJECTS_DIR)/storage.o: storage.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/storage.o storage.cpp

$(OBJECTS_DIR)/qpemessagebox.o: qpemessagebox.cpp \
		qpemessagebox.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpemessagebox.o qpemessagebox.cpp

$(OBJECTS_DIR)/timeconversion.o: backend/timeconversion.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/timeconversion.o backend/timeconversion.cpp

$(OBJECTS_DIR)/qpedebug.o: qpedebug.cpp \
		qpedebug.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpedebug.o qpedebug.cpp

$(OBJECTS_DIR)/qpemenubar.o: qpemenubar.cpp \
		qpemenubar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpemenubar.o qpemenubar.cpp

$(OBJECTS_DIR)/qpetoolbar.o: qpetoolbar.cpp \
		qpetoolbar.h \
		qpemenubar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpetoolbar.o qpetoolbar.cpp

$(OBJECTS_DIR)/categories.o: backend/categories.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categories.o backend/categories.cpp

$(OBJECTS_DIR)/stringutil.o: backend/stringutil.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/stringutil.o backend/stringutil.cpp

$(OBJECTS_DIR)/palmtoprecord.o: backend/palmtoprecord.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/palmtoprecord.o backend/palmtoprecord.cpp

$(OBJECTS_DIR)/task.o: backend/task.cpp \
		backend/task.h \
		backend/recordfields.h \
		backend/vobject_p.h \
		backend/qfiledirect_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/task.o backend/task.cpp

$(OBJECTS_DIR)/event.o: backend/event.cpp \
		backend/event.h \
		backend/qfiledirect_p.h \
		backend/vobject_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/event.o backend/event.cpp

$(OBJECTS_DIR)/contact.o: backend/contact.cpp \
		backend/contact.h \
		backend/vobject_p.h \
		backend/qfiledirect_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/contact.o backend/contact.cpp

$(OBJECTS_DIR)/categorymenu.o: categorymenu.cpp \
		categorymenu.h \
		categoryselect.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categorymenu.o categorymenu.cpp

$(OBJECTS_DIR)/categoryedit_p.o: categoryedit_p.cpp \
		categoryedit_p.h \
		categoryeditbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categoryedit_p.o categoryedit_p.cpp

$(OBJECTS_DIR)/categoryselect.o: categoryselect.cpp \
		$(MOC_DIR)/categoryselect.moc \
		categoryedit_p.h \
		categoryeditbase_p.h \
		categoryselect.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categoryselect.o categoryselect.cpp

$(OBJECTS_DIR)/categorywidget.o: categorywidget.cpp \
		categoryedit_p.h \
		categoryeditbase_p.h \
		categorywidget.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categorywidget.o categorywidget.cpp

$(OBJECTS_DIR)/vcc_yacc.o: backend/vcc_yacc.cpp \
		backend/vobject_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/vcc_yacc.o backend/vcc_yacc.cpp

$(OBJECTS_DIR)/vobject.o: backend/vobject.cpp \
		backend/vobject_p.h \
		backend/qfiledirect_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/vobject.o backend/vobject.cpp

$(OBJECTS_DIR)/mediarecorderplugininterface.o: mediarecorderplugininterface.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/mediarecorderplugininterface.o mediarecorderplugininterface.cpp

$(OBJECTS_DIR)/qdawg.o: qdawg.cpp \
		qdawg.h \
		qmemoryfile_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qdawg.o qdawg.cpp

$(OBJECTS_DIR)/quuid.o: quuid.cpp \
		quuid.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/quuid.o quuid.cpp

$(OBJECTS_DIR)/qlibrary.o: qlibrary.cpp \
		qlibrary_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qlibrary.o qlibrary.cpp

$(OBJECTS_DIR)/process.o: process.cpp \
		process.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/process.o process.cpp

$(OBJECTS_DIR)/process_unix.o: process_unix.cpp \
		$(MOC_DIR)/process_unix.moc \
		process.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/process_unix.o process_unix.cpp

$(OBJECTS_DIR)/qlibrary_unix.o: qlibrary_unix.cpp \
		qlibrary_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qlibrary_unix.o qlibrary_unix.cpp

$(OBJECTS_DIR)/fontmanager.o: fontmanager.cpp \
		fontmanager.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fontmanager.o fontmanager.cpp

$(OBJECTS_DIR)/fontdatabase.o: fontdatabase.cpp \
		pluginloader_p.h \
		../qtopia1/pluginloader.h \
		qpeapplication.h \
		fontfactoryinterface.h \
		fontdatabase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/fontdatabase.o fontdatabase.cpp

$(OBJECTS_DIR)/qpedecoration_qws.o: qpedecoration_qws.cpp \
		qcopenvelope_qws.h \
		qpedecoration_p.h \
		windowdecorationinterface.h \
		qpeapplication.h \
		resource.h \
		global.h \
		qlibrary.h \
		pluginloader_p.h \
		../qtopia1/pluginloader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qpedecoration_qws.o qpedecoration_qws.cpp

$(OBJECTS_DIR)/network.o: network.cpp \
		$(MOC_DIR)/network.moc \
		pluginloader_p.h \
		../qtopia1/pluginloader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/network.o network.cpp

$(OBJECTS_DIR)/networkinterface.o: networkinterface.cpp \
		networkinterface.h \
		network.h \
		config.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/networkinterface.o networkinterface.cpp

$(OBJECTS_DIR)/qcopenvelope_qws.o: qcopenvelope_qws.cpp \
		qcopenvelope_qws.h \
		global.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qcopenvelope_qws.o qcopenvelope_qws.cpp

$(OBJECTS_DIR)/power.o: power.cpp \
		power.h \
		custom.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/power.o power.cpp

$(OBJECTS_DIR)/ir.o: ir.cpp \
		ir.h \
		qcopenvelope_qws.h \
		applnk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ir.o ir.cpp

$(OBJECTS_DIR)/pluginloader_p.o: pluginloader_p.cpp \
		pluginloader_p.h \
		../qtopia1/pluginloader.h \
		../qtopia1/pluginloader.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/pluginloader_p.o pluginloader_p.cpp

$(OBJECTS_DIR)/qmemoryfile.o: qmemoryfile.cpp \
		qmemoryfile_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qmemoryfile.o qmemoryfile.cpp

$(OBJECTS_DIR)/qmemoryfile_unix.o: qmemoryfile_unix.cpp \
		qmemoryfile_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/qmemoryfile_unix.o qmemoryfile_unix.cpp

passwordbase_p.h: passwordbase_p.ui
	$(UIC) passwordbase_p.ui -o $(INTERFACE_DECL_PATH)/passwordbase_p.h

passwordbase_p.cpp: passwordbase_p.ui
	$(UIC) passwordbase_p.ui -i passwordbase_p.h -o passwordbase_p.cpp

categoryeditbase_p.h: categoryeditbase_p.ui
	$(UIC) categoryeditbase_p.ui -o $(INTERFACE_DECL_PATH)/categoryeditbase_p.h

categoryeditbase_p.cpp: categoryeditbase_p.ui
	$(UIC) categoryeditbase_p.ui -i categoryeditbase_p.h -o categoryeditbase_p.cpp

$(OBJECTS_DIR)/passwordbase_p.o: passwordbase_p.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/passwordbase_p.o passwordbase_p.cpp

$(OBJECTS_DIR)/categoryeditbase_p.o: categoryeditbase_p.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/categoryeditbase_p.o categoryeditbase_p.cpp

$(OBJECTS_DIR)/moc_menubutton.o: $(MOC_DIR)/moc_menubutton.cpp \
		menubutton.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_menubutton.o $(MOC_DIR)/moc_menubutton.cpp

$(OBJECTS_DIR)/moc_datebookmonth.o: $(MOC_DIR)/moc_datebookmonth.cpp \
		datebookmonth.h \
		calendar.h \
		timestring.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebookmonth.o $(MOC_DIR)/moc_datebookmonth.cpp

$(OBJECTS_DIR)/moc_fileselector.o: $(MOC_DIR)/moc_fileselector.cpp \
		fileselector.h \
		filemanager.h \
		applnk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_fileselector.o $(MOC_DIR)/moc_fileselector.cpp

$(OBJECTS_DIR)/moc_fileselector_p.o: $(MOC_DIR)/moc_fileselector_p.cpp \
		fileselector_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_fileselector_p.o $(MOC_DIR)/moc_fileselector_p.cpp

$(OBJECTS_DIR)/moc_imageedit.o: $(MOC_DIR)/moc_imageedit.cpp \
		imageedit.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_imageedit.o $(MOC_DIR)/moc_imageedit.cpp

$(OBJECTS_DIR)/moc_qpeapplication.o: $(MOC_DIR)/moc_qpeapplication.cpp \
		qpeapplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qpeapplication.o $(MOC_DIR)/moc_qpeapplication.cpp

$(OBJECTS_DIR)/moc_qpedialog.o: $(MOC_DIR)/moc_qpedialog.cpp \
		qpedialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qpedialog.o $(MOC_DIR)/moc_qpedialog.cpp

$(OBJECTS_DIR)/moc_tzselect.o: $(MOC_DIR)/moc_tzselect.cpp \
		tzselect.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_tzselect.o $(MOC_DIR)/moc_tzselect.cpp

$(OBJECTS_DIR)/moc_storage.o: $(MOC_DIR)/moc_storage.cpp \
		storage.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_storage.o $(MOC_DIR)/moc_storage.cpp

$(OBJECTS_DIR)/moc_qpemenubar.o: $(MOC_DIR)/moc_qpemenubar.cpp \
		qpemenubar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qpemenubar.o $(MOC_DIR)/moc_qpemenubar.cpp

$(OBJECTS_DIR)/moc_qpetoolbar.o: $(MOC_DIR)/moc_qpetoolbar.cpp \
		qpetoolbar.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qpetoolbar.o $(MOC_DIR)/moc_qpetoolbar.cpp

$(OBJECTS_DIR)/moc_categories.o: $(MOC_DIR)/moc_categories.cpp \
		categories.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_categories.o $(MOC_DIR)/moc_categories.cpp

$(OBJECTS_DIR)/moc_categorymenu.o: $(MOC_DIR)/moc_categorymenu.cpp \
		categorymenu.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_categorymenu.o $(MOC_DIR)/moc_categorymenu.cpp

$(OBJECTS_DIR)/moc_categoryedit_p.o: $(MOC_DIR)/moc_categoryedit_p.cpp \
		categoryedit_p.h \
		categoryeditbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_categoryedit_p.o $(MOC_DIR)/moc_categoryedit_p.cpp

$(OBJECTS_DIR)/moc_categoryselect.o: $(MOC_DIR)/moc_categoryselect.cpp \
		categoryselect.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_categoryselect.o $(MOC_DIR)/moc_categoryselect.cpp

$(OBJECTS_DIR)/moc_qlibrary_p.o: $(MOC_DIR)/moc_qlibrary_p.cpp \
		qlibrary_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qlibrary_p.o $(MOC_DIR)/moc_qlibrary_p.cpp

$(OBJECTS_DIR)/moc_process.o: $(MOC_DIR)/moc_process.cpp \
		process.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_process.o $(MOC_DIR)/moc_process.cpp

$(OBJECTS_DIR)/moc_qpedecoration_p.o: $(MOC_DIR)/moc_qpedecoration_p.cpp \
		qpedecoration_p.h \
		windowdecorationinterface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_qpedecoration_p.o $(MOC_DIR)/moc_qpedecoration_p.cpp

$(OBJECTS_DIR)/moc_ir.o: $(MOC_DIR)/moc_ir.cpp \
		ir.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_ir.o $(MOC_DIR)/moc_ir.cpp

$(OBJECTS_DIR)/moc_passwordbase_p.o: $(MOC_DIR)/moc_passwordbase_p.cpp \
		passwordbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_passwordbase_p.o $(MOC_DIR)/moc_passwordbase_p.cpp

$(OBJECTS_DIR)/moc_categoryeditbase_p.o: $(MOC_DIR)/moc_categoryeditbase_p.cpp \
		categoryeditbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_categoryeditbase_p.o $(MOC_DIR)/moc_categoryeditbase_p.cpp

$(MOC_DIR)/moc_menubutton.cpp: menubutton.h
	$(MOC) menubutton.h -o $(MOC_DIR)/moc_menubutton.cpp

$(MOC_DIR)/moc_datebookmonth.cpp: datebookmonth.h
	$(MOC) datebookmonth.h -o $(MOC_DIR)/moc_datebookmonth.cpp

$(MOC_DIR)/moc_fileselector.cpp: fileselector.h
	$(MOC) fileselector.h -o $(MOC_DIR)/moc_fileselector.cpp

$(MOC_DIR)/moc_fileselector_p.cpp: fileselector_p.h
	$(MOC) fileselector_p.h -o $(MOC_DIR)/moc_fileselector_p.cpp

$(MOC_DIR)/moc_imageedit.cpp: imageedit.h
	$(MOC) imageedit.h -o $(MOC_DIR)/moc_imageedit.cpp

$(MOC_DIR)/moc_qpeapplication.cpp: qpeapplication.h
	$(MOC) qpeapplication.h -o $(MOC_DIR)/moc_qpeapplication.cpp

$(MOC_DIR)/moc_qpedialog.cpp: qpedialog.h
	$(MOC) qpedialog.h -o $(MOC_DIR)/moc_qpedialog.cpp

$(MOC_DIR)/moc_tzselect.cpp: tzselect.h
	$(MOC) tzselect.h -o $(MOC_DIR)/moc_tzselect.cpp

$(MOC_DIR)/moc_storage.cpp: storage.h
	$(MOC) storage.h -o $(MOC_DIR)/moc_storage.cpp

$(MOC_DIR)/moc_qpemenubar.cpp: qpemenubar.h
	$(MOC) qpemenubar.h -o $(MOC_DIR)/moc_qpemenubar.cpp

$(MOC_DIR)/moc_qpetoolbar.cpp: qpetoolbar.h
	$(MOC) qpetoolbar.h -o $(MOC_DIR)/moc_qpetoolbar.cpp

$(MOC_DIR)/moc_categories.cpp: categories.h
	$(MOC) categories.h -o $(MOC_DIR)/moc_categories.cpp

$(MOC_DIR)/moc_categorymenu.cpp: categorymenu.h
	$(MOC) categorymenu.h -o $(MOC_DIR)/moc_categorymenu.cpp

$(MOC_DIR)/moc_categoryedit_p.cpp: categoryedit_p.h
	$(MOC) categoryedit_p.h -o $(MOC_DIR)/moc_categoryedit_p.cpp

$(MOC_DIR)/moc_categoryselect.cpp: categoryselect.h
	$(MOC) categoryselect.h -o $(MOC_DIR)/moc_categoryselect.cpp

$(MOC_DIR)/moc_qlibrary_p.cpp: qlibrary_p.h
	$(MOC) qlibrary_p.h -o $(MOC_DIR)/moc_qlibrary_p.cpp

$(MOC_DIR)/moc_process.cpp: process.h
	$(MOC) process.h -o $(MOC_DIR)/moc_process.cpp

$(MOC_DIR)/moc_qpedecoration_p.cpp: qpedecoration_p.h
	$(MOC) qpedecoration_p.h -o $(MOC_DIR)/moc_qpedecoration_p.cpp

$(MOC_DIR)/moc_ir.cpp: ir.h
	$(MOC) ir.h -o $(MOC_DIR)/moc_ir.cpp

$(MOC_DIR)/global.moc: global.cpp
	$(MOC) global.cpp -o $(MOC_DIR)/global.moc

$(MOC_DIR)/fileselector.moc: fileselector.cpp
	$(MOC) fileselector.cpp -o $(MOC_DIR)/fileselector.moc

$(MOC_DIR)/applnk.moc: applnk.cpp
	$(MOC) applnk.cpp -o $(MOC_DIR)/applnk.moc

$(MOC_DIR)/tzselect.moc: tzselect.cpp
	$(MOC) tzselect.cpp -o $(MOC_DIR)/tzselect.moc

$(MOC_DIR)/alarmserver.moc: alarmserver.cpp
	$(MOC) alarmserver.cpp -o $(MOC_DIR)/alarmserver.moc

$(MOC_DIR)/password.moc: password.cpp
	$(MOC) password.cpp -o $(MOC_DIR)/password.moc

$(MOC_DIR)/timestring.moc: timestring.cpp
	$(MOC) timestring.cpp -o $(MOC_DIR)/timestring.moc

$(MOC_DIR)/categoryselect.moc: categoryselect.cpp
	$(MOC) categoryselect.cpp -o $(MOC_DIR)/categoryselect.moc

$(MOC_DIR)/process_unix.moc: process_unix.cpp
	$(MOC) process_unix.cpp -o $(MOC_DIR)/process_unix.moc

$(MOC_DIR)/network.moc: network.cpp
	$(MOC) network.cpp -o $(MOC_DIR)/network.moc

$(MOC_DIR)/moc_passwordbase_p.cpp: passwordbase_p.h
	$(MOC) passwordbase_p.h -o $(MOC_DIR)/moc_passwordbase_p.cpp

$(MOC_DIR)/moc_categoryeditbase_p.cpp: categoryeditbase_p.h
	$(MOC) categoryeditbase_p.h -o $(MOC_DIR)/moc_categoryeditbase_p.cpp



lupdate-check:
	@sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <qtopia.pro >t$$$$.pro && \
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
	    <qtopia.pro >t$$$$.pro && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro

lrelease:
	lrelease qtopia.pro
	$(QPEDIR)/bin/qtopia-linstall qtopia.pro


ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: qtopia.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make libraries/qtopia
backend/vcc_yacc.cpp: backend/vcc.y
	$(YACC) $(YACCFLAGS) -p vcc backend/vcc.y
	-rm backend/vcc_yacc.cpp
	mv y.tab.c backend/vcc_yacc.cpp
