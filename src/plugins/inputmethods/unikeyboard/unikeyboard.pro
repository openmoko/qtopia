qtopia_project(qtopia plugin)
TARGET=qunikeyboard

HEADERS		= unikeyboard.h unikeyboardimpl.h
SOURCES		= unikeyboard.cpp unikeyboardimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/inputmethods/qunikeyboard.desktop
desktop.path=/plugins/inputmethods/
desktop.hint=desktop
INSTALLS+=desktop

pkg.name=qpe-unikeyboard
pkg.domain=libs
