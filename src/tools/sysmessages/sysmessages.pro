!qbuild{
qtopia_project(qtopia app)
TARGET=sysmessages
CONFIG+=no_quicklaunch
CONFIG+=singleexec_main
CONFIG+=no_tr
}

# Input
HEADERS += sysmessages.h 

SOURCES += main.cpp sysmessages.cpp

# SXE permissions required
pkg.domain=trusted

contains(PROJECTS,libraries/qtopiamail) {
    DEFINES+=MAIL_EXISTS
}

sysmsgservice.files=$$QTOPIA_DEPOT_PATH/services/SystemMessages/sysmessages
sysmsgservice.path=/services/SystemMessages
INSTALLS+=sysmsgservice
