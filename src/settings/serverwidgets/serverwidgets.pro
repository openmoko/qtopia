qtopia_project(qtopia app)
TARGET=serverwidgets
CONFIG+=qtopia_main

FORMS           = mainwindow.ui
HEADERS		= serverwidgets.h
SOURCES		= serverwidgets.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=serverwidgets*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/ServerWidgets.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/serverwidgets/*
pics.path=/pics/serverwidgets
pics.hint=pics
INSTALLS+=help desktop pics

pkg.desc=Changes Qtopia server UI.
pkg.domain=window
