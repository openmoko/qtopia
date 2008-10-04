!qbuild{
qtopia_project(desktop lib)
TARGET=qtuitestrunner

CONFIG += no_tr qtestlib link_test

# We need qtopiabase includes
qt_inc(qtopiabase)
depends(libraries/qtopiabase,fake)
qt_inc(qtopia)
depends(libraries/qtopia,fake)

VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
idep(INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest,INCLUDEPATH)
VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopia
idep(INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopia,INCLUDEPATH)
}

include(../lib_qt/lib_qt.pro)

SEMI_PRIVATE_HEADERS += \
        qtopiasystemtest_p.h

HEADERS +=\
        qtopiasystemtest.h \
        qtopiasystemtestmodem.h

SOURCES +=\
        qtopiasystemtest.cpp \
        qtopiasystemtest_p.cpp

contains(PROJECTS,tools/phonesim) {
    qbuild {
        MODULES*=phonesim
        dep.headers.TYPE=DEPENDS PERSISTED METOO
        dep.headers.EVAL="DEFINES+=QTUITEST_USE_PHONESIM"
    } else {
        depends(tools/phonesim/lib)
        idep(DEFINES+=QTUITEST_USE_PHONESIM,DEFINES)
    }
    SOURCES += qtopiasystemtestmodem.cpp
    RESOURCES += qtopiasystemtestmodem.qrc
}



!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtuitestrunner
headers.hint=headers
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtuitestrunner/private
pheaders.hint=private headers
INSTALLS+=pheaders

idep(LIBS*=-l$$TARGET)
idep(CONFIG*=qtestlib)
qt_inc($$TARGET)
}

