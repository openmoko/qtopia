qtopia_project(qtopia plugin)
TARGET=datebookplugin

FORMS = datebookoptionsbase.ui
HEADERS = datebookplugin.h datebookpluginoptions.h
SOURCES = datebookplugin.cpp datebookpluginoptions.cpp

depends(libraries/qtopiapim)

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/today/datebookplugin.desktop
desktop.path=/plugins/today/
desktop.hint=desktop
INSTALLS+=desktop

pkg.name=qpe-today-datebookplugin
pkg.domain=libs
