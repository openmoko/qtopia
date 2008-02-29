#############################################################################
# Automatically generated from applications/datebook/Makefile.in
# Build options from 
#############################################################################

default: all ln_quicklaunch # package

CONTROL = applications/datebook/qpe-datebook.control

ln_quicklaunch:
	-cd $(QPEDIR)/bin ; ln -sf quicklauncher datebook

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
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QTOPIA) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS) $(SYSCONF_CXXFLAGS_LIB) -DQTOPIA_APP_INTERFACE
CC	=	$(SYSCONF_CC) $(QT_C_MT)
CFLAGS	=	$(SYSCONF_CFLAGS_QTOPIA) $(SYSCONF_CFLAGS) $(SYSCONF_CFLAGS_LIB) -DQTOPIA_APP_INTERFACE
INCPATH =	-I$(MOC_DIR)
LFLAGS	=	$(SYSCONF_LFLAGS_QTOPIA) $(SYSCONF_RPATH_QTOPIA) $(SYSCONF_LFLAGS_QT) $(SYSCONF_RPATH_QT) $(SYSCONF_LFLAGS) $(QT_LFLAGS_MT)
LIBS	=	$(SUBLIBS) -lqpepim -luuid -lqpe $(SYSCONF_LIBS_QTOPIA) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_QTAPP)
MOC	=	$(SYSCONF_MOC)
UIC	=	$(SYSCONF_UIC)

####### Target

DESTDIR = $(QPEDIR)/plugins/application/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= datebook
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	dayview.h \
		datebook.h \
		dateentryimpl.h \
		dayviewheaderimpl.h \
		datebooksettings.h \
		datebookweek.h \
		datebookweekheaderimpl.h \
		monthview.h \
		layoutmanager.h \
		periodview.h \
		repeatentry.h \
		datebookdb.h \
		finddialog.h \
		findwidget_p.h
SOURCES =	dayview.cpp \
		datebook.cpp \
		dateentryimpl.cpp \
		dayviewheaderimpl.cpp \
		datebooksettings.cpp \
		datebookweek.cpp \
		datebookweekheaderimpl.cpp \
		monthview.cpp \
		layoutmanager.cpp \
		periodview.cpp \
		repeatentry.cpp \
		datebookdb.cpp \
		finddialog.cpp \
		findwidget_p.cpp \
		main.cpp
OBJECTS =	$(OBJECTS_DIR)/dayview.o \
		$(OBJECTS_DIR)/datebook.o \
		$(OBJECTS_DIR)/dateentryimpl.o \
		$(OBJECTS_DIR)/dayviewheaderimpl.o \
		$(OBJECTS_DIR)/datebooksettings.o \
		$(OBJECTS_DIR)/datebookweek.o \
		$(OBJECTS_DIR)/datebookweekheaderimpl.o \
		$(OBJECTS_DIR)/monthview.o \
		$(OBJECTS_DIR)/layoutmanager.o \
		$(OBJECTS_DIR)/periodview.o \
		$(OBJECTS_DIR)/repeatentry.o \
		$(OBJECTS_DIR)/datebookdb.o \
		$(OBJECTS_DIR)/finddialog.o \
		$(OBJECTS_DIR)/findwidget_p.o \
		$(OBJECTS_DIR)/main.o \
		$(OBJECTS_DIR)/dateentry.o \
		$(OBJECTS_DIR)/dayviewheader.o \
		$(OBJECTS_DIR)/datebooksettingsbase.o \
		$(OBJECTS_DIR)/datebookweekheader.o \
		$(OBJECTS_DIR)/findwidgetbase_p.o
INTERFACES =    dateentry.ui \
		dayviewheader.ui \
		datebooksettingsbase.ui \
		datebookweekheader.ui \
		findwidgetbase_p.ui
UICDECLS =      dateentry.h \
		dayviewheader.h \
		datebooksettingsbase.h \
		datebookweekheader.h \
		findwidgetbase_p.h
UICIMPLS =      dateentry.cpp \
		dayviewheader.cpp \
		datebooksettingsbase.cpp \
		datebookweekheader.cpp \
		findwidgetbase_p.cpp
