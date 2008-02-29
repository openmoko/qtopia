qtopia_project(qtopia app)
TARGET=appearance
CONFIG+=qtopia_main no_quicklaunch

phone {
    FORMS	= appearancesettingsbasephone.ui
} else {
    FORMS	= appearancesettingsbase.ui
}
TRANSLATABLES += appearancesettingsbase.ui \
                    appearancesettingsbasephone.ui

HEADERS		= appearance.h itemcontrol.h themeparser.h
SOURCES		= appearance.cpp itemcontrol.cpp themeparser.cpp main.cpp

    
!phone {
    HEADERS += samplewindow.h
    SOURCES += samplewindow.cpp
}
TRANSLATABLES +=samplewindow.h \
                samplewindow.cpp 

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Appearance.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=appearance*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/appearance/*
pics.path=/pics/appearance
pics.hint=pics
captureService.files=$$QTOPIA_DEPOT_PATH/services/Settings/appearance
captureService.path=/services/Settings
INSTALLS+=desktop help pics captureService

pkg.desc=Appearance settings for Qtopia.
pkg.domain=window,docapi,profiles,cardreader,drm
