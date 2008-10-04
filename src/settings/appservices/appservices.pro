!qbuild {
qtopia_project(qtopia app)
TARGET=appservices
CONFIG+=qtopia_main
}

HEADERS		= appservices.h applist.h appdetails.h mimetypes.h itemfactory.h
SOURCES		= appservices.cpp applist.cpp appdetails.cpp mimetypes.cpp itemfactory.cpp main.cpp

pics.files=$$QTOPIA_DEPOT_PATH/pics/appservices/*
pics.path=/pics/appservices
pics.hint=pics
INSTALLS+=pics

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/appservices.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=$$QTOPIA_DEPOT_PATH/help/html/appservices*.html
help.hint=help
INSTALLS+=help

pkg.desc=Allows you to choose which application provides which service.
pkg.domain=trusted
