!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

VPATH+=$$PWD
INCLUDEPATH+=$$PWD

HEADERS += \
        qtestslave.h \
        qtestslaveglobal.h \
        qtestwidgets.h

SOURCES += \
        qtestslave.cpp \
        qtestwidgets.cpp

!qbuild:!standalone {
    INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
}

standalone {
    TEMPLATE=lib
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    INCLUDEPATH+=$$(QTOPIA_DEPOT_PATH)/src/libraries/qtuitest
    TARGET=qtslave
}

