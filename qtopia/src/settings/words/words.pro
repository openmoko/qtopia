CONFIG		+= qtopiaapp

HEADERS		= words.h
SOURCES		= words.cpp main.cpp
INTERFACES	=

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

#QTOPIA_PHONE:!buildSingleExec {
!buildSingleExec {
    HEADERS+=../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.h
    SOURCES+=../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.cpp
}
buildSingleExec:!contains(PLUGIN_PROJECTS, 3rdparty/plugins/inputmethods/pkim) {
    HEADERS+=../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.h
    SOURCES+=../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.cpp
}

#TRANSLATABLES += ../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.cpp \
#    ../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.h

TARGET		= words

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=words*

pics.files=$${QTOPIA_DEPOT_PATH}/pics/words/*
pics.path=/pics/words
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/words.desktop
desktop.path=/apps/Settings
INSTALLS	+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Words settings dialog for the Qtopia environment.
