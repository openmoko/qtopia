TEMPLATE=app
TARGET=profileedit

CONFIG+=qtopia quicklaunch singleexec

HEADERS		= ringprofile.h ringtoneeditor.h
SOURCES		= main.cpp ringprofile.cpp ringtoneeditor.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=profileedit*
help.hint=help
INSTALLS+=help
desktop.files	=$$QTOPIA_DEPOT_PATH/apps/Settings/profileedit.desktop
desktop.path	=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files	=$$QTOPIA_DEPOT_PATH/pics/profileedit/*
pics.path	=/pics/profileedit
pics.hint=pics
INSTALLS+=pics
service.files   =$$QTOPIA_DEPOT_PATH/services/Profiles/profileedit
service.path	=/services/Profiles
INSTALLS+=service
serviceb.files   =$$QTOPIA_DEPOT_PATH/services/SettingsManager/profileedit
serviceb.path	=/services/SettingsManager
INSTALLS+=serviceb

pkg.desc=Profile settings for Qtopia.
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

