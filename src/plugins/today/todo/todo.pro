qtopia_project(qtopia plugin)
TARGET=todoplugin

FORMS = todooptionsbase.ui
HEADERS = todoplugin.h todopluginoptions.h
SOURCES = todoplugin.cpp todopluginoptions.cpp

depends(libraries/qtopiapim)

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/today/todoplugin.desktop
desktop.path=/plugins/today/
desktop.hint=desktop
INSTALLS+=desktop

pkg.name=qpe-today-todoplugin
pkg.domain=libs
