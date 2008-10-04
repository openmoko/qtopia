qtopia_project(unittest)
TARGET=tst_$$TARGET
include(../common.pri)
MAIN_SOURCES=$$PWD/main.cpp.in
COMMON_TESTS=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/plugins/outlook/tests/datebook/common_tests.cpp
include($$QTOPIA_DEPOT_PATH/src/qtopiadesktop/plugins/outlook/tests/main.pri)
