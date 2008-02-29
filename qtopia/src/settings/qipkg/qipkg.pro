CONFIG		+= qtopiaapp

HEADERS		= packagewizard.h
SOURCES		= packagewizard.cpp main.cpp
INTERFACES	= pkwizard.ui pkdesc.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= qipkg


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=qipkg*
info.files=$${QTOPIA_DEPOT_PATH}/bin/qipkg-info-file
info.path=/bin
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/qipkg.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/qipkg/*
pics.path=/pics/qipkg
INSTALLS += desktop info
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=A GUI front-end to ipkg for the Qtopia environment.
