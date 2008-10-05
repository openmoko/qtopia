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

startup.files=startup/qpe.sh \
              startup/qpe.env
startup.path=/
startup.hint=script
INSTALLS+=startup

script.files=scripts/*
script.path=/bin
script.hint=script
INSTALLS+=script

f_dir.files=.directory
f_dir.path=/apps/Devtools
f_dir.trtarget=Devtools
f_dir.hint=desktop nct prep_db
INSTALLS+=f_dir
qbuild:MODULES*=qtopia::prep_db

desktop.files+=desktop/bt-poweron.desktop
desktop.files+=desktop/bt-poweroff.desktop
desktop.files+=desktop/get-ssh-key.desktop
desktop.files+=desktop/fast-charge.desktop

desktop.path=/apps/Devtools
desktop.depends+=install_docapi_f_dir
desktop.hint=desktop
INSTALLS+=desktop

pics.files=*.png\
           *.svg
pics.path=/pics/devtools
pics.hint=pics
INSTALLS+=pics

startup.files=$$DEVICE_CONFIG_PATH/src/devtools/startup/qpe.sh \
              $$DEVICE_CONFIG_PATH/src/devtools/startup/qpe.env

startup.path=/
startup.hint=script
INSTALLS+=startup


help.source=$$DEVICE_CONFIG_PATH/help
help.files=qpe-devtools*
help.hint=help
INSTALLS+=help