SRCMOC	=	$(MOC_DIR)/moc_dayview.cpp \
		$(MOC_DIR)/moc_datebook.cpp \
		$(MOC_DIR)/moc_dateentryimpl.cpp \
		$(MOC_DIR)/moc_dayviewheaderimpl.cpp \
		$(MOC_DIR)/moc_datebooksettings.cpp \
		$(MOC_DIR)/moc_datebookweek.cpp \
		$(MOC_DIR)/moc_datebookweekheaderimpl.cpp \
		$(MOC_DIR)/moc_monthview.cpp \
		$(MOC_DIR)/moc_periodview.cpp \
		$(MOC_DIR)/moc_repeatentry.cpp \
		$(MOC_DIR)/moc_datebookdb.cpp \
		$(MOC_DIR)/moc_finddialog.cpp \
		$(MOC_DIR)/moc_findwidget_p.cpp \
		$(MOC_DIR)/moc_dateentry.cpp \
		$(MOC_DIR)/moc_dayviewheader.cpp \
		$(MOC_DIR)/moc_datebooksettingsbase.cpp \
		$(MOC_DIR)/moc_datebookweekheader.cpp \
		$(MOC_DIR)/moc_findwidgetbase_p.cpp \
		$(MOC_DIR)/datebookweek.moc \
		$(MOC_DIR)/repeatentry.moc
OBJMOC	=	$(OBJECTS_DIR)/moc_dayview.o \
		$(OBJECTS_DIR)/moc_datebook.o \
		$(OBJECTS_DIR)/moc_dateentryimpl.o \
		$(OBJECTS_DIR)/moc_dayviewheaderimpl.o \
		$(OBJECTS_DIR)/moc_datebooksettings.o \
		$(OBJECTS_DIR)/moc_datebookweek.o \
		$(OBJECTS_DIR)/moc_datebookweekheaderimpl.o \
		$(OBJECTS_DIR)/moc_monthview.o \
		$(OBJECTS_DIR)/moc_periodview.o \
		$(OBJECTS_DIR)/moc_repeatentry.o \
		$(OBJECTS_DIR)/moc_datebookdb.o \
		$(OBJECTS_DIR)/moc_finddialog.o \
		$(OBJECTS_DIR)/moc_findwidget_p.o \
		$(OBJECTS_DIR)/moc_dateentry.o \
		$(OBJECTS_DIR)/moc_dayviewheader.o \
		$(OBJECTS_DIR)/moc_datebooksettingsbase.o \
		$(OBJECTS_DIR)/moc_datebookweekheader.o \
		$(OBJECTS_DIR)/moc_findwidgetbase_p.o


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

$(OBJECTS_DIR)/dayview.o: dayview.cpp \
		dayview.h \
		periodview.h \
		dayviewheaderimpl.h \
		dayviewheader.h \
		layoutmanager.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dayview.o dayview.cpp

$(OBJECTS_DIR)/datebook.o: datebook.cpp \
		datebook.h \
		datebookdb.h \
		dayview.h \
		periodview.h \
		datebooksettings.h \
		datebooksettingsbase.h \
		datebookweek.h \
		layoutmanager.h \
		dateentryimpl.h \
		dateentry.h \
		monthview.h \
		finddialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebook.o datebook.cpp

$(OBJECTS_DIR)/dateentryimpl.o: dateentryimpl.cpp \
		dateentryimpl.h \
		dateentry.h \
		repeatentry.h \
		nulldb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dateentryimpl.o dateentryimpl.cpp

$(OBJECTS_DIR)/dayviewheaderimpl.o: dayviewheaderimpl.cpp \
		dayviewheaderimpl.h \
		dayviewheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dayviewheaderimpl.o dayviewheaderimpl.cpp

$(OBJECTS_DIR)/datebooksettings.o: datebooksettings.cpp \
		datebooksettings.h \
		datebooksettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebooksettings.o datebooksettings.cpp

$(OBJECTS_DIR)/datebookweek.o: datebookweek.cpp \
		$(MOC_DIR)/datebookweek.moc \
		datebookweek.h \
		periodview.h \
		layoutmanager.h \
		datebookdb.h \
		datebookweekheaderimpl.h \
		datebookweekheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookweek.o datebookweek.cpp

$(OBJECTS_DIR)/datebookweekheaderimpl.o: datebookweekheaderimpl.cpp \
		datebookweekheaderimpl.h \
		datebookweekheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookweekheaderimpl.o datebookweekheaderimpl.cpp

