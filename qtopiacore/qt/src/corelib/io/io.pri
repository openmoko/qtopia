# Qt core io module

HEADERS +=  \
        io/qabstractfileengine.h \
        io/qabstractfileengine_p.h \
        io/qbuffer.h \
        io/qdatastream.h \
        io/qdebug.h \
        io/qdir.h \
        io/qfile.h \
        io/qfileinfo.h \
        io/qiodevice.h \
        io/qiodevice_p.h \
        io/qprocess.h \
        io/qprocess_p.h \
        io/qtextstream.h \
        io/qtemporaryfile.h \
        io/qresource_p.h \
        io/qurl.h \
        io/qsettings.h \
        io/qsettings_p.h \
        io/qfsfileengine.h \
        io/qfsfileengine_p.h \
        io/qfilesystemwatcher.h \
        io/qfilesystemwatcher_p.h

SOURCES += \
        io/qabstractfileengine.cpp \
        io/qbuffer.cpp \
        io/qdatastream.cpp \
        io/qdebug.cpp \
        io/qdir.cpp \
        io/qfile.cpp \
        io/qfileinfo.cpp \
        io/qiodevice.cpp \
        io/qprocess.cpp \
        io/qtextstream.cpp \
        io/qtemporaryfile.cpp \
        io/qresource.cpp \
        io/qurl.cpp \
        io/qsettings.cpp \
        io/qfsfileengine.cpp \
        io/qfilesystemwatcher.cpp

win32 {
        SOURCES += io/qsettings_win.cpp
        SOURCES += io/qprocess_win.cpp
        wince-*:SOURCES += io/qfsfileengine_wce.cpp
        else:SOURCES += io/qfsfileengine_win.cpp

        SOURCES += io/qfilesystemwatcher_win.cpp
        HEADERS += io/qfilesystemwatcher_win_p.h
} else:unix {
        SOURCES += io/qfsfileengine_unix.cpp
        SOURCES += io/qprocess_unix.cpp
        mac:SOURCES += io/qsettings_mac.cpp

        linux-*:{
            SOURCES += io/qfilesystemwatcher_inotify.cpp
            HEADERS += io/qfilesystemwatcher_inotify_p.h
        }

        freebsd-*|macx-*:{
            SOURCES += io/qfilesystemwatcher_kqueue.cpp
            HEADERS += io/qfilesystemwatcher_kqueue_p.h
        }
}
