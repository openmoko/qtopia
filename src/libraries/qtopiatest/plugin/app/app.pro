qtopia_project(qtopia plugin)

DEFINES += QTOPIA_TEST_TARGET
DEFINES += APP_PLUGIN

SOURCES = \
        ../testslaveinterfaceimpl.cpp

TARGET = qtopiaapptestslave

# FIXME there's no reason why this shouldn't have translations.
CONFIG += no_tr syncqtopia
!qtopiatest: CONFIG += no_singleexec

plugin_type=qtopiatest

depends(libraries/qtopiatest/target)


include(../../qtestslave/qtestslave.pri)
include(../../qtopiasystemtestslave/qtopiasystemtestslave.pri)

qt_inc($$TARGET)

