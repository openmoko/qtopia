qtopia_project(qtopia app)
TARGET=btsettings
CONFIG+=qtopia_main

FORMS = btsettings.ui \
        device.ui \
        deviceinfo.ui \
        localservices.ui \
        localservicesettings.ui \
        visibility.ui \
        myheadset.ui

HEADERS		= mainwindow.h \
              localserviceswindow.h \
              remotedevicesdialog.h \
              headset.h

SOURCES		= mainwindow.cpp \
              localserviceswindow.cpp \
              remotedevicesdialog.cpp \
              headset.cpp \
              main.cpp 

depends(libraries/qtopiacomm)

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/btsettings.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=btsettings*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/btsettings/*
pics.path=/pics/btsettings
pics.hint=pics
INSTALLS+=desktop help pics

pkg.desc=Bluetooth Settings
pkg.domain=window,bluetooth,cardreader,pim,phonecomm
