qtopia_project(qtopia app)
TARGET=speeddial
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= speeddialedit.h
SOURCES		= speeddialedit.cpp main.cpp

TRANSLATABLES += speeddialedit.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/SpeedDial.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=speeddial*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/speeddial/*
pics.path=/pics/speeddial
pics.hint=pics
sounds.files=$$QTOPIA_DEPOT_PATH/sounds/speeddial/*
sounds.path=/sounds/speeddial
INSTALLS+=desktop help pics sounds

pkg.desc=Speed-dial settings dialog for Qtopia.
pkg.domain=window,docapi
