!qbuild{
qtopia_project(qtopia plugin)
TARGET=qpredictivekeyboard
CONFIG+=no_singleexec no_tr
depends(libraries/qtopiatheming)
}

HEADERS		= predictivekeyboard.h \
		  predictivekeyboardimpl.h\
                  keyboard.h

SOURCES		= predictivekeyboard.cpp \
		  predictivekeyboardimpl.cpp\
                  keyboard.cpp

pkg.name=qpe-predictivekeyboard
pkg.domain=trusted

# FIXME this file does not exist!
#dict.files=
#dict.files=words.qtrie
#dict.path=/etc
#INSTALLS += dict

pics.files=$$QTOPIA_DEPOT_PATH/pics/predictivekeyboard/*
pics.path=/pics/predictivekeyboard
pics.hint=pics
INSTALLS+=pics

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=im-predictive-keyboard.html
help.hint=help
INSTALLS+=help

settings.files=$$device_overrides(/etc/default/Trolltech/PredictiveKeyboard.conf) \
               $$device_overrides(/etc/default/Trolltech/PredictiveKeyboardLayout.conf)
!isEmpty(settings.files) {
    settings.path=/etc/default/Trolltech
    INSTALLS+=settings
}

