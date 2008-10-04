!qbuild{
qtopia_project(qtopia app)
TARGET=printserver
CONFIG+=singleexec_main
depends(libraries/qtopiaprinting)
}

HEADERS+= printserver.h

SOURCES+= main.cpp \
          printserver.cpp

pkg.desc=Print server for mobile printing
pkg.domain=trusted

service.files=$$QTOPIA_DEPOT_PATH/services/Print/printserver
service.path=/services/Print
INSTALLS+=service

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=printserver*
help.hint=help
INSTALLS+=help
