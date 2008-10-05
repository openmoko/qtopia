!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_browserscreen_deskphone

depends(server/core_server)
depends(server/phone/browserscreen/abstract)
depends(server/phone/homescreen/abstract)
depends(server/phone/themecontrol)
depends(server/phone/ui/browserstack)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(deskphone.pri)
