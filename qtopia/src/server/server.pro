TEMPLATE	= app

TARGET		= qpe
DESTDIR         = $$(QPEDIR)/bin

CONFIG		+= qtopiainc qtopialib

QTOPIA_PROJECT_TYPE=server

buildSingleexec {
    TEMPLATE=app

    SERVER_HEADERS += singleexec_quicklaunch.h
    SERVER_SOURCES += singleexec_quicklaunch.cpp

    # qcop is now part of the server
    QCOP_HEADERS += $${QTOPIA_DEPOT_PATH}/src/tools/qcop/qcopimpl.h
    QCOP_SOURCES += $${QTOPIA_DEPOT_PATH}/src/tools/qcop/qcopimpl.cpp

    sdk_qcop_headers.files=$${QCOP_HEADERS}
    sdk_qcop_headers.path=/src/tools/qcop/
    sdk_qcop_headers.CONFIG += no_default_install
    devsdk_qcop_sources.files=$${QCOP_SOURCES}
    devsdk_qcop_sources.path=/src/tools/qcop/
    devsdk_qcop_sources.CONFIG += no_default_install
    INSTALLS+= sdk_qcop_headers devsdk_qcop_sources
    sdk.depends += install_sdk_qcop_headers
    devsdk.depends += install_devsdk_qcop_sources

    system(ln -sf qpe $QPEDIR/bin/qcop)
    qcopSymlink.path=/bin
    qcopSymlink.commands=$${COMMAND_HEADER}ln -sf qpe "$(INSTALL_ROOT)$${qcopSymlink.path}/qcop"
    INSTALLS+=qcopSymlink

    INCLUDEPATH+=$${QTOPIA_DEPOT_PATH}/src/settings/backuprestore \
    $${QTOPIA_DEPOT_PATH}/src/libraries/qtopiacalc \
    $${QTOPIA_DEPOT_PATH}/src/libraries/mediaplayer

    buildSingleExec:include($$(QPEDIR)/src/server/singleexec_server_includes.pri)

    LIBS+=-Wl,-whole-archive 
    buildSingleExec:include($$(QPEDIR)/src/server/singleexec_server_libs.pri)
    LIBS+=-Wl,-no-whole-archive

    !QTOPIA_CORE:CONFIG+=pimlib
    LIBS+=-lcrypt -luuid
}

TRANSLATABLES +=  $${QTOPIA_DEPOT_PATH}/src/tools/qcop/qcopimpl.h \
                    $${QTOPIA_DEPOT_PATH}/src/tools/qcop/qcopimpl.cpp \
                    singleexec_quicklaunch.h \
                    singleexec_quicklaunch.cpp


qtest {
#    DEFINES += QTOPIA_SERVER
    include($$(QTESTDIR)/include/qtest_qpeserver.pri)
}

SERVER_HEADERS	+= server.h \
		  serverinterface.h \
		  documentlist.h \
		  suspendmonitor.h \
		  appicons.h \
		  applauncher.h \
		  stabmon.h \
		  inputmethods.h \
		  wait.h \
		  shutdownimpl.h \
		  transferserver.h \
		  qcopbridge.h \
		  packageslave.h \
		  irserver.h \
		  syncdialog.h \
		  serverapp.h \
		  qrr.h \

SERVER_SOURCES	+= main.cpp \
		  server.cpp \
		  serverinterface.cpp \
		  documentlist.cpp \
		  suspendmonitor.cpp \
		  appicons.cpp \
		  applauncher.cpp \
		  stabmon.cpp \
		  inputmethods.cpp \
		  wait.cpp \
		  shutdownimpl.cpp \
		  transferserver.cpp \
		  packageslave.cpp \
   		  irserver.cpp \
		  qcopbridge.cpp \
		  syncdialog.cpp \
		  serverapp.cpp \
		  qrr.cpp 


FIRSTUSE_HEADERS += ../settings/language/languagesettings.h \
            ../settings/systemtime/settime.h

FIRSTUSE_SOURCES += ../settings/language/language.cpp \
            ../settings/systemtime/settime.cpp

FIRSTUSE_INTERFACES += ../settings/language/languagesettingsbase.ui

