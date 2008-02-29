CONFIG += depend_includepath

QMAKE_INCREMENTAL =
SKIP_DEPENDS += qconfig.h qmodules.h
DEFINES += QT_NO_TEXTCODEC QT_NO_LIBRARY QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES \
           QT_NO_GEOM_VARIANT QT_NO_DATASTREAM

#qmake code
SOURCES += project.cpp property.cpp main.cpp generators/makefile.cpp \
           generators/unix/unixmake2.cpp generators/unix/unixmake.cpp meta.cpp \
           option.cpp generators/win32/winmakefile.cpp generators/win32/mingw_make.cpp \
           generators/makefiledeps.cpp generators/metamakefile.cpp generators/mac/pbuilder_pbx.cpp \
           generators/xmloutput.cpp
HEADERS += project.h property.h generators/makefile.h \
           generators/unix/unixmake.h meta.h option.h cachekeys.h \
           generators/win32/winmakefile.h generators/projectgenerator.h \
           generators/makefiledeps.h generators/metamakefile.h generators/mac/pbuilder_pbx.h \
           generators/xmloutput.h
contains(QT_EDITION, OpenSource) {
   DEFINES += QMAKE_OPENSOURCE_EDITION
} else {
   SOURCES +=  generators/win32/borland_bmake.cpp generators/win32/msvc_nmake.cpp \
               generators/projectgenerator.cpp \
               generators/win32/msvc_dsp.cpp generators/win32/msvc_vcproj.cpp \
	       generators/win32/msvc_objectmodel.cpp
   HEADERS +=  generators/win32/borland_bmake.h generators/win32/msvc_nmake.h \
               generators/win32/msvc_dsp.h generators/win32/msvc_vcproj.h \
               nerators/win32/mingw_make.h \
	       generators/win32/msvc_objectmodel.h
}

bootstrap { #Qt code
   DEFINES+=QT_NODLL QT_NO_THREAD
   SOURCES+= \
        qbitarray.cpp \
        qbuffer.cpp \
        qbytearray.cpp \
        qbytearraymatcher.cpp \
        qchar.cpp \
        qdatetime.cpp \
        qdir.cpp \
        qfile.cpp \
        qabstractfileengine.cpp \
        qfileinfo.cpp \
        qfsfileengine.cpp \
        qglobal.cpp \
        qhash.cpp \
        qiodevice.cpp \
        qlistdata.cpp \
        qlocale.cpp \
        qmap.cpp \
        qmetatype.cpp \
        qregexp.cpp \
        qstring.cpp \
        qstringlist.cpp \
        qstringmatcher.cpp \
        qtemporaryfile.cpp \
        qtextstream.cpp \
        qurl.cpp \
        qunicodetables.cpp \
        quuid.cpp \
	qsettings.cpp \
	qlibraryinfo.cpp \
	qvariant.cpp \
        qvector.cpp \
        qvsnprintf.cpp \
        md5.cpp

   HEADERS+= \
        qbitarray.h \
        qbuffer.h \
        qbytearray.h \
        qbytearraymatcher.h \
        qchar.h \
        qdatetime.h \
        qdatetime_p.h \
        qdir.h \
        qfile.h \
        qabstractfileengine.h \
        qfileinfo.h \
        qglobal.h \
        qhash.h \
        qiodevice.h \
        qlist.h \
        qlocale.h \
        qmap.h \
        qmetatype.h \
        qregexp.h \
        qstring.h \
        qstringlist.h \
        qstringmatcher.h \
        qtemporaryfile.h \
        qtextstream.h \
        qurl.h \
        quuid.h \
        qvector.h \
        md5.h

    unix {
        SOURCES += qfsfileengine_unix.cpp
        mac {
          SOURCES += qcore_mac.cpp qsettings_mac.cpp
          QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
          LIBS += -framework CoreServices
        }
    } else:win32 {
	SOURCES += qfsfileengine_win.cpp qsettings_win.cpp
        win32-msvc*:LIBS += ole32.lib advapi32.lib
    }

    qnx {
        CFLAGS += -fhonor-std
        LFLAGS += -lcpp
    }
    DEFINES *= QT_NO_QOBJECT
} else {
    CONFIG += qt
    QT = core
}
*-g++:profiling {
  QMAKE_CFLAGS = -pg
  QMAKE_CXXFLAGS = -pg
  QMAKE_LFLAGS = -pg
}

PRECOMPILED_HEADER = qmake_pch.h
