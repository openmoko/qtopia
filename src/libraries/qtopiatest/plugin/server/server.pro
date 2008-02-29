qtopia_project(qtopia plugin)

DEFINES += QTOPIA_TEST_TARGET
DEFINES += SERVER_PLUGIN

SOURCES = \
        ../testslaveinterfaceimpl.cpp

TARGET = qtopiaservertestslave

# FIXME there's no reason why this shouldn't have translations.
CONFIG += no_tr syncqtopia
# We access symbols in the server, so link_test will always fail
CONFIG += no_link_test

!qtopiatest: CONFIG += no_singleexec

plugin_type=qtopiatest

depends(libraries/qtopiatest/target)


# for singleexec, this code will be compiled into the app plugin
!enable_singleexec {
    include(../../qtestslave/qtestslave.pri)
} else {
    INCLUDEPATH += ../../qtestslave
}
include(../../qtopiaservertestslave/qtopiaservertestslave.pri)

qt_inc($$TARGET)

