QTOPIA_CALC_PACKAGE=libraries/qtopiacalc\
applications/calculator\
plugins/calculator/simple
contains(QTOPIA_CALC_PACKAGE,$${QTOPIA_ID}) {
    PACKAGE_NAME=qpe-calculator # TODO plugin version
    IPK_GROUP_PROJECTS=$${QTOPIA_CALC_PACKAGE}
    PACKAGE_DESCRIPTION=A pluggable calculator for the Qtopia environment that includes a simple interface.
}

QTOPIA_OBEX_PACKAGE=3rdparty/plugins/obex/openobex\
3rdparty/plugins/obex
contains(QTOPIA_OBEX_PACKAGE,$${QTOPIA_ID}) {
    PACKAGE_NAME=qpe-obex
    IPK_GROUP_PROJECTS=$${QTOPIA_OBEX_PACKAGE}
    PACKAGE_DESCRIPTION=Qtopia OBEX plugin.
}

QTOPIA_MEDIAPLAYER_PACKAGE=libraries/mediaplayer\
3rdparty/plugins/codecs/libffmpeg\
3rdparty/libraries/libavcodec\
3rdparty/libraries/libavformat
contains(QTOPIA_MEDIAPLAYER_PACKAGE,$${QTOPIA_ID}) {
    PACKAGE_NAME=qpe-libmediaplayer
    IPK_GROUP_PROJECTS=$${QTOPIA_MEDIAPLAYER_PACKAGE}
    PACKAGE_DESCRIPTION=Audio/Video Media Player
}

QTOPIA_NETSETUP_PACKAGE=settings/network\
plugins/network/dialup\
plugins/network/lan
contains(QTOPIA_NETSETUP_PACKAGE,$${QTOPIA_ID}) {
    PACKAGE_NAME=qpe-netsetup
    IPK_GROUP_PROJECTS=$${QTOPIA_NETSETUP_PACKAGE}
    PACKAGE_DESCRIPTION=Network configuration utility
}