$(OBJECTS_DIR)/monthview.o: monthview.cpp \
		monthview.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/monthview.o monthview.cpp

$(OBJECTS_DIR)/layoutmanager.o: layoutmanager.cpp \
		layoutmanager.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/layoutmanager.o layoutmanager.cpp

$(OBJECTS_DIR)/periodview.o: periodview.cpp \
		periodview.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/periodview.o periodview.cpp

$(OBJECTS_DIR)/repeatentry.o: repeatentry.cpp \
		$(MOC_DIR)/repeatentry.moc \
		repeatentry.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/repeatentry.o repeatentry.cpp

$(OBJECTS_DIR)/datebookdb.o: datebookdb.cpp \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookdb.o datebookdb.cpp

$(OBJECTS_DIR)/finddialog.o: finddialog.cpp \
		finddialog.h \
		findwidget_p.h \
		findwidgetbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/finddialog.o finddialog.cpp

$(OBJECTS_DIR)/findwidget_p.o: findwidget_p.cpp \
		findwidget_p.h \
		findwidgetbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/findwidget_p.o findwidget_p.cpp

$(OBJECTS_DIR)/main.o: main.cpp \
		datebook.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/main.o main.cpp

dateentry.h: dateentry.ui
	$(UIC) dateentry.ui -o $(INTERFACE_DECL_PATH)/dateentry.h

dateentry.cpp: dateentry.ui
	$(UIC) dateentry.ui -i dateentry.h -o dateentry.cpp

dayviewheader.h: dayviewheader.ui
	$(UIC) dayviewheader.ui -o $(INTERFACE_DECL_PATH)/dayviewheader.h

dayviewheader.cpp: dayviewheader.ui
	$(UIC) dayviewheader.ui -i dayviewheader.h -o dayviewheader.cpp

datebooksettingsbase.h: datebooksettingsbase.ui
	$(UIC) datebooksettingsbase.ui -o $(INTERFACE_DECL_PATH)/datebooksettingsbase.h

datebooksettingsbase.cpp: datebooksettingsbase.ui
	$(UIC) datebooksettingsbase.ui -i datebooksettingsbase.h -o datebooksettingsbase.cpp

datebookweekheader.h: datebookweekheader.ui
	$(UIC) datebookweekheader.ui -o $(INTERFACE_DECL_PATH)/datebookweekheader.h

datebookweekheader.cpp: datebookweekheader.ui
	$(UIC) datebookweekheader.ui -i datebookweekheader.h -o datebookweekheader.cpp

findwidgetbase_p.h: findwidgetbase_p.ui
	$(UIC) findwidgetbase_p.ui -o $(INTERFACE_DECL_PATH)/findwidgetbase_p.h

findwidgetbase_p.cpp: findwidgetbase_p.ui
	$(UIC) findwidgetbase_p.ui -i findwidgetbase_p.h -o findwidgetbase_p.cpp

$(OBJECTS_DIR)/dateentry.o: dateentry.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dateentry.o dateentry.cpp

$(OBJECTS_DIR)/dayviewheader.o: dayviewheader.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/dayviewheader.o dayviewheader.cpp

$(OBJECTS_DIR)/datebooksettingsbase.o: datebooksettingsbase.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebooksettingsbase.o datebooksettingsbase.cpp

$(OBJECTS_DIR)/datebookweekheader.o: datebookweekheader.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/datebookweekheader.o datebookweekheader.cpp

$(OBJECTS_DIR)/findwidgetbase_p.o: findwidgetbase_p.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/findwidgetbase_p.o findwidgetbase_p.cpp

$(OBJECTS_DIR)/moc_dayview.o: $(MOC_DIR)/moc_dayview.cpp \
		dayview.h \
		periodview.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_dayview.o $(MOC_DIR)/moc_dayview.cpp

$(OBJECTS_DIR)/moc_datebook.o: $(MOC_DIR)/moc_datebook.cpp \
		datebook.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebook.o $(MOC_DIR)/moc_datebook.cpp

$(OBJECTS_DIR)/moc_dateentryimpl.o: $(MOC_DIR)/moc_dateentryimpl.cpp \
		dateentryimpl.h \
		dateentry.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_dateentryimpl.o $(MOC_DIR)/moc_dateentryimpl.cpp

