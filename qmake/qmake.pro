#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles
QMAKE_INCREMENTAL = 
SKIP_DEPENDS += qconfig.h qmodules.h
CONFIG += console
CONFIG -= opengl qt shared x11sm
DESTDIR = ../bin/
DEPENDPATH += generators generators/unix generators/win32 \
              generators/mac $$QT_SOURCE_TREE/include $$QT_SOURCE_TREE/qmake
INCLUDEPATH += $$DEPENDPATH .
DEFINES += QT_NO_TEXTCODEC QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES
DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES QT_NODLL

#qmake code
SOURCES+=project.cpp property.cpp main.cpp makefile.cpp \
         unixmake2.cpp unixmake.cpp meta.cpp \
         borland_bmake.cpp msvc_nmake.cpp \
	 msvc_dsp.cpp msvc_vcproj.cpp option.cpp winmakefile.cpp \
	 projectgenerator.cpp metrowerks_xml.cpp mingw_make.cpp \
         pbuilder_pbx.cpp msvc_objectmodel.cpp

#Qt code
SOURCES+=qstring.cpp qtextstream.cpp \
         qiodevice.cpp qglobal.cpp \
	 qgdict.cpp qcstring.cpp \
	 qdatastream.cpp qgarray.cpp \
	 qbuffer.cpp qglist.cpp \
	 qptrcollection.cpp qfile.cpp \
	 qregexp.cpp quuid.cpp \
	 qgvector.cpp qgcache.cpp \
	 qbitarray.cpp qdir.cpp \
	 qfileinfo.cpp qdatetime.cpp \
	 qstringlist.cpp qmap.cpp \
       qsettings.cpp qunicodetables.cpp \
	 qlibrary.cpp 

exists($$QT_BUILD_TREE/src/tools/qconfig.cpp) {  #qconfig.cpp
    DEFINES += HAVE_QCONFIG_CPP                                           
    SOURCES += $$QT_BUILD_TREE/src/tools/qconfig.cpp
}

#where to find the Qt code, and platform dependant SOURCES
unix {
   VPATH += $$QT_SOURCE_TREE/src/tools
   SOURCES += qfile_unix.cpp qfileinfo_unix.cpp qdir_unix.cpp
   mac {
     VPATH += $$QT_SOURCE_TREE/src/kernel
     SOURCES += qsettings_mac.cpp qurl.cpp
   }
}
win32 {
   VPATH += $$QT_SOURCE_TREE/src/tools
   SOURCES += qfile_win.cpp qfileinfo_win.cpp qdir_win.cpp qsettings_win.cpp qlibrary_win.cpp
   *-msvc:LIBS += ole32.lib advapi32.lib
}
macx-*: LIBS += -framework CoreServices

qnx {
    CFLAGS += -fhonor-std
    LFLAGS += -lcpp
}
#installation
target.path=$$bins.path
INSTALLS        += target



mkspecs.path=$$data.path/mkspecs
mkspecs.files=$$QT_SOURCE_TREE/mkspecs
INSTALLS        += mkspecs

*-g++:profiling {
  QMAKE_CFLAGS = -pg
  QMAKE_CXXFLAGS = -pg
  QMAKE_LFLAGS = -pg
}
