qtopia_project(qtopia app)
TARGET=printserver
CONFIG+=singleexec_main

HEADERS+= printserver.h

SOURCES+= main.cpp \
          printserver.cpp

service.files=$$QTOPIA_DEPOT_PATH/services/Print/printserver
service.path=/services/Print
INSTALLS+=service

depends(libraries/qtopiaprinting)

pkg.desc=Print server for mobile printing
pkg.domain=window,print,phonecomm,cardreader
