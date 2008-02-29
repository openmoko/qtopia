#############################################################################
# Automatically generated from server/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = server/qpe-taskbar.control

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
INCPATH =	-I$(QPEDIR)/include -I$(QPEDIR)/src/settings/calibrate -I$(QPEDIR)/src/3rdparty/libraries/rsync
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqtopiaservices -lqpe -lcrypt -luuid $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= qpe
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	desktop.h \
		qprocess.h \
		appicons.h \
		taskbar.h \
		runningappbar.h \
		stabmon.h \
		inputmethods.h \
		systray.h \
		wait.h \
		shutdownimpl.h \
		launcher.h \
		launcherview.h \
		$(QPEDIR)/src/settings/calibrate/calibrate.h \
		startmenu.h \
		transferserver.h \
		qcopbridge.h \
		packageslave.h \
		irserver.h \
		devicebutton.h \
		devicebuttonmanager.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/buf.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/emit.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/search.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/types.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/whole.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.h
SOURCES =	desktop.cpp \
		qprocess.cpp \
		qprocess_unix.cpp \
		appicons.cpp \
		taskbar.cpp \
		runningappbar.cpp \
		stabmon.cpp \
		inputmethods.cpp \
		systray.cpp \
		wait.cpp \
		shutdownimpl.cpp \
		launcher.cpp \
		launcherview.cpp \
		$(QPEDIR)/src/settings/calibrate/calibrate.cpp \
		transferserver.cpp \
		packageslave.cpp \
		irserver.cpp \
		qcopbridge.cpp \
		startmenu.cpp \
		main.cpp \
		devicebutton.cpp \
		devicebuttonmanager.cpp \
		$(QPEDIR)/src/3rdparty/libraries/rsync/base64.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/buf.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/delta.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/emit.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/hex.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/mdfour.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/mksum.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/msg.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/patch.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/readsums.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/scoop.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/search.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stats.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/tube.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/version.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/whole.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.cpp
OBJECTS =	desktop.o \
		qprocess.o \
		qprocess_unix.o \
		appicons.o \
		taskbar.o \
		runningappbar.o \
		stabmon.o \
		inputmethods.o \
		systray.o \
		wait.o \
		shutdownimpl.o \
		launcher.o \
		launcherview.o \
		$(QPEDIR)/src/settings/calibrate/calibrate.o \
		transferserver.o \
		packageslave.o \
		irserver.o \
		qcopbridge.o \
		startmenu.o \
		main.o \
		devicebutton.o \
		devicebuttonmanager.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/base64.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/buf.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/delta.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/emit.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/hex.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/mdfour.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/mksum.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/msg.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/patch.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/readsums.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/scoop.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/search.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stats.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/tube.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/version.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/whole.o \
		$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.o \
		shutdown.o \
		syncdialog.o
INTERFACES =    shutdown.ui \
		syncdialog.ui
UICDECLS =      shutdown.h \
		syncdialog.h
UICIMPLS =      shutdown.cpp \
		syncdialog.cpp
SRCMOC	=	moc_desktop.cpp \
		moc_qprocess.cpp \
		moc_appicons.cpp \
		moc_taskbar.cpp \
		moc_runningappbar.cpp \
		moc_inputmethods.cpp \
		moc_systray.cpp \
		moc_shutdownimpl.cpp \
		moc_launcher.cpp \
		moc_launcherview.cpp \
		$(QPEDIR)/src/settings/calibrate/moc_calibrate.cpp \
		moc_startmenu.cpp \
		moc_transferserver.cpp \
		moc_qcopbridge.cpp \
		moc_packageslave.cpp \
		moc_irserver.cpp \
		moc_devicebuttonmanager.cpp \
		qprocess_unix.moc \
		appicons.moc \
		moc_shutdown.cpp \
		moc_syncdialog.cpp
