!qbuild{
CONFIG+=no_qtopiabase
qtopia_project(qtopia core lib)
TARGET=qtopiabase
CONFIG += qtopia_visibility

x11 {
    VPATH+=$$QT_DEPOT_PATH/src/gui/embedded
    INCLUDEPATH+=$$QT_DEPOT_PATH/src/gui/embedded
}
depends(3rdparty/libraries/dlmalloc)
}

HEADERS+=\
    custom.h \
    qabstractipcinterfacegroup.h \
    qabstractipcinterfacegroupmanager.h \
    qabstractipcinterface.h \
    qdawg.h \
    qglobalpixmapcache.h \
    qlog.h \
    qsignalintercepter.h \
    qslotinvoker.h \
    qsoundcontrol.h \
    qstorage.h \
    qtopiaabstractservice.h \
    qtopiachannel.h \
    qtopiaglobal.h \
    qtopiaipcadaptor.h \
    qtopiaipcenvelope.h \
    qtopiaipcmarshal.h \
    qtopialog.h \
    qtopialog-config.h \
    qtopianamespace.h \
    qtopiaservices.h \
    qtopiasxe.h \
    qtopiatimer.h \
    qtranslatablesettings.h \
    quniqueid.h \
    version.h \
    # Valuespace code
    qexpressionevaluator.h \
    qfilemonitor.h \
    qmallocpool.h \
    qpacketprotocol.h \
    qsystemlock.h \
    qtopiailglobal.h \
    qvaluespace.h

PRIVATE_HEADERS+=\
    qactionconfirm_p.h \
    qmemoryfile_p.h \
    # Valuespace code
    qfixedpointnumber_p.h

# FIXME this should be public or it should not be used outside of libqtopiabase
SEMI_PRIVATE_HEADERS+=\
    testslaveinterface_p.h\
    qcopenvelope_p.h

SOURCES+=\
    qactionconfirm.cpp\
    qabstractipcinterfacegroup.cpp \
    qabstractipcinterfacegroupmanager.cpp \
    qcopenvelope.cpp \
    qdawg.cpp \
    qlog.cpp \
    qmemoryfile.cpp \
    qmemoryfile_unix.cpp \
    qsignalintercepter.cpp \
    qslotinvoker.cpp \
    qsoundcontrol.cpp \
    qstorage.cpp \
    qtopiaabstractservice.cpp \
    qtopiachannel.cpp \
    qtopiaipcadaptor.cpp \
    qtopiaipcenvelope.cpp \
    qtopiaipcmarshal.cpp \
    qtopialog.cpp \
    qtopianamespace.cpp\
    qtopiaservices.cpp \
    qtopiasxe.cpp \
    qtopiatimer.cpp \
    qtranslatablesettings.cpp \
    quniqueid.cpp \
    # Valuespace code
    applayer.cpp \
    inilayer.cpp \
    qexpressionevaluator.cpp \
    qfilemonitor.cpp \
    qfixedpointnumber.cpp \
    qmallocpool.cpp \
    qpacketprotocol.cpp \
    qsystemlock.cpp \
    qvaluespace.cpp

enable_dbus_ipc {
    SOURCES += qabstractipcinterface_dbus.cpp
} else {
    SOURCES += qabstractipcinterface.cpp
}

!x11 {
    PRIVATE_HEADERS+=\
        qsharedmemorycache_p.h

    SOURCES+=\
        qsharedmemorycache.cpp
}

x11 {
    HEADERS+=\
        qcopchannel_x11.h

    PRIVATE_HEADERS+=\
        qcopchannel_x11_p.h \
        qunixsocket_p.h \
        qunixsocketserver_p.h

    SOURCES+=\
        qcopchannel_x11.cpp \
        qunixsocket.cpp \
        qunixsocketserver.cpp \
        qglobalpixmapcache_x11.cpp

    enable_sxe {
        HEADERS+=\
            qtransportauth_qws.h\
            qtransportauthdefs_qws.h

        PRIVATE_HEADERS+=\
            qtransportauth_qws_p.h
        MOC_COMPILE_EXCEPTIONS+=\
            qtransportauth_qws_p.h

        SOURCES+=\
            qtransportauth_qws.cpp
    }
}

defbtn.files=$$device_overrides(/etc/defaultbuttons.conf)
defbtn.path=/etc
INSTALLS+=defbtn

!qbuild{
# We need to prevent some files from appearing in TRANSLATABLES
TRANSLATABLES*=$$FORMS $$HEADERS $$SOURCES
CONFIG+=no_auto_translatables

QTOPIA_CUSTOM=$$QPEDIR/src/libraries/qtopiabase/custom-qtopia
HEADERS+=$${QTOPIA_CUSTOM}.h
SOURCES+=$${QTOPIA_CUSTOM}.cpp

headers.files=$$HEADERS
headers.path=/include/qtopiabase
headers.hint=sdk headers
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopiabase/private
pheaders.hint=sdk headers
INSTALLS+=pheaders

sdk_qtopiabase_custom_headers.files=custom-*-*.h
sdk_qtopiabase_custom_headers.path=/src/libraries/qtopiabase
sdk_qtopiabase_custom_headers.hint=sdk
INSTALLS+=sdk_qtopiabase_custom_headers

enable_dbus_ipc:depends(3rdparty/libraries/qtdbus)

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
