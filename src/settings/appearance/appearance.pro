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

defaults.files=$${QTOPIA_DEPOT_PATH}/etc/default/FontMap.conf
defaults.path=/etc/default
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Appearance.desktop
desktop.path=/apps/Settings
help.files=$${QTOPIA_DEPOT_PATH}/help/html/appearance*
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/appearance/*
pics.path=/pics/appearance
INSTALLS	+= desktop help defaults
PICS_INSTALLS+=pics

TARGET		= appearance