OBJMOC	=	moc_desktop.o \
		moc_qprocess.o \
		moc_appicons.o \
		moc_taskbar.o \
		moc_runningappbar.o \
		moc_inputmethods.o \
		moc_systray.o \
		moc_shutdownimpl.o \
		moc_launcher.o \
		moc_launcherview.o \
		$(QPEDIR)/src/settings/calibrate/moc_calibrate.o \
		moc_startmenu.o \
		moc_transferserver.o \
		moc_qcopbridge.o \
		moc_packageslave.o \
		moc_irserver.o \
		moc_devicebuttonmanager.o \
		moc_shutdown.o \
		moc_syncdialog.o


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
	tmake server.pro

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

desktop.o: desktop.cpp \
		desktop.h \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		launcher.h \
		$(QPEDIR)/include/qpe/config.h \
		launcherview.h \
		$(QPEDIR)/include/qpe/storage.h \
		qcopbridge.h \
		startmenu.h \
		taskbar.h \
		$(QPEDIR)/include/qpe/custom.h \
		transferserver.h \
		irserver.h \
		packageslave.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/mimetype.h \
		$(QPEDIR)/include/qpe/password.h \
		$(QPEDIR)/include/qpe/power.h \
		$(QPEDIR)/include/qpe/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		devicebuttonmanager.h \
		devicebutton.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/global.h

qprocess.o: qprocess.cpp \
		qprocess.h

qprocess_unix.o: qprocess_unix.cpp \
		qprocess_unix.moc \
		qprocess.h \
		qcleanuphandler_p.h

appicons.o: appicons.cpp \
		appicons.moc \
		appicons.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

taskbar.o: taskbar.cpp \
		startmenu.h \
		inputmethods.h \
		$(QPEDIR)/include/qpe/inputmethodinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		mrulist.h \
		$(QPEDIR)/include/qpe/applnk.h \
		runningappbar.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		systray.h \
		$(QPEDIR)/include/qpe/taskbarappletinterface.h \
		$(QPEDIR)/src/settings/calibrate/calibrate.h \
		wait.h \
		$(QPEDIR)/include/qpe/resource.h \
		appicons.h \
		taskbar.h \
		$(QPEDIR)/include/qpe/custom.h \
		desktop.h \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/global.h \
		apps.h \
		../settings/systemtime/settime.h \
		$(QPEDIR)/include/qpe/timestring.h

runningappbar.o: runningappbar.cpp \
		qprocess.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/global.h \
		runningappbar.h \
		$(QPEDIR)/include/qtopia/applnk.h

stabmon.o: stabmon.cpp \
		stabmon.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

inputmethods.o: inputmethods.cpp \
		inputmethods.h \
		$(QPEDIR)/include/qpe/inputmethodinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qpe/global.h

systray.o: systray.cpp \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/config.h \
		quicklauncher.h \
		systray.h \
		$(QPEDIR)/include/qpe/taskbarappletinterface.h \
		$(QPEDIR)/include/qpe/qcom.h

wait.o: wait.cpp \
		wait.h \
		$(QPEDIR)/include/qpe/resource.h

shutdownimpl.o: shutdownimpl.cpp \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

launcher.o: launcher.cpp \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qpe/mimetype.h \
		$(QPEDIR)/include/qpe/storage.h \
		$(QPEDIR)/include/qpe/categories.h \
		$(QPEDIR)/include/qtopia/private/palmtopuidgen.h \
		$(QPEDIR)/include/qpe/version.h \
		launcherview.h \
		launcher.h \
		syncdialog.h \
		desktop.h \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/lnkproperties.h \
		mrulist.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.h

launcherview.o: launcherview.cpp \
		launcherview.h \
		$(QPEDIR)/include/qpe/storage.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/qpedebug.h \
		$(QPEDIR)/include/qpe/categories.h \
		$(QPEDIR)/include/qtopia/private/palmtopuidgen.h \
		$(QPEDIR)/include/qpe/categoryselect.h \
		$(QPEDIR)/include/qpe/menubutton.h \
		$(QPEDIR)/include/qpe/mimetype.h \
		$(QPEDIR)/include/qpe/resource.h \
		$(QPEDIR)/include/qpe/qpetoolbar.h

