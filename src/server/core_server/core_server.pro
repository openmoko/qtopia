!qbuild{
qtopia_project(server_lib)
TARGET=qpe_core_server

x11:idep(LIBS+=-lXtst)

depends(libraries/qtopia)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))

!x11:qtuitest:DEFINES+=QTOPIA_USE_TEST_SLAVE
}

include(core_server.pri)

# install services
launcherservice.files=$$QTOPIA_DEPOT_PATH/services/Launcher/qpe
launcherservice.path=/services/Launcher
INSTALLS+=launcherservice

qtopiapowermanager.files=$$QTOPIA_DEPOT_PATH/services/QtopiaPowerManager/qpe
qtopiapowermanager.path=/services/QtopiaPowerManager
INSTALLS+=qtopiapowermanager

suspendservice.files=$$QTOPIA_DEPOT_PATH/services/Suspend/qpe
suspendservice.path=/services/Suspend
INSTALLS+=suspendservice

timeupdateservice.files=$$QTOPIA_DEPOT_PATH/services/TimeUpdate/qpe
timeupdateservice.path=/services/TimeUpdate
INSTALLS+=timeupdateservice

virtualkeyboardservice.files=$$QTOPIA_DEPOT_PATH/services/VirtualKeyboard/qpe
virtualkeyboardservice.path=/services/VirtualKeyboard
INSTALLS+=virtualkeyboardservice

tasks.files=$$device_overrides(/etc/Tasks.cfg)
tasks.path=/etc
INSTALLS+=tasks

# install various conf files
settings.files=\
    $$device_overrides(/etc/default/Trolltech/locale.conf)\
    $$device_overrides(/etc/default/Trolltech/Security.conf)\
    $$device_overrides(/etc/default/Trolltech/IniValueSpace.conf)\
    $$device_overrides(/etc/default/Trolltech/Hardware.conf)\
    $$device_overrides(/etc/default/Trolltech/qpe.conf)\
    $$device_overrides(/etc/default/Trolltech/UIFactory.conf)\
    $$device_overrides(/etc/default/Trolltech/ServerWidgets.conf)\
    $$device_overrides(/etc/default/Trolltech/Launcher.conf)
settings.path=/etc/default/Trolltech
INSTALLS+=settings

