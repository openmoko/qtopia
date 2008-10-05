!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_usbgadgetselector

depends(server/core_server)
depends(server/comm/usbgadget)
depends(libraries/qtopiabase)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(usbgadgetselector.pri)

usbgadgetservice.files=$$QTOPIA_DEPOT_PATH/services/UsbGadget/*.service
usbgadgetservice.path=/services/UsbGadget
INSTALLS+=usbgadgetservice
