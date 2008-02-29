CONFIG		+= qtopiaapp

HEADERS		= packagewizard.h
SOURCES		= packagewizard.cpp main.cpp
INTERFACES	= pkwizard.ui pkdesc.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= qipkg


help.files=$${QTOPIA_DEPOT_PATH}/help/html/qipkg*
help.path=/help/html
info.files=$${QTOPIA_DEPOT_PATH}/bin/qipkg-info-file
info.path=/bin
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/qipkg.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/qipkg/*
pics.path=/pics/qipkg
INSTALLS += desktop info help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=A GUI front-end to ipkg for the Qtopia environment.
