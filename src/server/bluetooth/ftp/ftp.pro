!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_ftp

requires(enable_bluetooth)
requires(contains(PROJECTS,server/bluetooth/powermgr))
requires(contains(PROJECTS,server/bluetooth/servicemgr))

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(ftp.pri)

btftpservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/ftp.xml
btftpservices.path=/etc/bluetooth/sdp
INSTALLS+=btftpservices
