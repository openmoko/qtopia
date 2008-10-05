!enable_singleexec {
    qbuild:MKSPEC.LFLAGS+=-rdynamic
    else:QMAKE_LFLAGS+=-rdynamic
}

SOURCES+=\
    main.cpp

equals(QTOPIA_UI,home) {
    deskpics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/deskphone/*
    deskpics.path=/pics/qpe/deskphone
    deskpics.hint=pics
    INSTALLS+=deskpics
}

bins.files=\
    $$QTOPIA_DEPOT_PATH/bin/targzip\
    $$QTOPIA_DEPOT_PATH/bin/targunzip
bins.path=/bin
bins.hint=script
INSTALLS+=bins

appspics.files=$$QTOPIA_DEPOT_PATH/pics/*.* $$QTOPIA_DEPOT_PATH/pics/icons
appspics.path=/pics
appspics.hint=pics
INSTALLS+=appspics

pics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/*.*
pics.path=/pics/qpe
pics.hint=pics
INSTALLS+=pics

speeddialsounds.files=$$QTOPIA_DEPOT_PATH/sounds/speeddial
speeddialsounds.path=/sounds
INSTALLS+=speeddialsounds

!qbuild{
servicedefs.files=$$files($$QTOPIA_DEPOT_PATH/services/*.service)
} else {
    servicedefs.files=$$QTOPIA_DEPOT_PATH/services/*.service
}
servicedefs.path=/services
INSTALLS+=servicedefs

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=\
    index.html\
    qpe*\
    help-*\
    help.html\
    document*\
    device*\
    appservices.html\
    callhistory.html\
    calibrat*\
    dialer*\
    valuespace.html
help.hint=help
INSTALLS+=help

phonepics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/phone/*
phonepics.path=/pics/qpe/phone
phonepics.hint=pics
INSTALLS+=phonepics

globalphonepics.files=$$QTOPIA_DEPOT_PATH/pics/phone/*
globalphonepics.path=/pics/phone
globalphonepics.hint=pics
INSTALLS+=globalphonepics


defaultalerts.files=$$QTOPIA_DEPOT_PATH/etc/SystemRingTones/*.wav
defaultalerts.path=/etc/SystemRingTones
defaultalerts.hint=content nct
defaultalerts.categories=SystemRingtones
defaultalerts.trtarget=QtopiaRingTones
INSTALLS+=defaultalerts


ANIMFILE=$$QTOPIA_DEPOT_PATH/pics/qpe/splash/$${QTOPIA_DISP_WIDTH}x$${QTOPIA_DISP_HEIGHT}/splash.gif
exists($$ANIMFILE) {
    splash.files=$$ANIMFILE
} else {
    # Fall-back un-animated
    splash.files=$$QTOPIA_DEPOT_PATH/pics/qpe/splash/splash.png
}
splash.path=/pics/qpe
INSTALLS+=splash

enable_sxe {
    qbuild {
        security.commands=\
            "mkdir -p $$QTOPIA_IMAGE/etc/rekey $$QTOPIA_IMAGE/etc/sxe_qtopia $$QTOPIA_IMAGE/etc/sxe_domains"\
            "install -m 0600 "$$path(/etc,project)"/sxe.* $$QTOPIA_IMAGE/etc"\
            "install -m 0500 "$$path(/etc,project)"/sxe_domains/* $$QTOPIA_IMAGE/etc/sxe_domains"
        SXE_SCRIPTS=sxe_qtopia sxe_sandbox sxe_unsandbox sxe_reloadconf
        for(file,SXE_SCRIPTS) {
            security.commands+=\
                "install -m 0500 "$$device_overrides(/etc/sxe_qtopia/$$file)" $$QTOPIA_IMAGE/etc/sxe_qtopia"
        }
        # This is documented in src/build/doc/src/deviceprofiles.qdoc
        !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/sxe_domains) {
            security.commands+=\
                "install -m 0500 $$DEVICE_CONFIG_PATH/etc/sxe_domains/* $$QTOPIA_IMAGE/etc/sxe_domains"
        }
        security.hint=image
    } else {
        security.path=/etc
        security.commands=$$COMMAND_HEADER\
            install -c $$QTOPIA_DEPOT_PATH/etc/sxe.* $(INSTALL_ROOT)/etc $$LINE_SEP\
            chmod 0600 $(INSTALL_ROOT)/etc/sxe.* $$LINE_SEP\
            mkdir -p $(INSTALL_ROOT)/etc/rekey $$LINE_SEP\
            mkdir -p $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP\
            mkdir -p $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP

        SXE_SCRIPTS=sxe_qtopia sxe_sandbox sxe_unsandbox sxe_reloadconf
        for(file,SXE_SCRIPTS) {
            security.commands+=\
                install -m 0500 -c $$device_overrides(/etc/sxe_qtopia/$$file) $(INSTALL_ROOT)/etc/sxe_qtopia $$LINE_SEP
        }

        security.commands+=\
            install -m 0500 -c $$QTOPIA_DEPOT_PATH/etc/sxe_domains/* $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP

        # This is documented in src/build/doc/src/deviceprofiles.qdoc
        !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/sxe_domains) {
            security.commands+=\
                install -m 0500 -c $$DEVICE_CONFIG_PATH/etc/sxe_domains/* $(INSTALL_ROOT)/etc/sxe_domains $$LINE_SEP
        }
        INSTALLS+=security
    }

    security_settings.files=$$device_overrides(/etc/default/Trolltech/Sxe.conf)
    security_settings.path=/etc/default/Trolltech
    INSTALLS+=security_settings
}

EXTRA_TS_FILES=\
    QtopiaApplications QtopiaGames QtopiaSettings QtopiaI18N QtopiaServices\
    QtopiaNetworkServices QtopiaBeaming QtopiaColorSchemes QtopiaDefaults\
    QtopiaRingTones QtopiaThemes Categories-Qtopia

pkg.name=qpe-taskbar
pkg.desc=Launcher for QPE
pkg.domain=qpe

