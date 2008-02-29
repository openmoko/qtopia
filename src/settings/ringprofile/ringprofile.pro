qtopia_project(qtopia app)
TARGET=profileedit
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= ringprofile.h ringtoneeditor.h 
SOURCES		= main.cpp ringprofile.cpp ringtoneeditor.cpp 

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=profileedit*
help.hint=help
desktop.files	=$$QTOPIA_DEPOT_PATH/apps/Settings/RingProfiles.desktop
desktop.path	=/apps/Settings
desktop.hint=desktop
pics.files	=$$QTOPIA_DEPOT_PATH/pics/profileedit/*
pics.path	=/pics/profileedit
pics.hint=pics
service.files   =$$QTOPIA_DEPOT_PATH/services/Profiles/profileedit
service.path	=/services/Profiles
serviceb.files   =$$QTOPIA_DEPOT_PATH/services/SettingsManager/profileedit
serviceb.path	=/services/SettingsManager
INSTALLS+=help desktop pics service serviceb

pkg.desc=Phone profile settings for Qtopia.
pkg.domain=window,alarm,profiles,docapi,cardreader,mediasession,drm