unix{
    !buildSingleExec {
        SERVER_HEADERS	+= $${FIRSTUSE_HEADERS}
        SERVER_SOURCES	+= $${FIRSTUSE_SOURCES}
        INTERFACES	+= $${FIRSTUSE_INTERFACES}
    }
    buildSingleExec:!contains(APP_PROJECTS, settings/language) {

        FIRSTUSE_HEADERS -= ../settings/systemtime/settime.h
        FIRSTUSE_SOURCES -= ../settings/systemtime/settime.cpp
        SERVER_HEADERS	+= $${FIRSTUSE_HEADERS}
        SERVER_SOURCES	+= $${FIRSTUSE_SOURCES}
        INTERFACES	+= $${FIRSTUSE_INTERFACES}
    }
    buildSingleExec:!contains(APP_PROJECTS, settings/systemtime) {
	SERVER_HEADERS += ../settings/systemtime/settime.h
	SERVER_SOURCES += ../settings/systemtime/settime.cpp
    }
}



INTERFACES += shutdown.ui 
TRANSLATABLES += $${SERVER_HEADERS} $${SERVER_SOURCES} $${INTERFACES} \
                $${FIRSTUSE_HEADERS} $${FIRSTUSE_SOURCES} $${FIRSTUSE_INTERFACES}

sdk_server_headers.files=$${SERVER_HEADERS}
sdk_server_headers.path=/src/server/
sdk_server_headers.CONFIG += no_default_install
devsdk_server_sources.files=$${SERVER_SOURCES} $${INTERFACES} $${FORMS}
devsdk_server_sources.path=/src/server/
devsdk_server_sources.CONFIG += no_default_install
INSTALLS+= sdk_server_headers devsdk_server_sources
sdk.depends += install_sdk_server_headers
devsdk.depends += install_devsdk_server_sources

CALIBRATE_HEADERS += ../settings/calibrate/calibrate.h
CALIBRATE_SOURCES += ../settings/calibrate/calibrate.cpp
sdk_calibrate_headers.files=$${CALIBRATE_HEADERS}
sdk_calibrate_headers.path=/src/settings/calibrate/
sdk_calibrate_headers.CONFIG += no_default_install
devsdk_calibrate_sources.files=$${CALIBRATE_SOURCES}
devsdk_calibrate_sources.path=/src/settings/calibrate/
devsdk_calibrate_sources.CONFIG += no_default_install
INSTALLS+= sdk_calibrate_headers devsdk_calibrate_sources
sdk.depends += install_sdk_calibrate_headers
devsdk.depends += install_devsdk_calibrate_sources

TRANSLATABLES += ../settings/calibrate/calibrate.h \
                    ../settings/calibrate/calibrate.cpp

# RSYNC
# Cant use a vpath because these need to be installed also
RSYNC_HEADERS += \
	../3rdparty/libraries/rsync/buf.h \
	../3rdparty/libraries/rsync/checksum.h \
	../3rdparty/libraries/rsync/command.h \
	../3rdparty/libraries/rsync/emit.h \
	../3rdparty/libraries/rsync/job.h \
	../3rdparty/libraries/rsync/netint.h \
	../3rdparty/libraries/rsync/protocol.h \
	../3rdparty/libraries/rsync/prototab.h \
	../3rdparty/libraries/rsync/rsync.h \
	../3rdparty/libraries/rsync/search.h \
	../3rdparty/libraries/rsync/stream.h \
	../3rdparty/libraries/rsync/sumset.h \
	../3rdparty/libraries/rsync/trace.h \
	../3rdparty/libraries/rsync/types.h \
	../3rdparty/libraries/rsync/util.h \
	../3rdparty/libraries/rsync/whole.h \
	../3rdparty/libraries/rsync/config_rsync.h \
	../3rdparty/libraries/rsync/qrsync.h
RSYNC_SOURCES += \
	../3rdparty/libraries/rsync/base64.c \
	../3rdparty/libraries/rsync/buf.c \
	../3rdparty/libraries/rsync/checksum.c \
	../3rdparty/libraries/rsync/command.c \
	../3rdparty/libraries/rsync/delta.c \
	../3rdparty/libraries/rsync/emit.c \
	../3rdparty/libraries/rsync/hex.c \
	../3rdparty/libraries/rsync/job.c \
	../3rdparty/libraries/rsync/mdfour.c \
	../3rdparty/libraries/rsync/mksum.c \
	../3rdparty/libraries/rsync/msg.c \
	../3rdparty/libraries/rsync/netint.c \
	../3rdparty/libraries/rsync/patch.c \
	../3rdparty/libraries/rsync/prototab.c \
	../3rdparty/libraries/rsync/readsums.c \
	../3rdparty/libraries/rsync/scoop.c \
	../3rdparty/libraries/rsync/search.c \
	../3rdparty/libraries/rsync/stats.c \
	../3rdparty/libraries/rsync/stream.c \
	../3rdparty/libraries/rsync/sumset.c \
	../3rdparty/libraries/rsync/trace.c \
	../3rdparty/libraries/rsync/tube.c \
	../3rdparty/libraries/rsync/util.c \
	../3rdparty/libraries/rsync/version.c \
	../3rdparty/libraries/rsync/whole.c \
	../3rdparty/libraries/rsync/qrsync.cpp

