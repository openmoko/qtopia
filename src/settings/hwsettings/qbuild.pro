TEMPLATE=app
CONFIG+=qtopia quicklaunch singleexec
TARGET=hwsettings
MODULES*=handwriting

FORMS += gprefbase.ui charseteditbase.ui
HEADERS += pensettingswidget.h mainwindow.h charsetedit.h uniselect.h
SOURCES += pensettingswidget.cpp main.cpp mainwindow.cpp charsetedit.cpp uniselect.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/hwsettings.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/hwsettings/*
pics.path=/pics/hwsettings
pics.hint=pics
INSTALLS+=pics
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=hwsettings*.html
help.hint=help
INSTALLS+=help

EXTRA_TS_FILES=QtopiaHandwriting

pkg.description=Settings for handwriting recognition
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

