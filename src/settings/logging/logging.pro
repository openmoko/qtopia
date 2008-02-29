qtopia_project(qtopia app)
TARGET=logging
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= loggingedit.h loggingview.h
SOURCES		= loggingedit.cpp loggingview.cpp main.cpp

TRANSLATABLES += loggingedit.cpp loggingview.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Logging.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=logging*html
help.hint=help
pics.files=pics/*
pics.path=/pics/logging
pics.hint=pics
INSTALLS+=desktop help pics

pkg.desc=Logging settings dialog for Qtopia.
pkg.domain=window,docapi
