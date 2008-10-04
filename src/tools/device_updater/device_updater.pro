!qbuild{
qtopia_project(desktop app)
CONFIG+=no_install no_singleexec
}

FORMS*=deviceupdaterbase.ui configurebase.ui

HEADERS+= \
    deviceupdater.h \
    localsocket.h \
    localsocketlistener.h \
    packagescanner.h \
    inetdadaptor.h \
    deviceconnector.h \
    scannerthread.h \
    configure.h \
    configuredata.h \
    deviceupdaterdialog.h

SOURCES+= \
    main.cpp \
    deviceupdater.cpp \
    localsocket.cpp \
    localsocketlistener.cpp \
    packagescanner.cpp \
    inetdadaptor.cpp \
    deviceconnector.cpp \
    scannerthread.cpp \
    configure.cpp \
    configuredata.cpp \
    deviceupdaterdialog.cpp

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
