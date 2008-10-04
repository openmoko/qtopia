!qbuild{
qtopia_project(desktop app)
TARGET=sxe_installer
CONFIG+=no_install no_tr

VPATH+=$$QTE_DEPOT_PATH/src/gui/embedded
INCLUDEPATH+=$$QTE_DEPOT_PATH/src/gui/embedded

### Also needs functionality from the package manager
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity

### Also needs qLog
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase

qt_inc(qtopiasecurity)
depends(libraries/qtopiasecurity,fake)
qt_inc(qtopia)
depends(libraries/qtopia,fake)
qt_inc(qtopiabase)
depends(libraries/qtopiabase,fake)
}

DEFINES+=SXE_INSTALLER

SOURCES	= main.cpp

SOURCES	+= keyfiler.cpp qpackageregistry.cpp qsxepolicy.cpp qtopialog.cpp qlog.cpp qtopiasxe.cpp qtopianamespace.cpp
HEADERS += qtopiasxe.h keyfiler_p.h qpackageregistry.h qsxepolicy.h qtopialog.h qlog.h qtopianamespace.h
