!qbuild{
qtopia_project(stub)

defineReplace(maketarget) {
    target=$$1
    target~=s,/,_,g
    target~=s,\.\._,,g
    return($$target)
}

# Depend on everything so we can guarantee that this directory is processed last
for(p,PROJECTS) {
    depends($$p,fake)
}
QMAKE_STRIP=
}

flash.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/*
flash.path=/../

startup.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startup/qpe.sh \
              $$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startup/qpe.env
startup.path=/
startup.hint=script
INSTALLS+=startup

qbuild {
# QBuild doesn't have a $$files function (though it probably should)
script.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/scripts/*
} else {
script.files=$$files($$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/scripts/*)
}
script.path=/bin
script.hint=script
INSTALLS+=script

f_dir.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/.directory
f_dir.path=/apps/Devtools
f_dir.trtarget=Devtools
f_dir.hint=desktop nct prep_db
MODULES*=qtopia::prep_db
INSTALLS+=f_dir

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/network-services-start.desktop
desktop.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/network-services-stop.desktop
desktop.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/sdcard-umount.desktop
desktop.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/sdio-storage.desktop
desktop.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/sdio-wifi.desktop
desktop.path=/apps/Devtools
desktop.depends+=install_docapi_f_dir
desktop.hint=desktop
INSTALLS+=desktop

pics.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/*.png\
           $$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/*.svg
pics.path=/pics/devtools
pics.hint=pics
INSTALLS+=pics

help.source=$$DEVICE_CONFIG_PATH/help
help.files=qpe-devtools*
help.hint=help
INSTALLS+=help

