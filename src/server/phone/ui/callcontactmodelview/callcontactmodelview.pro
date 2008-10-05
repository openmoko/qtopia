!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_ui_callcontactmodelview

requires(enable_telephony)

depends(server/core_server)
depends(server/pim/savetocontacts)
depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)
depends(3rdparty/libraries/inputmatch)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(callcontactmodelview.pri)
