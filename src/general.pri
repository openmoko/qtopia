# This file contains projects that are eligable for inclusion in the Free Edition.

# Qtopia Core files
QTE_PROJECTS=\
    tools/qtopiacore/moc\
    tools/qtopiacore/uic\
    tools/qtopiacore/rcc\
    libraries/qtopiacore/corelib\
    libraries/qtopiacore/gui\
    libraries/qtopiacore/network\
    #libraries/qtopiacore/opengl\
    libraries/qtopiacore/sql\
    libraries/qtopiacore/xml\
    plugins/qtopiacore
!equals(QTE_MINOR_VERSION,1):QTE_PROJECTS+=libraries/qtopiacore/svg
PROJECTS*=$$QTE_PROJECTS

# Qtopia Platform files
PROJECTS*=\
    # A placeholder for installing Qt files
    qt\
    server\
    libraries/qtopia\
    libraries/qtopiail\
    libraries/qtopiacomm\
    libraries/qtopiaaudio\

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
    #obex
    3rdparty/libraries/openobex\
    3rdparty/libraries/inputmatch\
    3rdparty/libraries/sqlite\
    3rdparty/applications/sqlite

enable_ssl:PROJECTS*=\
    3rdparty/libraries/openssl/crypto\
    3rdparty/libraries/openssl/ssl

# input methods that are available in all editions
PROJECTS*=\
    plugins/inputmethods/keyboard\
    plugins/inputmethods/dockedkeyboard

!media:PROJECTS*=\
    libraries/qtopiapim

enable_qtopiabase {
    PROJECTS*=libraries/qtopiabase
    PROJECTS-=libraries/qtopiail
}

# test tools and validator
qtopiatest:PROJECTS*=\
    libraries/qtopiatest/qtesttools/host \
    libraries/qtopiatest/qtesttools/target \
    tools/validator \
    tools/validator/3rdparty/qscintilla

# unit test stuff
qtopiatest:!enable_singleexec:PROJECTS*=\
    libraries/qtopiatest/qunittest

# system test stuff
qtopiatest:PROJECTS*=\
    libraries/qtopiatest/qsystemtest \
    libraries/qtopiatest/qtestslave \
    libraries/qtopiatest/qsystemtestslave \
    libraries/qtopiatest/qtopiasystemtestslave \
    libraries/qtopiatest/qtopiaservertestslave \
    libraries/qtopiatest/overrides

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
	applications/textedit \
	games/qasteroids\
	games/fifteen\
        games/snake\
        games/minesweep\
        plugins/content/id3 \
        plugins/content/exif

    build_helix {
        PROJECTS*=\
            3rdparty/libraries/helix \
            3rdparty/libraries/libtimidity \
            3rdparty/plugins/codecs/libtimidity \
            plugins/mediadevices/builtin \
            libraries/qtopiamedia \
            tools/mediaserver \
            applications/mediaplayer
    }
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
    applications/helpbrowser \
    applications/sysinfo \
    tools/qcop \
    tools/vsexplorer \
    tools/symlinker \
    tools/qdawggen \
    tools/dbmigrate

!build_helix:PROJECTS*=tools/qss

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
    plugins/qtopiacore/iconengines/qtopiasvgiconengine

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

enable_singleexec:PROJECTS*=\
    plugins/qtopiacore/imageformats/jpeg\
    plugins/qtopiacore/imageformats/mng\
    plugins/qtopiacore/imageformats/svg

PROJECTS*=\
    libraries/handwriting\
    settings/handwriting\
    3rdparty/plugins/inputmethods/pkim

THEMES *= smart

phone {
    PROJECTS*=\
        3rdparty/libraries/gsm\
        libraries/qtopiasmil\
        settings/ringprofile\
        settings/speeddial\
        plugins/codecs/wavrecord\

    enable_infrared:PROJECTS*=\
        settings/beaming

    # Phone Themes
    THEMES*=\
        crisp\
        qtopia\
        portal
        # broken in 4.2.0
        # gel
}

!platform {
    PROJECTS*=settings/words

    # Qtmail stuff
    !media:PROJECTS*=\
        libraries/qtopiamail\
        applications/qtmail
}

enable_samples:PROJECTS+=settings/serverwidgets

