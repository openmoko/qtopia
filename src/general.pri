# This file contains projects that are eligable for inclusion in the Free Edition.

# Qtopia Core files
QTE_PROJECTS=\
    libraries/qtopiacore/tools/moc\
    libraries/qtopiacore/tools/uic\
    libraries/qtopiacore/tools/rcc\
    libraries/qtopiacore/corelib\
    libraries/qtopiacore/gui\
    libraries/qtopiacore/network\
    libraries/qtopiacore/sql\
    libraries/qtopiacore/xml\
    libraries/qtopiacore/script\
    libraries/qtopiacore/svg\
    plugins/qtopiacore
contains(QTE_CONFIG,opengl):QTE_PROJECTS+=libraries/qtopiacore/opengl
PROJECTS*=$$QTE_PROJECTS

# Qtopia Platform files
PROJECTS*=\
    # A placeholder for installing Qt files
    qt\
    server\
    libraries/qtopiabase\
    libraries/qtopia\
    libraries/qtopiacomm\
    libraries/qtopiaaudio\
    tools/qdsync/common\
    tools/qdsync/app\
    tools/qdsync/base\
    tools/qdsync/pim\

# Dummy entries (finegrained dependencies, retained in case libqtopiacomm is ever split up)
PROJECTS*=\
    libraries/qtopiacomm/bluetooth\
    libraries/qtopiacomm/ir\
    libraries/qtopiacomm/network\
    libraries/qtopiacomm/obex\
    libraries/qtopiacomm/serial\
    libraries/qtopiacomm/vpn

# 3rdparty stuff
PROJECTS*=\
    3rdparty/libraries/zlib\
    3rdparty/libraries/alsa\
    3rdparty/libraries/md5\
    3rdparty/libraries/tar\
    3rdparty/libraries/crypt\
    #obex
    3rdparty/libraries/openobex\
    3rdparty/libraries/inputmatch\
    3rdparty/libraries/sqlite\
    3rdparty/applications/sqlite

!x11 {
    # input methods that are available in all editions
    PROJECTS*=\
        plugins/inputmethods/predictivekeyboard\
        plugins/inputmethods/keyboard\
        plugins/inputmethods/dockedkeyboard
    
    contains(LANGUAGES,zh_CN):PROJECTS*=plugins/inputmethods/pinyin
}

!media:PROJECTS*=\
    libraries/qtopiapim

# qtopiatest
qtopiatest {
    PROJECTS*=\
        libraries/qtopiacore/qtestlib\
        libraries/qtopiatest/host \
        libraries/qtopiatest/target \
        libraries/qtopiatest/overrides
    !x11 {
        PROJECTS*=\
            libraries/qtopiatest/plugin/app \
            libraries/qtopiatest/plugin/server \
            libraries/qtopiatest/qsystemtestrunner
    }
    !x11 {
        # performance test stuff
        PROJECTS*=\
            plugins/qtopiacore/gfxdrivers/perftestqvfb \
            plugins/qtopiacore/gfxdrivers/perftestlinuxfb
    }
}

# non-platform stuff
!platform:!media {
    PROJECTS*=\
        applications/addressbook \
        applications/datebook \
        applications/todo\
	applications/calculator \
	applications/camera \
	applications/clock \
	applications/mediarecorder \
	applications/photoedit \
	applications/sysinfo \
	applications/textedit \
	games/qasteroids\
	games/fifteen\
        games/snake\
        games/minesweep\
        plugins/content/id3 \
        plugins/content/exif
}

PROJECTS*=\
    settings/appearance\
    settings/language \
    settings/logging \
    settings/network \
    settings/systemtime \
    settings/worldtime \
    settings/light-and-power \
    settings/packagemanager\
    settings/homescreen \
    applications/helpbrowser \
    tools/qcop \
    tools/vsexplorer \
    tools/qdawggen \
    tools/dbmigrate \
    tools/device_updater \
    3rdparty/applications/micro_httpd

enable_qtopiamedia {
    PROJECTS*=\
        libraries/qtopiamedia \
        tools/mediaserver \
        applications/mediaplayer

        isEmpty(DEVICE_CONFIG_PATH) {
            PROJECTS*=\
                plugins/audiohardware/desktop
        }

    contains(QTOPIAMEDIA_ENGINES,helix) {
        PROJECTS*=\
            3rdparty/libraries/helix \
            plugins/mediaengines/helix
    }

    contains(QTOPIAMEDIA_ENGINES,gstreamer) {
        PROJECTS*=\
            3rdparty/libraries/gstreamer \
            plugins/mediaengines/gstreamer
    }

    contains(QTOPIAMEDIA_ENGINES,cruxus) {
        PROJECTS*=\
            3rdparty/libraries/libtimidity \
            3rdparty/plugins/codecs/libtimidity \
            plugins/mediaengines/cruxus
    }
}

