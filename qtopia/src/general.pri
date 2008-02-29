#These are in all packages, free and commercial

SERVER=server

CORE_LIBRARY_PROJECTS += qt libraries/qtopia

CORE_LIBRARY_PROJECTS+=libraries/qtopia1
QTOPIA_SQL:LIBRARY_PROJECTS += libraries/qtopiasql
CORE_LIBRARY_PROJECTS+=libraries/qtopia2

!buildSingleexec:LIBRARY_PROJECTS+=3rdparty/plugins/obex/openobex
!buildSingleexec:PLUGIN_PROJECTS+=3rdparty/plugins/obex

!QTOPIA_CORE {
    LIBRARY_PROJECTS+=\
	libraries/qtopiapim\
	libraries/mediaplayer\
	libraries/qtopiacalc

    !QTOPIA_PHONE:THEME_PROJECTS+=mediaplayer/techno

    LIBRARY_PROJECTS+=libraries/qtopiapim1
}

BUILD_LIBFREETYPE:LIBRARY_PROJECTS += 3rdparty/libraries/freetype

QTOPIA_SQLITE {
    LIBRARY_PROJECTS+=3rdparty/libraries/sqlite
    PLUGIN_PROJECTS+=plugins/sqldrivers/sqlite
}

QTOPIA_SSL|QTOPIA_VOIP {
    LIBRARY_PROJECTS+=\
        3rdparty/libraries/openssl/crypto\
        3rdparty/libraries/openssl/ssl
}

QTOPIA_VOIP {
    LIBRARY_PROJECTS+=3rdparty/applications/kphone/gsm
    LIBRARY_PROJECTS+=3rdparty/applications/kphone/ilbc
    LIBRARY_PROJECTS+=3rdparty/applications/kphone/dissipate2
    LIBRARY_PROJECTS+=3rdparty/applications/kphone/voipuseragent
}

# in everything
buildPointerApps {

# these are in core and pda, but not phone.  mostly because they don't work
# with phone.  that may change.
    !QTOPIA_PHONE {
	PLUGIN_PROJECTS += plugins/inputmethods/handwriting\
	    plugins/inputmethods/keyboard\
	    plugins/inputmethods/unikeyboard
    }
}

!QTOPIA_CORE {
    APP_PROJECTS+=\
	applications/addressbook \
	applications/calculator \
	applications/camera \
	applications/clock \
	applications/datebook \
	applications/mediarecorder \
	applications/music \
	applications/videos \
	applications/photoedit \
	applications/sysinfo \
	applications/textedit \
	applications/todo\
	games/fifteen\
	games/minesweep\
	games/parashoot\
	games/qasteroids\
	games/snake\
	games/solitaire
    !free_package:APP_PROJECTS+=settings/words
}

APP_PROJECTS+=\
    applications/qss \
    settings/appearance\
    settings/language \
    settings/network \
    settings/security \
    settings/systemtime \
    settings/worldtime \
    settings/light-and-power \
    applications/helpbrowser \
    tools/quicklauncher \
    tools/qcop \
    tools/qdawggen \
    tools/symlinker

contains(QTOPIA_ARCH,sharp):APP_PROJECTS+=3rdparty/tools/atd

qtest {
    INCLUDEPATH	+= $$(QTESTDIR)/qtopia $$(QTESTDIR)/qtestremote
}

QTOPIA_LIBMAD:PLUGIN_PROJECTS+=3rdparty/plugins/codecs/libmad
QTOPIA_LIBFFMPEG:PLUGIN_PROJECTS+=3rdparty/plugins/codecs/libffmpeg
QTOPIA_LIBAMR:PLUGIN_PROJECTS+=3rdparty/plugins/codecs/libamr

PLUGIN_PROJECTS+=\
    plugins/decorations/flat\
    plugins/fontfactories/freetype\
    plugins/imagecodecs/notepad\
    plugins/imagecodecs/wbmp\
    plugins/imagecodecs/ota\
    plugins/textcodecs/simple8\
    plugins/network/dialup\
    plugins/network/lan\
    3rdparty/plugins/textcodecs/jp

QTOPIA_PDA {
    APP_PROJECTS+=\
	applications/today \
	games/mindbreaker\
	settings/appservices \
	settings/buttoneditor \
	settings/launchersettings \
	settings/rotation \
	settings/sound \
	settings/pluginmgr \
	settings/qipkg \
	3rdparty/applications/embeddedkonsole\
	3rdparty/applications/keypebble

    PLUGIN_PROJECTS+=\
	plugins/applets/batteryapplet\
	plugins/applets/brightness\
	plugins/applets/clipboardapplet\
	plugins/applets/clockapplet\
	plugins/applets/irreceiver\
	plugins/applets/netmonapplet\
	plugins/applets/volumeapplet\
	plugins/applets/mountmon\
	plugins/decorations/polished\
	plugins/styles/flat\
	plugins/styles/fresh\
	plugins/today/datebook\
	plugins/today/todo\
	plugins/calculator/fraction\
	plugins/calculator/conversion\
	plugins/calculator/simple\
	plugins/calculator/advanced\
	3rdparty/plugins/applets/cardmon

    buildPointerApps:PLUGIN_PROJECTS+=3rdparty/plugins/inputmethods/pickboard
}

# Things that dont work or are not needed in the singleexec build
buildSingleexec {
    APP_PROJECTS-=\
	tools/quicklauncher\
	tools/qcop\
	tools/symlinker\
	applications/mediarecorder \
	applications/scribble \
	settings/pluginmgr \
	settings/qipkg \
	3rdparty/applications/embeddedkonsole\
	3rdparty/applications/keypebble

    LIBRARY_PROJECTS-=\
	3rdparty/libraries/freetype

    PLUGIN_PROJECTS-=\
	3rdparty/plugins/codecs/libmad\
	plugins/fontfactories/freetype\
	3rdparty/plugins/inputmethods/pickboard
}

# Projects which exist only to fulfill dependancies
contains(APP_PROJECTS,applications/mediarecorder) {
    PLUGIN_PROJECTS+=\
	plugins/codecs/wavplugin\
	plugins/codecs/wavrecord
}

contains(PLUGIN_PROJECTS,3rdparty/plugins/codecs/libffmpeg) | contains(PLUGIN_PROJECTS,3rdparty/plugins/codecs/libamr) {
    LIBRARY_PROJECTS+=\
	3rdparty/libraries/amr
}

contains(PLUGIN_PROJECTS,3rdparty/plugins/codecs/libffmpeg) {
    LIBRARY_PROJECTS+=\
	3rdparty/libraries/libavcodec\
	3rdparty/libraries/libavformat
}

contains(PLUGIN_PROJECTS,plugins/codecs/wavplugin) | contains(PLUGIN_PROJECTS,plugins/codecs/wavrecord):LIBRARY_PROJECTS+=3rdparty/libraries/gsm

contains(PLUGIN_PROJECTS,plugins/inputmethods/handwriting) {
    LIBRARY_PROJECTS += libraries/handwriting
    APP_PROJECTS += settings/handwriting
}

