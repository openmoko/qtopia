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

flash.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/*
flash.path=/../

startup.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startup/qpe.sh \
              $$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startup/qpe.env
startup.path=/
startup.hint=script

testenv.files_=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startup/qpe_test.env
testenv.path=/
testenv.commands=-$(INSTALL_FILE) $$testenv.files_ $(INSTALL_ROOT)/qpe.env

script.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/scripts/*
script.path=/bin
script.hint=script

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/desktop/*.desktop 
desktop.path=/apps/Devtools
desktop.hint=desktop

pics.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/*.png\
           $$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/*.svg
pics.path=/pics/devtools
pics.hint=pics

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=devtools*
help.hint=help

motd.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/motd.html
motd.path=/etc

conf.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/etc/*
conf.path=/etc

# files required to flash Greenphone via miniSD or USB
greenphone_flash.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/flash-status-*.gif
greenphone_flash.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/trolltech_sdk_*.sh
greenphone_flash.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/tools.tgz
greenphone_flash.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/root.tgz
greenphone_flash.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/user_default.tgz
greenphone_flash.files+=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/flash-files/pkg_1.0_all.ipk
greenphone_flash.path=/../greenphone_flash
greenphone_flash.CONFIG+=no_default_install

greenphone_flash_post.depends=install_greenphone_flash
greenphone_flash_post.commands=(mv -f "\$(INSTALL_ROOT)$${greenphone_flash.path}/trolltech_sdk_flash.sh" "\$(INSTALL_ROOT)$${greenphone_flash.path}/trolltech_flash.sh"); \
                               (cd "\$(INSTALL_ROOT)$${greenphone_flash.path}" ; tar -xzf tools.tgz busyboxT gifanim ; mv -f busyboxT tar)
greenphone_flash_post.CONFIG+=no_path

INSTALLS+=desktop pics script startup motd conf help
#INSTALLS+=greenphone_flash greenphone_flash_post
QMAKE_EXTRA_TARGETS+=testenv
