CONFIG+=no_qtopiabase
qtopia_project(qtopia core lib)
TARGET=qtopiabase
CONFIG += no_auto_translatables qtopia_visibility

QTOPIABASE_HEADERS+=\
    qtopiaglobal.h \
    qtopiasxe.h \
    custom.h \
    custom-qtopia.h \
    qlog.h \
    qtopialog.h \
    qtopialog-config.h \
    qsignalintercepter.h \
    qslotinvoker.h \
    qstorage.h \
    qtopianamespace.h \
    qtopiaservices.h \
    qtopiaabstractservice.h \
    qtopiachannel.h \
    qtopiaipcadaptor.h \
    qpowerstatus.h \
    qtopiaipcenvelope.h \
    qtopiaipcmarshal.h \
    qabstractipcinterfacegroup.h \
    qabstractipcinterfacegroupmanager.h \
    qabstractipcinterface.h \
    qsoundcontrol.h \
    quniqueid.h \
    qtranslatablesettings.h \
    qdawg.h \
    version.h \
    qmemoryfile_p.h \
    qglobalpixmapcache.h

QTOPIABASE_PRIVATE_HEADERS+=\
    dbusapplicationchannel_p.h \
    dbusipccommon_p.h \
    qactionconfirm_p.h\
    qcopenvelope_p.h \
    qsharedmemorycache_p.h

QTOPIABASE_SOURCES+=\
    dbusapplicationchannel_p.cpp \
    dbusipccommon_p.cpp \
    qactionconfirm.cpp\
    qabstractipcinterface.cpp \
    qabstractipcinterfacegroup.cpp \
    qabstractipcinterfacegroupmanager.cpp \
    qcopenvelope.cpp \
    qlog.cpp \
    qsignalintercepter.cpp \
    qslotinvoker.cpp \
    qstorage.cpp \
    qtopianamespace.cpp\
    qtopiaservices.cpp \
    qtopiaabstractservice.cpp \
    qtopiachannel.cpp \
    qtopiaipcadaptor.cpp \
    qpowerstatus.cpp \
    qtopiaipcenvelope.cpp \
    qtopiaipcmarshal.cpp \
    qtopialog.cpp \
    qsoundcontrol.cpp \
    quniqueid.cpp \
    qtranslatablesettings.cpp \
    qdawg.cpp \
    qmemoryfile.cpp \
    qsharedmemorycache.cpp

VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiail
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/qtopiail

QTOPIABASE_SOURCES+=\
    framework/applayer.cpp \
    framework/inilayer.cpp \
    framework/qfilemonitor.cpp \
    framework/qmallocpool.cpp \
    framework/qpacketprotocol.cpp \
    framework/qsystemlock.cpp \
    framework/qvaluespace.cpp \
    framework/qexpressionevaluator.cpp \
    framework/qfixedpointnumber.cpp

QTOPIABASE_HEADERS+=\
    framework/qfilemonitor.h \
    framework/qmallocpool.h \
    framework/qpacketprotocol.h \
    framework/qsystemlock.h \
    framework/qvaluespace.h \
    framework/qexpressionevaluator.h \
    framework/qfixedpointnumber_p.h \
    framework/qtopiailglobal.h

PREFIX=QTOPIABASE
resolve_include()

enable_dbusipc:depends(3rdparty/libraries/qtdbus)

#These are being marked for future removal.
#dealing with these should save 200k of the
#stripped lib size.
# contact - used by passwd to print contact info
# event - used by datebookdb
# datebookdb - used by settime to reset alarms
# palmtoprecord - used by contact && event

win32 { 
    SOURCES+=qmemoryfile_win.cpp
        DEFINES+=\
        QTOPIA_INTERNAL_APPLNKASSIGN QTOPIA_INTERNAL_FSLP\
        QTOPIA_INTERNAL_PRELOADACCESS QTOPIA_INTERNAL_FD\
        QTOPIA_WIN32PROCESS_SUPPORT
}

TRANSLATABLES+=qmemoryfile_win.cpp

unix {
    SOURCES+=qmemoryfile_unix.cpp
        LIBS          +=-lm
#!staticlib:LIBS+=$$QMAKE_LIBS_DYNLOAD
}

TRANSLATABLES+=qmemoryfile_unix.cpp \
               qtopianamespace.cpp

sdk_qtopiabase_headers.files=$$QTOPIABASE_HEADERS
sdk_qtopiabase_headers.path=/include/qtopiabase
sdk_qtopiabase_headers.hint=sdk headers

sdk_qtopiabase_private_headers.files=$$QTOPIABASE_PRIVATE_HEADERS
sdk_qtopiabase_private_headers.path=/include/qtopiabase/private
sdk_qtopiabase_private_headers.hint=sdk headers

sdk_qtopiabase_custom_headers.files=custom-*-*.h
sdk_qtopiabase_custom_headers.path=/src/libraries/qtopiabase
sdk_qtopiabase_custom_headers.hint=sdk
INSTALLS+=sdk_qtopiabase_custom_headers

INSTALLS+=sdk_qtopiabase_headers sdk_qtopiabase_private_headers

QTOPIA_CUSTOM=$$QPEDIR/src/libraries/qtopiabase/custom-qtopia
HEADERS+=$${QTOPIA_CUSTOM}.h
SOURCES+=$${QTOPIA_CUSTOM}.cpp

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
