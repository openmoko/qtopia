qtopia_project(qtopia app)
TARGET=hwsettings
CONFIG+=qtopia_main no_quicklaunch

FORMS += gprefbase.ui charseteditbase.ui
HEADERS += pensettingswidget.h mainwindow.h charsetedit.h uniselect.h
SOURCES += pensettingswidget.cpp main.cpp mainwindow.cpp charsetedit.cpp uniselect.cpp

depends(libraries/handwriting)

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Handwriting.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/hwsettings/*
pics.path=/pics/hwsettings
pics.hint=pics
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=hwsettings*.html
help.hint=help

INSTALLS+= desktop pics help
EXTRA_TS_FILES=QtopiaHandwriting

pkg.description=Settings for handwriting recognition
pkg.domain=window,handwriting