$(QPEDIR)/src/settings/calibrate/calibrate.o: $(QPEDIR)/src/settings/calibrate/calibrate.cpp \
		$(QPEDIR)/src/settings/calibrate/calibrate.h \
		$(QPEDIR)/include/qpe/resource.h

transferserver.o: transferserver.cpp \
		$(QPEDIR)/include/qpe/process.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qtopia/private/contact.h \
		$(QPEDIR)/include/qtopia/private/palmtoprecord.h \
		$(QPEDIR)/include/qtopia/private/recordfields.h \
		$(QPEDIR)/include/qpe/quuid.h \
		$(QPEDIR)/include/qpe/version.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		transferserver.h \
		qprocess.h

packageslave.o: packageslave.cpp \
		packageslave.h \
		qprocess.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

irserver.o: irserver.cpp \
		irserver.h \
		$(QPEDIR)/include/qpe/qlibrary.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/qcom.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		obexinterface.h \
		$(QPEDIR)/include/qpe/qcom.h

qcopbridge.o: qcopbridge.cpp \
		qcopbridge.h \
		transferserver.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/version.h

startmenu.o: startmenu.cpp \
		startmenu.h \
		sidething.h \
		mrulist.h \
		$(QPEDIR)/include/qpe/applnk.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qpe/config.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/resource.h

main.o: main.cpp \
		desktop.h \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		taskbar.h \
		$(QPEDIR)/include/qpe/custom.h \
		stabmon.h \
		launcher.h \
		$(QPEDIR)/include/qpe/config.h \
		launcherview.h \
		$(QPEDIR)/include/qpe/storage.h \
		$(QPEDIR)/include/qpe/network.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h \
		$(QPEDIR)/include/qpe/alarmserver.h \
		$(QPEDIR)/src/settings/calibrate/../calibrate/calibrate.h \
		$(QPEDIR)/include/qpe/global.h

devicebutton.o: devicebutton.cpp \
		$(QPEDIR)/include/qtopia/resource.h \
		devicebutton.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h

devicebuttonmanager.o: devicebuttonmanager.cpp \
		devicebutton.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h \
		devicebuttonmanager.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/qcopenvelope_qws.h \
		$(QPEDIR)/include/qtopia/resource.h

$(QPEDIR)/src/3rdparty/libraries/rsync/base64.o: $(QPEDIR)/src/3rdparty/libraries/rsync/base64.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h

$(QPEDIR)/src/3rdparty/libraries/rsync/buf.o: $(QPEDIR)/src/3rdparty/libraries/rsync/buf.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/buf.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h

$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.o: $(QPEDIR)/src/3rdparty/libraries/rsync/checksum.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.h

$(QPEDIR)/src/3rdparty/libraries/rsync/command.o: $(QPEDIR)/src/3rdparty/libraries/rsync/command.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.h

$(QPEDIR)/src/3rdparty/libraries/rsync/delta.o: $(QPEDIR)/src/3rdparty/libraries/rsync/delta.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/emit.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/search.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/types.h

$(QPEDIR)/src/3rdparty/libraries/rsync/emit.o: $(QPEDIR)/src/3rdparty/libraries/rsync/emit.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/emit.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h

$(QPEDIR)/src/3rdparty/libraries/rsync/hex.o: $(QPEDIR)/src/3rdparty/libraries/rsync/hex.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h

$(QPEDIR)/src/3rdparty/libraries/rsync/job.o: $(QPEDIR)/src/3rdparty/libraries/rsync/job.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/mdfour.o: $(QPEDIR)/src/3rdparty/libraries/rsync/mdfour.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/types.h

$(QPEDIR)/src/3rdparty/libraries/rsync/mksum.o: $(QPEDIR)/src/3rdparty/libraries/rsync/mksum.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.h

$(QPEDIR)/src/3rdparty/libraries/rsync/msg.o: $(QPEDIR)/src/3rdparty/libraries/rsync/msg.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h

$(QPEDIR)/src/3rdparty/libraries/rsync/netint.o: $(QPEDIR)/src/3rdparty/libraries/rsync/netint.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h

