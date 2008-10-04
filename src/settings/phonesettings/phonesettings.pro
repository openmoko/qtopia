!qbuild {
qtopia_project(qtopia app)
TARGET=phonesettings
CONFIG+=qtopia_main
depends(libraries/qtopiapim)
depends(libraries/qtopiaphone)
}

FORMS		= channeledit.ui
HEADERS		= phonesettings.h
SOURCES		= phonesettings.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=phonesettings*
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/phonesettings.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/phonesettings/*
pics.path=/pics/phonesettings
pics.hint=pics
INSTALLS+=pics

service.files=$$QTOPIA_DEPOT_PATH/services/VoiceMail/phonesettings
service.path=/services/VoiceMail
INSTALLS+=service

pkg.desc=Telephony settings dialog for Qtopia.
pkg.domain=trusted
