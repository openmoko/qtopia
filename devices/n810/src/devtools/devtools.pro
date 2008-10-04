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

startup.files=$$DEVICE_CONFIG_PATH/src/devtools/startup/qpe.sh \
                $$DEVICE_CONFIG_PATH/src/devtools/startup/qpe.env

startup.path=/
startup.hint=script
INSTALLS+=startup

!equals(QTOPIA_UI,home) {
    qbuild {
    # QBuild doesn't have a $$files function (though it probably should)
    script.files=$$DEVICE_CONFIG_PATH/src/devtools/scripts/*
    } else {
    script.files=$$files($$DEVICE_CONFIG_PATH/src/devtools/scripts/*)
    }
    script.path=/bin
    script.hint=script
    INSTALLS+=script

    f_dir.files=$$DEVICE_CONFIG_PATH/src/devtools/.directory
    f_dir.path=/apps/Devtools
    f_dir.trtarget=Devtools
    f_dir.hint=desktop nct prep_db
    qbuild:MODULES*=qtopia::prep_db
    INSTALLS+=f_dir

    desktop.files+=$$DEVICE_CONFIG_PATH/src/devtools/desktop/usbnet-start.desktop
    desktop.files+=$$DEVICE_CONFIG_PATH/src/devtools/desktop/usbnet-stop.desktop
    desktop.files+=$$DEVICE_CONFIG_PATH/src/devtools/desktop/usbnet-restart.desktop
    desktop.files+=$$DEVICE_CONFIG_PATH/src/devtools/desktop/maemo.desktop
    desktop.path=/apps/Devtools
    desktop.depends+=install_docapi_f_dir
    desktop.hint=desktop
    INSTALLS+=desktop

    pics.files=$$DEVICE_CONFIG_PATH/src/devtools/*.png\
                $$DEVICE_CONFIG_PATH/src/devtools/*.svg
    pics.path=/pics/devtools
    pics.hint=pics
    INSTALLS+=pics
}
