#############################################################################
# Automatically generated from applications/datebook/Makefile.in
# Build options from 
#############################################################################

default: all # package

CONTROL = applications/datebook/qpe-datebook.control

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
LIBS	=	$(SUBLIBS) -lqpe -lqpepim -luuid $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/bin/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= datebook
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	datebookday.h \
		datebook.h \
		dateentryimpl.h \
		datebookdayheaderimpl.h \
		datebooksettings.h \
		datebookweek.h \
		datebookweekheaderimpl.h \
		monthview.h \
		layoutmanager.h \
		periodview.h \
		repeatentry.h \
		datebookdb.h \
		datepicker.h \
		datetimeedit.h
SOURCES =	main.cpp \
		datebookday.cpp \
		datebook.cpp \
		dateentryimpl.cpp \
		datebookdayheaderimpl.cpp \
		datebooksettings.cpp \
		datebookweek.cpp \
		datebookweekheaderimpl.cpp \
		monthview.cpp \
		layoutmanager.cpp \
		periodview.cpp \
		repeatentry.cpp \
		datebookdb.cpp \
		datepicker.cpp \
		datetimeedit.cpp
OBJECTS =	main.o \
		datebookday.o \
		datebook.o \
		dateentryimpl.o \
		datebookdayheaderimpl.o \
		datebooksettings.o \
		datebookweek.o \
		datebookweekheaderimpl.o \
		monthview.o \
		layoutmanager.o \
		periodview.o \
		repeatentry.o \
		datebookdb.o \
		datepicker.o \
		datetimeedit.o \
		dateentry.o \
		datebookdayheader.o \
		datebooksettingsbase.o \
		datebookweekheader.o
INTERFACES =    dateentry.ui \
		datebookdayheader.ui \
		datebooksettingsbase.ui \
		datebookweekheader.ui
UICDECLS =      dateentry.h \
		datebookdayheader.h \
		datebooksettingsbase.h \
		datebookweekheader.h
UICIMPLS =      dateentry.cpp \
		datebookdayheader.cpp \
		datebooksettingsbase.cpp \
		datebookweekheader.cpp
SRCMOC	=	moc_datebookday.cpp \
		moc_datebook.cpp \
		moc_dateentryimpl.cpp \
		moc_datebookdayheaderimpl.cpp \
		moc_datebookweek.cpp \
		moc_datebookweekheaderimpl.cpp \
		moc_monthview.cpp \
		moc_periodview.cpp \
		moc_repeatentry.cpp \
		moc_datebookdb.cpp \
		moc_datepicker.cpp \
		moc_datetimeedit.cpp \
		datebookweek.moc \
		repeatentry.moc \
		datepicker.moc \
		moc_dateentry.cpp \
		moc_datebookdayheader.cpp \
		moc_datebooksettingsbase.cpp \
		moc_datebookweekheader.cpp
OBJMOC	=	moc_datebookday.o \
		moc_datebook.o \
		moc_dateentryimpl.o \
		moc_datebookdayheaderimpl.o \
		moc_datebookweek.o \
		moc_datebookweekheaderimpl.o \
		moc_monthview.o \
		moc_periodview.o \
		moc_repeatentry.o \
		moc_datebookdb.o \
		moc_datepicker.o \
		moc_datetimeedit.o \
		moc_dateentry.o \
		moc_datebookdayheader.o \
		moc_datebooksettingsbase.o \
		moc_datebookweekheader.o


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
	tmake datebook.pro

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
		datebook.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		$(QPEDIR)/include/qpe/qpeapplication.h

datebookday.o: datebookday.cpp \
		datebookday.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datebookdayheaderimpl.h \
		datebookdayheader.h \
		layoutmanager.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		datebookdb.h \
		$(QPEDIR)/include/qtopia/ir.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		$(QPEDIR)/include/qtopia/qpedebug.h

datebook.o: datebook.cpp \
		datebook.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datebookday.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		datebooksettings.h \
		datebooksettingsbase.h \
		datebookweek.h \
		layoutmanager.h \
		dateentryimpl.h \
		dateentry.h \
		monthview.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/qpedebug.h \
		$(QPEDIR)/include/qtopia/finddialog.h \
		$(QPEDIR)/include/qtopia/ir.h \
		$(QPEDIR)/include/qtopia/qpemenubar.h \
		$(QPEDIR)/include/qtopia/qpemessagebox.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/sound.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		$(QPEDIR)/include/qtopia/qpetoolbar.h \
		$(QPEDIR)/include/qtopia/tzselect.h \
		$(QPEDIR)/include/qtopia/xmlreader.h \
		$(QPEDIR)/include/qtopia/applnk.h

