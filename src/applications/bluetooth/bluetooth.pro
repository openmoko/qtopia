qtopia_project(qtopia app)
TARGET=bluetooth
CONFIG+=qtopia_main

HEADERS		= dirdeleterdialog.h \
                  mainwindow.h
SOURCES		= mainwindow.cpp \
                  dirdeleterdialog.cpp \
                  main.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/bluetooth.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# TODO Bluetooth can't install help when there are no help files to install
# Either this code should be removed or the missing help files should be added
#help.source=$$QTOPIA_DEPOT_PATH/help
#help.files=bluetooth*
#help.hint=help
#INSTALLS+=help

depends(libraries/qtopiacomm)
depends(libraries/qtopiabase)

pkg.desc=A Bluetooth Applications Suite.
pkg.domain=window,bluetooth,doc_server,doc_write