!enable_qtopiamedia:!x11 {
    PROJECTS*=\
        tools/qss
}

!media:PROJECTS*=\
    settings/security

!no_quicklaunch|enable_singleexec:PROJECTS*=tools/quicklauncher

PROJECTS*=tools/content_installer

build_libamr:PROJECTS*=\
    3rdparty/libraries/amr\
    3rdparty/plugins/codecs/libamr

PROJECTS*=\
    plugins/network/lan \
    plugins/network/dialing \
    plugins/qtopiacore/iconengines/qtopiaiconengine \
    plugins/qtopiacore/iconengines/qtopiasvgiconengine \
    plugins/qtopiacore/iconengines/qtopiapiciconengine \
    plugins/qtopiacore/imageformats/picture

media {
    PROJECTS*=\
	applications/camera \
	applications/clock \
	applications/mediarecorder \
	applications/photoedit \
        plugins/content/id3

    build_helix {
        PROJECTS*=\
            3rdparty/libraries/helix \
            libraries/qtopiamedia \
            applications/mediaplayer
    }
}

PROJECTS*=\
    libraries/qtopiaprinting \
    tools/printserver

enable_bluetooth:PROJECTS*=\
    plugins/qtopiaprinting/bluetooth

enable_infrared:PROJECTS*=\
    settings/beaming

enable_bluetooth:PROJECTS*=\
    settings/btsettings \
    applications/bluetooth \
    plugins/network/bluetooth

enable_dbus {
    PROJECTS*=\
    3rdparty/libraries/dbus \
    3rdparty/libraries/qtdbus
}

enable_dbusipc {
    PROJECTS*=\
    3rdparty/applications/dbus \
    tools/qtopia-dbus-launcher
}

PROJECTS*=\
    libraries/handwriting\
    settings/handwriting

!x11 {
    PROJECTS*=\
        3rdparty/plugins/inputmethods/pkim
}

THEMES *= smart

phone {
    PROJECTS*=\
        3rdparty/libraries/gsm\
        settings/ringprofile\
        settings/speeddial\
        plugins/codecs/wavrecord\

    enable_modem:PROJECTS*=\    
        libraries/qtopiasmil

    enable_infrared:PROJECTS*=\
        settings/beaming

    # Phone Themes
    THEMES*= \
        classic \
        crisp \
        finxi \
        qtopia
}

!platform {
    PROJECTS*=settings/words

    # Qtmail stuff
    !media:PROJECTS*=\
        libraries/qtopiamail\
        applications/qtmail\
        plugins/composers/email\
        plugins/composers/generic\
        plugins/composers/mms\
        plugins/viewers/generic

    !media:enable_modem:PROJECTS*=\    
        plugins/viewers/smil
}

enable_samples:PROJECTS+=settings/serverwidgets

phone {
    PROJECTS*=\
        libraries/qtopiaphone\
        tools/atinterface

    enable_cell {
        PROJECTS*=\
            settings/callforwarding\
            settings/phonenetworks\
            settings/phonesettings
    }

    enable_modem {
        PROJECTS*=\
            libraries/qtopiaphonemodem
        for(p,PHONEVENDORS) {
            exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
        }
        for(m,MULTIPLEXERS) {
            exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
        }
    }

    # This isn't supported but it's included anyway
    PROJECTS*=tools/phonesim tools/phonesim/lib tools/phonesim_target
    CONFIG+=qtopiatest_use_phonesim

    enable_voip:PROJECTS*=\
        3rdparty/libraries/dissipate2\
        tools/sipagent \
        settings/sipsettings
}

drmagent {
    PROJECTS*=\
        3rdparty/libraries/drmagent\
        plugins/drmagent/bscidrmagent \
        settings/drmbrowser
}

enable_sxe {
    PROJECTS*=\
        libraries/qtopiasecurity\
        tools/sxe_installer \
        tools/sxe_policy_runner \
        tools/sxemonitor \
        tools/rlimiter \
        tools/sysmessages
}