dateentryimpl.o: dateentryimpl.cpp \
		dateentryimpl.h \
		dateentry.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		repeatentry.h \
		datetimeedit.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qpe/categoryselect.h \
		$(QPEDIR)/include/qpe/global.h \
		$(QPEDIR)/include/qpe/timeconversion.h \
		$(QPEDIR)/include/qpe/tzselect.h

datebookdayheaderimpl.o: datebookdayheaderimpl.cpp \
		datebookdayheaderimpl.h \
		datebookdayheader.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h

datebooksettings.o: datebooksettings.cpp \
		datebooksettings.h \
		datebooksettingsbase.h \
		$(QPEDIR)/include/qpe/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

datebookweek.o: datebookweek.cpp \
		datebookweek.moc \
		datebookweek.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		layoutmanager.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		datebookdb.h \
		datebookweekheaderimpl.h \
		datebookweekheader.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/timestring.h

datebookweekheaderimpl.o: datebookweekheaderimpl.cpp \
		datebookweekheaderimpl.h \
		datebookweekheader.h

monthview.o: monthview.cpp \
		monthview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h \
		$(QPEDIR)/include/qtopia/config.h

layoutmanager.o: layoutmanager.cpp \
		layoutmanager.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datebookdb.h

periodview.o: periodview.cpp \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h

repeatentry.o: repeatentry.cpp \
		repeatentry.moc \
		repeatentry.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h

datebookdb.o: datebookdb.cpp \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		$(QPEDIR)/include/qtopia/global.h \
		$(QPEDIR)/include/qtopia/pim/private/eventxmlio_p.h \
		$(QPEDIR)/include/qtopia/pim/event.h

datepicker.o: datepicker.cpp \
		datepicker.moc \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h

datetimeedit.o: datetimeedit.cpp \
		datetimeedit.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

dateentry.h: dateentry.ui
	$(UIC) dateentry.ui -o $(INTERFACE_DECL_PATH)/dateentry.h

dateentry.cpp: dateentry.ui
	$(UIC) dateentry.ui -i dateentry.h -o dateentry.cpp

datebookdayheader.h: datebookdayheader.ui
	$(UIC) datebookdayheader.ui -o $(INTERFACE_DECL_PATH)/datebookdayheader.h

datebookdayheader.cpp: datebookdayheader.ui
	$(UIC) datebookdayheader.ui -i datebookdayheader.h -o datebookdayheader.cpp

datebooksettingsbase.h: datebooksettingsbase.ui
	$(UIC) datebooksettingsbase.ui -o $(INTERFACE_DECL_PATH)/datebooksettingsbase.h

datebooksettingsbase.cpp: datebooksettingsbase.ui
	$(UIC) datebooksettingsbase.ui -i datebooksettingsbase.h -o datebooksettingsbase.cpp

datebookweekheader.h: datebookweekheader.ui
	$(UIC) datebookweekheader.ui -o $(INTERFACE_DECL_PATH)/datebookweekheader.h

datebookweekheader.cpp: datebookweekheader.ui
	$(UIC) datebookweekheader.ui -i datebookweekheader.h -o datebookweekheader.cpp

dateentry.o: dateentry.cpp

datebookdayheader.o: datebookdayheader.cpp

datebooksettingsbase.o: datebooksettingsbase.cpp

datebookweekheader.o: datebookweekheader.cpp

moc_datebookday.o: moc_datebookday.cpp \
		datebookday.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_datebook.o: moc_datebook.cpp \
		datebook.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_dateentryimpl.o: moc_dateentryimpl.cpp \
		dateentryimpl.h \
		dateentry.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_datebookdayheaderimpl.o: moc_datebookdayheaderimpl.cpp \
		datebookdayheaderimpl.h \
		datebookdayheader.h

moc_datebookweek.o: moc_datebookweek.cpp \
		datebookweek.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		layoutmanager.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		datebookdb.h

moc_datebookweekheaderimpl.o: moc_datebookweekheaderimpl.cpp \
		datebookweekheaderimpl.h \
		datebookweekheader.h

moc_monthview.o: moc_monthview.cpp \
		monthview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h

