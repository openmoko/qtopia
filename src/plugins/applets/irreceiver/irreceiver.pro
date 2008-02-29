qtopia_project(qtopia plugin)
TARGET		= irreceiverapplet

HEADERS		= irreceiver.h irreceiverappletimpl.h ircontroller.h
SOURCES		= irreceiver.cpp irreceiverappletimpl.cpp ircontroller.cpp

depends(libraries/qtopiacomm/ir)
depends(libraries/qtopiacomm)

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/irreceiverapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

