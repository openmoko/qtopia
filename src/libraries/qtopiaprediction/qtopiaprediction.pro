qtopia_project(qtopia core lib)
TARGET=qtopiaprediction

VPATH+=$$QTOPIA_DEPOT_PATH/src/plugins/inputmethods/predictivekeyboard/
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/plugins/inputmethods/predictivekeyboard/
DEPENDPATH+=$$QTOPIA_DEPOT_PATH/src/plugins/inputmethods/predictivekeyboard/

HEADERS += \
    wordprediction.h \
    pred.h

SOURCES += \
    wordprediction.cpp \
    pred.cpp

VERSION=4.3.1

