!qbuild{

requires(enable_cell)

qtopia_project(server_lib)
TARGET=qpe_phone_samples_e1

depends(server/core_server)
depends(server/phone/telephony/dialercontrol)
depends(server/phone/telephony/dialproxy)
depends(server/phone/themecontrol)
depends(server/phone/ui/browserstack)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(e1.pri)

samplespics.files=$$QTOPIA_DEPOT_PATH/pics/samples/*
samplespics.path=/pics/samples
samplespics.hint=pics
INSTALLS+=samplespics

samplesprofilepics.files=$$QTOPIA_DEPOT_PATH/pics/profiles/*
samplesprofilepics.path=/pics/profiles
samplesprofilepics.hint=pics
INSTALLS+=samplesprofilepics
