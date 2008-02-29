qtopia_project(qtopia plugin)
TARGET=emailplugin

HEADERS = emailplugin.h emailpluginoptions.h
SOURCES = emailplugin.cpp emailpluginoptions.cpp

depends(libraries/qtopiamail)

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/today/emailplugin.desktop
desktop.path=/plugins/today/
desktop.hint=desktop
INSTALLS+=desktop

pkg.name=qpe-today-emailplugin
pkg.domain=libs
