include($$(QPEDIR)/etc/themes/stub.pri)
!builtin_theme {
    PACKAGE_NAME=qpe-mediaplayer-techno-skin
    PACKAGE_DEPENDS=qpe-libmediaplayer
    PACKAGE_DESCRIPTION=Techno skin for Media Player
    TRANSLATIONS=
}

technodata.files=$${QTOPIA_DEPOT_PATH}/pics/mediaplayer/skins/techno/*
technodata.path=/pics/mediaplayer/skins/techno

PICS_INSTALLS+=technodata