TRANSLATABLES += $${RSYNC_HEADERS} $${RSYNC_SOURCES}

sdk_rsync_headers.files=$${RSYNC_HEADERS}
sdk_rsync_headers.path=/src/3rdparty/libraries/rsync
sdk_rsync_headers.CONFIG += no_default_install
devsdk_rsync_sources.files=$${RSYNC_SOURCES}
devsdk_rsync_sources.path=/src/3rdparty/libraries/rsync
devsdk_rsync_sources.CONFIG += no_default_install
INSTALLS+= sdk_rsync_headers devsdk_rsync_sources
sdk.depends += install_sdk_rsync_headers
devsdk.depends += install_devsdk_rsync_sources

PDA_HEADERS += pda/launcherview.h pda/loadingwidget.h
PDA_SOURCES += pda/launcherview.cpp pda/loadingwidget.cpp

TRANSLATABLES += $${PDA_HEADERS} $${PDA_SOURCES}

PHONE_HEADERS_CONTAINER += phone/phoneimpl.h \
	    phone/contextlabel.h \
	    phone/phonelauncher.h \
	    phone/homescreen.h \
	    phone/phonelauncherview.h \
	    phone/callhistory.h \
	    phone/calllistbox.h \
	    phone/phoneprofile.h \
	    phone/callscreen.h \
	    phone/phonemanager.h \
	    phone/dialercontrol.h \
	    phone/messagebox.h \
	    phone/quickdial.h \
	    phone/numberdisplay.h \
	    phone/dialer.h \
            phone/phonestatusserver.h

PHONE_SOURCES_CONTAINER += phone/phoneimpl.cpp \
	    phone/contextlabel.cpp \
	    phone/phonelauncher.cpp \
	    phone/homescreen.cpp \
	    phone/phonelauncherview.cpp \
	    phone/callhistory.cpp \
	    phone/calllistbox.cpp \
	    phone/phoneprofile.cpp \
	    phone/callscreen.cpp \
	    phone/phonemanager.cpp \
	    phone/dialercontrol.cpp \
	    phone/messagebox.cpp \
	    phone/quickdial.cpp \
	    phone/numberdisplay.cpp \
	    phone/dialer.cpp  \
            phone/phonestatusserver.cpp

PDA_HEADERS_CONTAINER += pda/launcher.h \
	    pda/launchertab.h \
	    pda/taskbar.h \
	    pda/runningappbar.h \
	    pda/systray.h \
	    pda/startmenu.h

PDA_SOURCES_CONTAINER += pda/launcher.cpp \
	    pda/launchertab.cpp \
	    pda/taskbar.cpp \
	    pda/runningappbar.cpp \
	    pda/systray.cpp \
	    pda/startmenu.cpp 
            
SERVER_HEADERS_FIRSTUSE = firstuse.h
SERVER_SOURCES_FIRSTUSE = firstuse.cpp

QTOPIA_PHONE{
    PHONE_HEADERS += $${PHONE_HEADERS_CONTAINER}
    PHONE_SOURCES += $${PHONE_SOURCES_CONTAINER}

    CONFIG*=pimlib
    QTOPIA_SQL:LIBS += -lqtopiasql
    LIBS += -lqcopbridge
} else {
    PDA_HEADERS += $${PDA_HEADERS_CONTAINER}
    PDA_SOURCES += $${PDA_SOURCES_CONTAINER}
}
unix{
    SERVER_HEADERS	+= $${SERVER_HEADERS_FIRSTUSE}
    SERVER_SOURCES += $${SERVER_SOURCES_FIRSTUSE}
}