$(QPEDIR)/src/3rdparty/libraries/rsync/patch.o: $(QPEDIR)/src/3rdparty/libraries/rsync/patch.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h

$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.o: $(QPEDIR)/src/3rdparty/libraries/rsync/prototab.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/command.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/prototab.h

$(QPEDIR)/src/3rdparty/libraries/rsync/readsums.o: $(QPEDIR)/src/3rdparty/libraries/rsync/readsums.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/netint.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/protocol.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h

$(QPEDIR)/src/3rdparty/libraries/rsync/scoop.o: $(QPEDIR)/src/3rdparty/libraries/rsync/scoop.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h

$(QPEDIR)/src/3rdparty/libraries/rsync/search.o: $(QPEDIR)/src/3rdparty/libraries/rsync/search.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/search.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/checksum.h

$(QPEDIR)/src/3rdparty/libraries/rsync/stats.o: $(QPEDIR)/src/3rdparty/libraries/rsync/stats.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/stream.o: $(QPEDIR)/src/3rdparty/libraries/rsync/stream.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.o: $(QPEDIR)/src/3rdparty/libraries/rsync/sumset.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/trace.o: $(QPEDIR)/src/3rdparty/libraries/rsync/trace.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/tube.o: $(QPEDIR)/src/3rdparty/libraries/rsync/tube.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/stream.h

$(QPEDIR)/src/3rdparty/libraries/rsync/util.o: $(QPEDIR)/src/3rdparty/libraries/rsync/util.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h

$(QPEDIR)/src/3rdparty/libraries/rsync/version.o: $(QPEDIR)/src/3rdparty/libraries/rsync/version.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h

$(QPEDIR)/src/3rdparty/libraries/rsync/whole.o: $(QPEDIR)/src/3rdparty/libraries/rsync/whole.c \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/config_linux.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/trace.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/fileutil.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/sumset.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/job.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/buf.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/whole.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/util.h

