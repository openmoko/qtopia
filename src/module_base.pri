# Qtopia Base projects (required for every edition)
!qbuild:PROJECTS*=\
    # A placeholder for installing Qt files
    qt

PROJECTS*=\
    applications/helpbrowser \
    3rdparty/libraries/dlmalloc \
    libraries/qtopiabase \
    3rdparty/libraries/md5 \
    3rdparty/libraries/zlib \
    libraries/qtopia \
    libraries/qtopiacollective \
    3rdparty/libraries/openobex\
    libraries/qtopiacomm
!qbuild:PROJECTS*=\
    # Dummy entries (only needed for the old build system)
    libraries/qtopiacomm/vpn\
    libraries/qtopiacomm/usb\
    libraries/qtopiacomm/obex\
    libraries/qtopiacomm/bluetooth\
    libraries/qtopiacomm/obex\
    libraries/qtopiacomm/ir\
    libraries/qtopiacomm/network\
    libraries/qtopiacomm/serial

PROJECTS*=\
    3rdparty/libraries/easing \
    libraries/qtopiagfx \
    libraries/qtopiatheming \
    3rdparty/libraries/sqlite \
    tools/content_installer \
    tools/dbmigrate \
    tools/dbmigrateservice \
    tools/qcop \
    tools/qdawggen \
    plugins/qtopiacore/iconengines/qtopiaiconengine \
    plugins/qtopiacore/iconengines/qtopiasvgiconengine \
    plugins/qtopiacore/iconengines/qtopiapiciconengine \
    plugins/qtopiacore/imageformats/picture \

!qbuild:PROJECTS*=\
    server/main                                     #provides main.cpp

PROJECTS*=\
    server/core_server \                            #core/minimal server
    server/phone/serverinterface/platform \         #placeholder for proper QAbstractServerInterface
    server/ui/abstractinterfaces/slideinmessagebox \#slide in message box

equals(LAUNCH_METHOD,quicklaunch):PROJECTS*=server/processctrl/quickexe
!no_quicklaunch|enable_singleexec:PROJECTS*=tools/quicklauncher

build_qtopia_sqlite:PROJECTS*=\
    3rdparty/applications/sqlite

!qbuild:enable_dbus:PROJECTS*=\
    3rdparty/libraries/qtdbus

enable_dbusipc {
    PROJECTS*=\
    3rdparty/applications/dbus \
    tools/qtopia-dbus-launcher
}

#process control is part of base
PROJECTS*=\
    server/memory/base \                        #default memory monitor
    server/memory/monitor \                     #default memory monitor
    server/infrastructure/devicefeatures \      #QtopiaFeatures population
    server/infrastructure/inputdevice \         #input device adjustment at runtime
    server/infrastructure/stabmonitor \         #stab monitoring
    server/infrastructure/storagemonitor \      #user storage monitoring
    server/processctrl/appmonitor \             #state monitoring of UI applications
    server/processctrl/appshutdown \            #application shutdown
    server/processctrl/startup \                #application preloading support
    server/processctrl/terminationhandler \     #backend for QTerminationHandler
    server/processctrl/taskmanagerentry \       #fake taskmanager entries
    server/ui/abstractinterfaces/taskmanager \  #abstract task manager interface
    server/ui/taskmanager \                     #default task manager implementation
    server/ui/components/delayedwaitdialog \    #delayed wait dialog
    server/ui/launcherviews/appview \           #application launcher view
    server/ui/launcherviews/base \              #base launcher view
    server/ui/launcherviews/documentview \      #document launcher view
    server/ui/launcherviews/taskmanagerview \   #taskmanager launcher view
    server/ui/shutdown \                        #shutdown dialog
    server/ui/shutdownsplash \                  #splash screen shown during shutdown phase
    server/ui/dfltcrashdlg \                    #default crash dialog
    server/ui/waitindicator \                   #hour glass widget
    server/phone/ui/browserstack \              #utility for stack management of browserscreen
    server/phone/browserscreen/abstract \       #abstract browser interface
    server/phone/browserscreen/gridbrowser \    #grid based browser UI
    server/phone/homescreen/abstract \          #abstract home/idle screen interface
    server/phone/themecontrol                   #theme management for various themed server widgets

# Dynamic rotation support
build_rotate:PROJECTS*=\
    server/infrastructure/rotation\              #dynamic rotation service
    settings/rotation

