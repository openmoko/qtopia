qtopia_project(qtopia app)
TARGET=light-and-power
CONFIG+=qtopia_main no_quicklaunch

FORMS	= lightsettingsbase.ui 
PHONE_FORMS = displaysettingsbase.ui
phone{
    FORMS += $${PHONE_FORMS}
}
TRANSLATABLES = $${PHONE_FORMS}
HEADERS		= light.h
SOURCES		= light.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=light-and-power*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Light.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/light-and-power/*
pics.path=/pics/light-and-power
pics.hint=pics
captureService.files=$$QTOPIA_DEPOT_PATH/services/Settings/light-and-power
captureService.path=/services/Settings
INSTALLS+=help desktop pics captureService

pkg.desc=Light and Power settings dialog for Qtopia.
pkg.domain=window,lightandpower,profiles,screensaver,screensaverconfig