TRANSLATABLES += $${PHONE_HEADERS_CONTAINER} \
                    $${PHONE_SOURCES_CONTAINER} \
                    $${PDA_SOURCES_CONTAINER} \
                    $${PDA_SOURCES_CONTAINER} \
                    $${SERVER_HEADERS_FIRSTUSE} \
                    $${SERVER_SOURCES_FIRSTUSE} \

QTOPIA_PHONE{
    sdk_phone_headers.files=$${PHONE_HEADERS}
    sdk_phone_headers.path=/src/server/phone
    sdk_phone_headers.CONFIG += no_default_install
    devsdk_phone_sources.files=$${PHONE_SOURCES}
    devsdk_phone_sources.path=/src/server/phone
    devsdk_phone_sources.CONFIG += no_default_install
    INSTALLS+= sdk_phone_headers devsdk_phone_sources
    sdk.depends += install_sdk_phone_headers
    devsdk.depends += install_devsdk_phone_sources
} else {
    sdk_pda_headers.files=$${PDA_HEADERS}
    sdk_pda_headers.path=/src/server/pda
    sdk_pda_headers.CONFIG += no_default_install
    devsdk_pda_sources.files=$${PDA_SOURCES}
    devsdk_pda_sources.path=/src/server/pda
    devsdk_pda_sources.CONFIG += no_default_install
    INSTALLS+= sdk_pda_headers devsdk_pda_sources
    sdk.depends += install_sdk_pda_headers
    devsdk.depends += install_devsdk_pda_sources
}

INCLUDEPATH += ../settings/calibrate
DEPENDPATH  += ../settings/calibrate

INCLUDEPATH += ../3rdparty/libraries/rsync
DEPENDPATH  += ../3rdparty/libraries/rsync

HEADERS += $${QCOP_HEADERS} $${SERVER_HEADERS} $${RSYNC_HEADERS} $${CALIBRATE_HEADERS} $${PHONE_HEADERS} $${PDA_HEADERS}
SOURCES += $${QCOP_SOURCES} $${SERVER_SOURCES} $${RSYNC_SOURCES} $${CALIBRATE_SOURCES} $${PHONE_SOURCES} $${PDA_SOURCES}

bins.files=$${QTOPIA_DEPOT_PATH}/bin/targzip\
$${QTOPIA_DEPOT_PATH}/bin/targunzip\
$${QTOPIA_DEPOT_PATH}/bin/qtopia-update-symlinks
bins.path=/bin

calibratedesktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Calibrate.desktop
calibratedesktop.path=/apps/Settings
calibrate.files=$${QTOPIA_DEPOT_PATH}/services/calibrate/calibrate
calibrate.path=/services/calibrate/
calibratepics.files=$${QTOPIA_DEPOT_PATH}/pics/calibrate/*
calibratepics.path=/pics/calibrate/
INSTALLS+=calibrate calibratedesktop
PICS_INSTALLS+=calibratepics

appspics.files=$${QTOPIA_DEPOT_PATH}/pics/*.* $${QTOPIA_DEPOT_PATH}/pics/icons $${QTOPIA_DEPOT_PATH}/pics/i18n
appspics.path=/pics
pics.files=$${QTOPIA_DEPOT_PATH}/pics/qpe/*.* $${QTOPIA_DEPOT_PATH}/pics/qpe/icons $${QTOPIA_DEPOT_PATH}/pics/qpe/i18n
pics.path=/pics/qpe
qpesettings.files=$${QTOPIA_DEPOT_PATH}/etc/default/qpe.conf
qpesettings.path=/etc/default/
INSTALLS+=qpesettings
localesettings.files=$${QTOPIA_DEPOT_PATH}/etc/default/locale.conf
localesettings.path=/etc/default/
INSTALLS+=localesettings
fontmapsettings.files=$${QTOPIA_DEPOT_PATH}/etc/default/FontMap.conf
fontmapsettings.path=/etc/default
INSTALLS+=fontmapsettings
secsettings.files=$${QTOPIA_DEPOT_PATH}/etc/default/Security.conf
secsettings.path=/etc/default/
INSTALLS+=secsettings
wallpaperpics.files=$${QTOPIA_DEPOT_PATH}/pics/wallpaper/*
wallpaperpics.path=/pics/wallpaper
INSTALLS+=wallpaperpics
inputmethods.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/.directory
inputmethods.path=/plugins/inputmethods/
INSTALLS+=inputmethods
obex.files=$${QTOPIA_DEPOT_PATH}/plugins/obex/.directory
obex.path=/plugins/obex/
INSTALLS+=obex
network.files=$${QTOPIA_DEPOT_PATH}/plugins/network/.directory
network.path=/plugins/network/
INSTALLS+=network

QTOPIA_PHONE {
    phonepics.files=$${QTOPIA_DEPOT_PATH}/pics/qpe/phone/*
    phonepics.path=/pics/qpe/phone/
    globalphonepics.files=$${QTOPIA_DEPOT_PATH}/pics/phone/*
    globalphonepics.path=/pics/phone/
    contextbarpics.files=$${QTOPIA_DEPOT_PATH}/pics/contextbar/*
    contextbarpics.path=/pics/contextbar
    cameradesktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/camera.desktop
    cameradesktop.path=/apps/Applications/
    profilesettings.files=$${QTOPIA_DEPOT_PATH}/etc/default/PhoneProfile.conf
    profilesettings.path=/etc/default/
    defaultalerts.files=$${QTOPIA_DEPOT_PATH}/etc/SystemRingTones/*
    defaultalerts.path=/etc/SystemRingTones/
    callhistorydesktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/callhistory.desktop
    callhistorydesktop.path=/apps/Applications/
    callhistorypics.files=$${QTOPIA_DEPOT_PATH}/pics/callhistory/*
    callhistorypics.path=/pics/callhistory/
    callhistoryservice.files=$${QTOPIA_DEPOT_PATH}/services/callhistory/qpe
    callhistoryservice.path=/services/callhistory/
    defaultpics.files=$${QTOPIA_DEPOT_PATH}/pics/themes/default*
    defaultpics.path=/pics/themes

    INSTALLS+=cameradesktop profilesettings defaultalerts callhistorydesktop callhistoryservice
    PICS_INSTALLS+=phonepics globalphonepics contextbarpics callhistorypics defaultpics

    CONFIG+=builtin_theme
    include($${QTOPIA_DEPOT_PATH}/etc/themes/crisp/crisp.pro)
    include($${QTOPIA_DEPOT_PATH}/etc/themes/qtopia/qtopia.pro)
}

!QTOPIA_PHONE {
    pdapics.files=$${QTOPIA_DEPOT_PATH}/pics/qpe/pda/*
    pdapics.path=/pics/qpe/pda/
    applets.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/.directory
    applets.path=/plugins/applets/
    INSTALLS+=shutdown applets
    PICS_INSTALLS+=pdapics
}

ANIMFILE=$${QTOPIA_DEPOT_PATH}/pics/qpe/splash/$${QTOPIA_DISP_WIDTH}x$${QTOPIA_DISP_HEIGHT}/splash.gif
exists($${ANIMFILE}) {
    splash.files=$${ANIMFILE}
} else {
    # Fall-back un-animated
    splash.files=$${QTOPIA_DEPOT_PATH}/src/server/phone/splash.png
}
splash.path=/pics/qpe
# Currently only used on phone
QTOPIA_PHONE:INSTALLS+=splash

sounds.files=$${QTOPIA_DEPOT_PATH}/sounds/speeddial
sounds.path=/sounds/
QTOPIA_PHONE {
dialerservice.files=$${QTOPIA_DEPOT_PATH}/services/Dialer.service
dialerservice.path=/services/
dialer.files=$${QTOPIA_DEPOT_PATH}/services/Dialer/qpe
dialer.path=/services/Dialer/
}
shutdown.files=$${QTOPIA_DEPOT_PATH}/services/shutdown/shutdown
shutdown.path=/services/shutdown/
target.path=/bin
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=\
    index.html\
    qpe*\
    help-*\
    help.html\
    document*\
    appservices.html\
    calibrat*\

!QTOPIA_PHONE {
    help.files+=\
	categor*\
	about*\
	backuprestore.html\
	beaming.html\
	sync.html\
	popup-calendar.html
}
beam.files=$${QTOPIA_DEPOT_PATH}/etc/beam
beam.path=/etc/
QTOPIA_PHONE:INSTALLS+=dialerservice dialer target bins beam
!QTOPIA_PHONE:INSTALLS+=target bins beam
PICS_INSTALLS+=appspics pics
HELP_INSTALLS+=help

iconsettings.files=$$(QPEDIR)/etc/default/IconSizes.conf
iconsettings.path=/etc/default/
INSTALLS+=iconsettings

defaultbuttons {
    defbtn.files=$$(QPEDIR)/etc/defaultbuttons.conf
    defbtn.path=/etc
    INSTALLS+=defbtn
}

#en_US/.directory is always available
dicts.commands=$${COMMAND_HEADER}\
    mkdir -p $(INSTALL_ROOT)/i18n/en_US;\
    install -c $${QTOPIA_DEPOT_PATH}/i18n/en_US/.directory $(INSTALL_ROOT)/i18n/en_US;\
    mkdir -p $(INSTALL_ROOT)/etc/dict/en_US;\
    if [ -f $${QTOPIA_DEPOT_PATH}/etc/dict/en_US/words.dawg ]; then\
        install -c $${QTOPIA_DEPOT_PATH}/etc/dict/en_US/*.dawg $(INSTALL_ROOT)/etc/dict/en_US;\
    fi;\
    LANGUAGES="$$LANGUAGES"; for lang in \$$LANGUAGES; do\
	mkdir -p $(INSTALL_ROOT)/i18n/\$$lang;\
	install -c $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/.directory $(INSTALL_ROOT)/i18n/\$$lang;\
	mkdir -p $(INSTALL_ROOT)/etc/dict/\$$lang;\
	if [ -f $${QTOPIA_DEPOT_PATH}/etc/dict/\$$lang/words.dawg ]; then\
	    install -c $${QTOPIA_DEPOT_PATH}/etc/dict/\$$lang/*.dawg $(INSTALL_ROOT)/etc/dict/\$$lang;\
	fi;\
    done
dicts.CONFIG=no_path
INSTALLS+=dicts

PACKAGES=QtopiaApps QtopiaApplications QtopiaGames QtopiaSettings QtopiaI18N QtopiaServices\
         QtopiaNetworkServices QtopiaBeaming QtopiaColorSchemes QtopiaDefaults\
	 QtopiaMediaPlayerSkins QtopiaPlugins Categories-qtopia QtopiaRingTones \
         QtopiaThemes
i18n.commands=$${COMMAND_HEADER}\
    TRANSLATIONS="$$TRANSLATIONS"; for lang in \$$TRANSLATIONS; do\
	PACKAGES="$$PACKAGES"; for pkg in \$$PACKAGES; do\
	    $${DQTDIR}/bin/lrelease $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/\$$pkg.ts \
		-qm $(INSTALL_ROOT)/i18n/\$$lang/\$$pkg.qm;\
	done;\
    done
i18n.CONFIG=no_path
!isEmpty(DQTDIR):INSTALLS+=i18n

nct_lupdate.commands=$${COMMAND_HEADER}\
    cd $${QTOPIA_DEPOT_PATH}/src/server;\
    $${QTOPIA_DEPOT_PATH}/bin/nct_lupdate -nowarn -depot "$${QTOPIA_DEPOT_PATH}" "$$TRANSLATIONS"\
	`find $${QTOPIA_DEPOT_PATH} -type f -a \
	    # apps, plugins and i18n
	    \( \( -path '$${QTOPIA_DEPOT_PATH}/apps/*' -o\
	          -path '$${QTOPIA_DEPOT_PATH}/plugins/*' -o\
	          -path '$${QTOPIA_DEPOT_PATH}/i18n/*' \) -a\
	       # .directory and .desktop files
	       \( -name '.directory' -o -name '*.desktop' \) \) -o\
	    # etc
	    \( \( -path '$${QTOPIA_DEPOT_PATH}/etc/*' \) -a\
	       # .conf and .scheme files
	       \( -name '*.conf' -o -name '*.scheme' -o -name '*.desktop' \) \) -o\
	    # pics
	    \( \( -path '$${QTOPIA_DEPOT_PATH}/pics/*' \) -a\
	       # config files (media player skins)
	       \( -name 'config' \) \) -o\
	    # services (all files)
	    \( -path '$${QTOPIA_DEPOT_PATH}/services/*' \)`
lupdate.depends+=nct_lupdate
QMAKE_EXTRA_UNIX_TARGETS+=nct_lupdate

PACKAGE_NAME = qpe-taskbar
PACKAGE_DESCRIPTION = Launcher for QPE
PACKAGE_DEPENDS	+= qpe-libqtopia2