$(OBJECTS_DIR)/moc_dayviewheaderimpl.o: $(MOC_DIR)/moc_dayviewheaderimpl.cpp \
		dayviewheaderimpl.h \
		dayviewheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_dayviewheaderimpl.o $(MOC_DIR)/moc_dayviewheaderimpl.cpp

$(OBJECTS_DIR)/moc_datebooksettings.o: $(MOC_DIR)/moc_datebooksettings.cpp \
		datebooksettings.h \
		datebooksettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebooksettings.o $(MOC_DIR)/moc_datebooksettings.cpp

$(OBJECTS_DIR)/moc_datebookweek.o: $(MOC_DIR)/moc_datebookweek.cpp \
		datebookweek.h \
		periodview.h \
		layoutmanager.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebookweek.o $(MOC_DIR)/moc_datebookweek.cpp

$(OBJECTS_DIR)/moc_datebookweekheaderimpl.o: $(MOC_DIR)/moc_datebookweekheaderimpl.cpp \
		datebookweekheaderimpl.h \
		datebookweekheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebookweekheaderimpl.o $(MOC_DIR)/moc_datebookweekheaderimpl.cpp

$(OBJECTS_DIR)/moc_monthview.o: $(MOC_DIR)/moc_monthview.cpp \
		monthview.h \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_monthview.o $(MOC_DIR)/moc_monthview.cpp

$(OBJECTS_DIR)/moc_periodview.o: $(MOC_DIR)/moc_periodview.cpp \
		periodview.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_periodview.o $(MOC_DIR)/moc_periodview.cpp

$(OBJECTS_DIR)/moc_repeatentry.o: $(MOC_DIR)/moc_repeatentry.cpp \
		repeatentry.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_repeatentry.o $(MOC_DIR)/moc_repeatentry.cpp

$(OBJECTS_DIR)/moc_datebookdb.o: $(MOC_DIR)/moc_datebookdb.cpp \
		datebookdb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebookdb.o $(MOC_DIR)/moc_datebookdb.cpp

$(OBJECTS_DIR)/moc_finddialog.o: $(MOC_DIR)/moc_finddialog.cpp \
		finddialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_finddialog.o $(MOC_DIR)/moc_finddialog.cpp

$(OBJECTS_DIR)/moc_findwidget_p.o: $(MOC_DIR)/moc_findwidget_p.cpp \
		findwidget_p.h \
		findwidgetbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_findwidget_p.o $(MOC_DIR)/moc_findwidget_p.cpp

$(OBJECTS_DIR)/moc_dateentry.o: $(MOC_DIR)/moc_dateentry.cpp \
		dateentry.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_dateentry.o $(MOC_DIR)/moc_dateentry.cpp

$(OBJECTS_DIR)/moc_dayviewheader.o: $(MOC_DIR)/moc_dayviewheader.cpp \
		dayviewheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_dayviewheader.o $(MOC_DIR)/moc_dayviewheader.cpp

$(OBJECTS_DIR)/moc_datebooksettingsbase.o: $(MOC_DIR)/moc_datebooksettingsbase.cpp \
		datebooksettingsbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebooksettingsbase.o $(MOC_DIR)/moc_datebooksettingsbase.cpp

$(OBJECTS_DIR)/moc_datebookweekheader.o: $(MOC_DIR)/moc_datebookweekheader.cpp \
		datebookweekheader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_datebookweekheader.o $(MOC_DIR)/moc_datebookweekheader.cpp

$(OBJECTS_DIR)/moc_findwidgetbase_p.o: $(MOC_DIR)/moc_findwidgetbase_p.cpp \
		findwidgetbase_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/moc_findwidgetbase_p.o $(MOC_DIR)/moc_findwidgetbase_p.cpp

$(MOC_DIR)/moc_dayview.cpp: dayview.h
	$(MOC) dayview.h -o $(MOC_DIR)/moc_dayview.cpp

$(MOC_DIR)/moc_datebook.cpp: datebook.h
	$(MOC) datebook.h -o $(MOC_DIR)/moc_datebook.cpp

$(MOC_DIR)/moc_dateentryimpl.cpp: dateentryimpl.h
	$(MOC) dateentryimpl.h -o $(MOC_DIR)/moc_dateentryimpl.cpp

