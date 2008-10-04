qtopia_project(qtopia plugin)
TARGET=coloreffect
plugin_type = photoediteffects

INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/applications/photoedit

HEADERS     =  coloreffect.h
SOURCES     =  coloreffect.cpp

depends(applications/photoedit)

effect_settings.files=coloreffect.conf
effect_settings.path=/etc/photoedit
effect_settings.hint=nct
INSTALLS += effect_settings