$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.o: $(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.cpp \
		$(QPEDIR)/src/3rdparty/libraries/rsync/qrsync.h \
		$(QPEDIR)/src/3rdparty/libraries/rsync/rsync.h

shutdown.h: shutdown.ui
	$(UIC) shutdown.ui -o $(INTERFACE_DECL_PATH)/shutdown.h

shutdown.cpp: shutdown.ui
	$(UIC) shutdown.ui -i shutdown.h -o shutdown.cpp

syncdialog.h: syncdialog.ui
	$(UIC) syncdialog.ui -o $(INTERFACE_DECL_PATH)/syncdialog.h

syncdialog.cpp: syncdialog.ui
	$(UIC) syncdialog.ui -i syncdialog.h -o syncdialog.cpp

shutdown.o: shutdown.cpp

syncdialog.o: syncdialog.cpp

moc_desktop.o: moc_desktop.cpp \
		desktop.h \
		shutdownimpl.h \
		shutdown.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

moc_qprocess.o: moc_qprocess.cpp \
		qprocess.h

moc_appicons.o: moc_appicons.cpp \
		appicons.h

moc_taskbar.o: moc_taskbar.cpp \
		taskbar.h \
		$(QPEDIR)/include/qpe/custom.h

moc_runningappbar.o: moc_runningappbar.cpp \
		runningappbar.h \
		$(QPEDIR)/include/qtopia/applnk.h

moc_inputmethods.o: moc_inputmethods.cpp \
		inputmethods.h \
		$(QPEDIR)/include/qpe/inputmethodinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h

moc_systray.o: moc_systray.cpp \
		systray.h \
		$(QPEDIR)/include/qpe/taskbarappletinterface.h \
		$(QPEDIR)/include/qpe/qcom.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h

moc_shutdownimpl.o: moc_shutdownimpl.cpp \
		shutdownimpl.h \
		shutdown.h

moc_launcher.o: moc_launcher.cpp \
		launcher.h \
		$(QPEDIR)/include/qpe/config.h \
		launcherview.h \
		$(QPEDIR)/include/qpe/storage.h

moc_launcherview.o: moc_launcherview.cpp \
		launcherview.h \
		$(QPEDIR)/include/qpe/storage.h

$(QPEDIR)/src/settings/calibrate/moc_calibrate.o: $(QPEDIR)/src/settings/calibrate/moc_calibrate.cpp \
		$(QPEDIR)/src/settings/calibrate/calibrate.h

moc_startmenu.o: moc_startmenu.cpp \
		startmenu.h

moc_transferserver.o: moc_transferserver.cpp \
		transferserver.h

moc_qcopbridge.o: moc_qcopbridge.cpp \
		qcopbridge.h

moc_packageslave.o: moc_packageslave.cpp \
		packageslave.h

moc_irserver.o: moc_irserver.cpp \
		irserver.h

moc_devicebuttonmanager.o: moc_devicebuttonmanager.cpp \
		devicebuttonmanager.h \
		devicebutton.h \
		$(QPEDIR)/include/qtopia/services/services.h \
		$(QPEDIR)/include/qtopia/applnk.h

moc_shutdown.o: qprocess_unix.moc \
		qprocess_unix.cpp \
		qprocess.h \
		qcleanuphandler_p.h

moc_syncdialog.o: appicons.moc \
		appicons.cpp \
		appicons.h \
		$(QPEDIR)/include/qpe/qcopenvelope_qws.h

moc_desktop.cpp: desktop.h
	$(MOC) desktop.h -o moc_desktop.cpp

moc_qprocess.cpp: qprocess.h
	$(MOC) qprocess.h -o moc_qprocess.cpp

moc_appicons.cpp: appicons.h
	$(MOC) appicons.h -o moc_appicons.cpp

moc_taskbar.cpp: taskbar.h
	$(MOC) taskbar.h -o moc_taskbar.cpp

moc_runningappbar.cpp: runningappbar.h
	$(MOC) runningappbar.h -o moc_runningappbar.cpp

moc_inputmethods.cpp: inputmethods.h
	$(MOC) inputmethods.h -o moc_inputmethods.cpp

moc_systray.cpp: systray.h
	$(MOC) systray.h -o moc_systray.cpp

moc_shutdownimpl.cpp: shutdownimpl.h
	$(MOC) shutdownimpl.h -o moc_shutdownimpl.cpp

moc_launcher.cpp: launcher.h
	$(MOC) launcher.h -o moc_launcher.cpp

moc_launcherview.cpp: launcherview.h
	$(MOC) launcherview.h -o moc_launcherview.cpp

$(QPEDIR)/src/settings/calibrate/moc_calibrate.cpp: $(QPEDIR)/src/settings/calibrate/calibrate.h
	$(MOC) $(QPEDIR)/src/settings/calibrate/calibrate.h -o $(QPEDIR)/src/settings/calibrate/moc_calibrate.cpp

moc_startmenu.cpp: startmenu.h
	$(MOC) startmenu.h -o moc_startmenu.cpp

moc_transferserver.cpp: transferserver.h
	$(MOC) transferserver.h -o moc_transferserver.cpp

moc_qcopbridge.cpp: qcopbridge.h
	$(MOC) qcopbridge.h -o moc_qcopbridge.cpp

moc_packageslave.cpp: packageslave.h
	$(MOC) packageslave.h -o moc_packageslave.cpp

moc_irserver.cpp: irserver.h
	$(MOC) irserver.h -o moc_irserver.cpp

moc_devicebuttonmanager.cpp: devicebuttonmanager.h
	$(MOC) devicebuttonmanager.h -o moc_devicebuttonmanager.cpp

qprocess_unix.moc: qprocess_unix.cpp
	$(MOC) qprocess_unix.cpp -o qprocess_unix.moc

appicons.moc: appicons.cpp
	$(MOC) appicons.cpp -o appicons.moc

moc_shutdown.cpp: shutdown.h
	$(MOC) shutdown.h -o moc_shutdown.cpp

moc_syncdialog.cpp: syncdialog.h
	$(MOC) syncdialog.h -o moc_syncdialog.cpp



lupdate:
	lupdate server.pro

lrelease:
	lrelease server.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: server.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make server
