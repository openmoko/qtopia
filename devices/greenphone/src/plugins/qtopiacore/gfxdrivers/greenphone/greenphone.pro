!qbuild{
qtopia_project(embedded qtopia core plugin)
TARGET = greenphonescreen
CONFIG += no_tr
LIBS += -lblend
}

HEADERS += greenphonescreendriverplugin.h greenphonescreen.h
SOURCES += greenphonescreendriverplugin.cpp greenphonescreen.cpp

