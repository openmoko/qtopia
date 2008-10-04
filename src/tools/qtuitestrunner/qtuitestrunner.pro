!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

!qbuild:!standalone{
qtopia_project(desktop app)
CONFIG += no_tr no_install qtestlib
QT += script
depends(tools/qtuitestrunner/lib_qtopia)
}

qtuitest_use_phonesim:DEFINES += QTUITEST_USE_PHONESIM

SOURCES +=\
        main.cpp\
        qscriptsystemtest.cpp \
        qtscript_qtcore.cpp \
        qtscript_bindings.cpp \
        scriptpreprocessor.cpp

HEADERS +=\
        qscriptsystemtest.h\
        scriptpreprocessor.h\
        qtscript_qtcore.h \
        qtscript_bindings.h

RESOURCES += scripts.qrc

standalone {
    TEMPLATE=app
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    VPATH+=$$PWD
    INCLUDEPATH+=$$PWD
    QT+=script
    TARGET=qtuitestrunner
    include($$PWD/lib_qt/lib_qt.pro)
}