$(MOC_DIR)/moc_dayviewheaderimpl.cpp: dayviewheaderimpl.h
	$(MOC) dayviewheaderimpl.h -o $(MOC_DIR)/moc_dayviewheaderimpl.cpp

$(MOC_DIR)/moc_datebooksettings.cpp: datebooksettings.h
	$(MOC) datebooksettings.h -o $(MOC_DIR)/moc_datebooksettings.cpp

$(MOC_DIR)/moc_datebookweek.cpp: datebookweek.h
	$(MOC) datebookweek.h -o $(MOC_DIR)/moc_datebookweek.cpp

$(MOC_DIR)/moc_datebookweekheaderimpl.cpp: datebookweekheaderimpl.h
	$(MOC) datebookweekheaderimpl.h -o $(MOC_DIR)/moc_datebookweekheaderimpl.cpp

$(MOC_DIR)/moc_monthview.cpp: monthview.h
	$(MOC) monthview.h -o $(MOC_DIR)/moc_monthview.cpp

$(MOC_DIR)/moc_periodview.cpp: periodview.h
	$(MOC) periodview.h -o $(MOC_DIR)/moc_periodview.cpp

$(MOC_DIR)/moc_repeatentry.cpp: repeatentry.h
	$(MOC) repeatentry.h -o $(MOC_DIR)/moc_repeatentry.cpp

$(MOC_DIR)/moc_datebookdb.cpp: datebookdb.h
	$(MOC) datebookdb.h -o $(MOC_DIR)/moc_datebookdb.cpp

$(MOC_DIR)/moc_finddialog.cpp: finddialog.h
	$(MOC) finddialog.h -o $(MOC_DIR)/moc_finddialog.cpp

$(MOC_DIR)/moc_findwidget_p.cpp: findwidget_p.h
	$(MOC) findwidget_p.h -o $(MOC_DIR)/moc_findwidget_p.cpp

$(MOC_DIR)/datebookweek.moc: datebookweek.cpp
	$(MOC) datebookweek.cpp -o $(MOC_DIR)/datebookweek.moc

$(MOC_DIR)/repeatentry.moc: repeatentry.cpp
	$(MOC) repeatentry.cpp -o $(MOC_DIR)/repeatentry.moc

$(MOC_DIR)/moc_dateentry.cpp: dateentry.h
	$(MOC) dateentry.h -o $(MOC_DIR)/moc_dateentry.cpp

$(MOC_DIR)/moc_dayviewheader.cpp: dayviewheader.h
	$(MOC) dayviewheader.h -o $(MOC_DIR)/moc_dayviewheader.cpp

$(MOC_DIR)/moc_datebooksettingsbase.cpp: datebooksettingsbase.h
	$(MOC) datebooksettingsbase.h -o $(MOC_DIR)/moc_datebooksettingsbase.cpp

$(MOC_DIR)/moc_datebookweekheader.cpp: datebookweekheader.h
	$(MOC) datebookweekheader.h -o $(MOC_DIR)/moc_datebookweekheader.cpp

$(MOC_DIR)/moc_findwidgetbase_p.cpp: findwidgetbase_p.h
	$(MOC) findwidgetbase_p.h -o $(MOC_DIR)/moc_findwidgetbase_p.cpp



lupdate-check:
	@sed -e '/$$$$QTOPIA1DIR[^ ]*/ d' \
	    -e '/$$(.*/ d' \
	    -e 's/^static:.*//' \
	    <datebook.pro >t$$$$.pro && \
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
	    <datebook.pro >t$$$$.pro && \
	lupdate  t$$$$.pro && \
	rm t$$$$.pro

lrelease:
	lrelease datebook.pro
	$(QPEDIR)/bin/qtopia-linstall datebook.pro


ipkg: $(DESTDIR)$(SYSCONF_LINK_TARGET) # among other things...
	CTL="$(CONTROL)"; for ctrl in $$CTL; do cd $(QPEDIR)/ipkg; ../bin/mkipks -platform generic -quicklaunch $(QPEDIR)/src/$$ctrl ; done

package: ipkg
	cd $(QPEDIR)/ipkg; 	../bin/mkPackages

Makefile: datebook.pro
	cd $(QPEDIR)/src; 	./configure  -platform generic -make applications/datebook
