CONFIG		+= qtopiaapp

HEADERS		= appearance.h themeparser.h fontmap.h
SOURCES		= appearance.cpp themeparser.cpp main.cpp fontmap.cpp
QTOPIA_PHONE {
    INTERFACES	= appearancesettingsbasephone.ui
} else {
    INTERFACES	= appearancesettingsbase.ui
}

TRANSLATABLES = $${HEADERS} $${SOURCES} appearancesettingsbase.ui \
                    appearancesettingsbasephone.ui
    


!QTOPIA_PHONE {
    HEADERS += samplewindow.h
    SOURCES += samplewindow.cpp
}

TRANSLATABLES +=samplewindow.h \
                samplewindow.cpp 

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Appearance.desktop
desktop.path=/apps/Settings
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=appearance*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/appearance/*
pics.path=/pics/appearance
INSTALLS	+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

TARGET		= appearance