moc_periodview.o: moc_periodview.cpp \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_repeatentry.o: moc_repeatentry.cpp \
		repeatentry.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_datebookdb.o: moc_datebookdb.cpp \
		datebookdb.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h

moc_datepicker.o: moc_datepicker.cpp \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h

moc_datetimeedit.o: moc_datetimeedit.cpp \
		datetimeedit.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h

moc_dateentry.o: datebookweek.moc \
		datebookweek.cpp \
		datebookweek.h \
		periodview.h \
		$(QPEDIR)/include/qtopia/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		layoutmanager.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		datebookdb.h \
		datebookweekheaderimpl.h \
		datebookweekheader.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/timestring.h

moc_datebookdayheader.o: repeatentry.moc \
		repeatentry.cpp \
		repeatentry.h \
		$(QPEDIR)/include/qpe/pim/event.h \
		$(QPEDIR)/include/qtopia/pim/pimrecord.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/quuid.h \
		$(QPEDIR)/include/qtopia/timeconversion.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h

moc_datebooksettingsbase.o: datepicker.moc \
		datepicker.cpp \
		$(QPEDIR)/include/qtopia/config.h \
		$(QPEDIR)/include/qtopia/resource.h \
		$(QPEDIR)/include/qtopia/qpeapplication.h \
		$(QPEDIR)/include/qtopia/qpeglobal.h \
		$(QPEDIR)/include/qtopia/timestring.h \
		datepicker.h \
		$(QPEDIR)/include/qpe/timestring.h \
		$(QPEDIR)/include/qtopia/pim/calendar.h

moc_datebookweekheader.o: moc_dateentry.cpp \
		dateentry.h

moc_datebookday.cpp: datebookday.h
	$(MOC) datebookday.h -o moc_datebookday.cpp

moc_datebook.cpp: datebook.h
	$(MOC) datebook.h -o moc_datebook.cpp

moc_dateentryimpl.cpp: dateentryimpl.h
	$(MOC) dateentryimpl.h -o moc_dateentryimpl.cpp

moc_datebookdayheaderimpl.cpp: datebookdayheaderimpl.h
	$(MOC) datebookdayheaderimpl.h -o moc_datebookdayheaderimpl.cpp

moc_datebookweek.cpp: datebookweek.h
	$(MOC) datebookweek.h -o moc_datebookweek.cpp

moc_datebookweekheaderimpl.cpp: datebookweekheaderimpl.h
	$(MOC) datebookweekheaderimpl.h -o moc_datebookweekheaderimpl.cpp

moc_monthview.cpp: monthview.h
	$(MOC) monthview.h -o moc_monthview.cpp

moc_periodview.cpp: periodview.h
	$(MOC) periodview.h -o moc_periodview.cpp

moc_repeatentry.cpp: repeatentry.h
	$(MOC) repeatentry.h -o moc_repeatentry.cpp

moc_datebookdb.cpp: datebookdb.h
	$(MOC) datebookdb.h -o moc_datebookdb.cpp

moc_datepicker.cpp: datepicker.h
	$(MOC) datepicker.h -o moc_datepicker.cpp

moc_datetimeedit.cpp: datetimeedit.h
	$(MOC) datetimeedit.h -o moc_datetimeedit.cpp

datebookweek.moc: datebookweek.cpp
	$(MOC) datebookweek.cpp -o datebookweek.moc

repeatentry.moc: repeatentry.cpp
	$(MOC) repeatentry.cpp -o repeatentry.moc

datepicker.moc: datepicker.cpp
	$(MOC) datepicker.cpp -o datepicker.moc

moc_dateentry.cpp: dateentry.h
	$(MOC) dateentry.h -o moc_dateentry.cpp

moc_datebookdayheader.cpp: datebookdayheader.h
	$(MOC) datebookdayheader.h -o moc_datebookdayheader.cpp

moc_datebooksettingsbase.cpp: datebooksettingsbase.h
	$(MOC) datebooksettingsbase.h -o moc_datebooksettingsbase.cpp

moc_datebookweekheader.cpp: datebookweekheader.h
	$(MOC) datebookweekheader.h -o moc_datebookweekheader.cpp



lupdate:
	lupdate datebook.pro

lrelease:
	lrelease datebook.pro

ipkg: $(DESTDIR)$(TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks $(QPEDIR)/src/$$ctrl; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../scripts/mkPackages

Makefile: datebook.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/datebook
