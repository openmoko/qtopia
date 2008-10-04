!qbuild{
    qtopia_project(qtopia plugin)
    CONFIG += no_tr qtestlib
    plugin_type = qtuitest_application
    TARGET=qtuitest_appslave
}

HEADERS += \
        qtopiasystemtestslave.h

SOURCES += \
        qtopiasystemtestslave.cpp

include(../slave_qt/slave_qt.pro)

!qbuild{
    headers.files=$$HEADERS
    headers.hint=headers sdk
    headers.path=/include/qtuitest_appslave
    INSTALLS+=headers

    depends(libraries/qtuitest)
}

