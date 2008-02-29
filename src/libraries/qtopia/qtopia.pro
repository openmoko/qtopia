qtopia_project(qtopia core lib)
TARGET=qtopia

VERSION=4.0.0
UI_HEADERS_DIR=$$QPEDIR/include/qtopia/private

DEFINES+=QTOPIA_PAGE_SIZE=$$LITERAL_SQUOTE$$QTOPIA_PAGE_SIZE$$LITERAL_SQUOTE
DEFINES+=QTOPIA_PAGE_MASK=$$LITERAL_SQUOTE$$QTOPIA_PAGE_MASK$$LITERAL_SQUOTE
QTOPIA_FORMS+=passwordbase_p.ui

#DEFINES+=EZX_A780

QTOPIA_HEADERS+=\
    qmimetype.h\
    qdocumentselector.h\
    qtopiaapplication.h\
    qtopiasxe.h \
    qterminationhandler.h\
    qtranslatablesettings.h\
    qtopialog.h\
    qtopialog-config.h\
    qlog.h\
    qtimezonewidget.h\
    qworldmap.h\
    qpassworddialog.h\
    qtimestring.h\
    qstorage.h\
    qcategorymanager.h\
    qcategoryselector.h\
    qwindowdecorationinterface.h\
    mediarecorderplugininterface.h\
    qdawg.h\
    custom.h\
    qtopiaglobal.h\
    inputmethodinterface.h\
    qtaskbarappletplugin.h\
    todayplugininterface.h\
    qapplicationplugin.h\
    version.h\
    qpluginmanager.h\
    quniqueid.h\
    qwaitwidget.h\
    qwizard.h\
    qtopianamespace.h\
    qtimezone.h\
    qthumbnail.h\
    qimagedocumentselector.h\
    qtopiasql.h\
    qiconselector.h\
    qsoundcontrol.h\
    inheritedshareddata.h\
    qtopiaipcenvelope.h\
    qtopiaipcmarshal.h\
    qtopiaipcadaptor.h\
    qtopiachannel.h\
    qtopiaabstractservice.h\
    qtopiaservices.h\
    qtopiaserviceselector.h\
    qpowerstatus.h\
    qcolorselector.h\
    qdocumentproperties.h\
    qdevicebutton.h\
    qdevicebuttonmanager.h\
    qdl.h\
    qdlbrowserclient.h\
    qdlclient.h\
    qdleditclient.h\
    qdllink.h\
    qdsaction.h\
    qdsservices.h\
    qdsserviceinfo.h\
    qdsactionrequest.h\
    qdsdata.h\
    qtopianetwork.h\
    qexportedbackground.h\
    qtopianetworkinterface.h\
    qtopiastyle.h\
    qpdastyle.h\
    qphonestyle.h\
    ezxphonestyle.h\
    qsoftmenubar.h\
    qstartmenuaccessory.h\
    qspeeddial.h\
    qstoragedeviceselector.h\
    perftest.h\
    qsignalintercepter.h\
    qconstcstring.h\
    qtopiafeatures.h\
    qdeviceindicators.h\
    themedview.h\
    qslotinvoker.h\
    qphoneprofile.h\
    qabstractipcinterface.h\
    qabstractipcinterfacegroup.h\
    qabstractipcinterfacegroupmanager.h\
    qhardwareinterface.h\
    qhardwaremanager.h\
    qbatteryaccessory.h\
    qbootsourceaccessory.h\
    qhandsfreeaccessory.h\
    qhandsetaccessory.h\
    qheadsetaccessory.h\
    qinfraredaccessory.h\
    qserialcommaccessory.h\
    qvibrateaccessory.h\
    qspeakerphoneaccessory.h\
    qkeypadlightaccessory.h\
    qtopiasendvia.h\
    qanalogclock.h\
    qsignalsource.h

QTOPIA_PRIVATE_HEADERS+=\
    qimagedocumentselector_p.h\
    thumbnailview_p.h\
    singleview_p.h\
    qactionconfirm_p.h\
    qtopiaresource_p.h\
    qpedecoration_p.h\
    qdl_p.h\
    qdlsourceselector_p.h\
    qdlwidgetclient_p.h\
    qds_p.h\
    qdsserviceinfo_p.h\
    qdsactionrequest_p.h\
    themedviewinterface_p.h\
    qdsaction_p.h\
    qdsdata_p.h\
    phonedecoration_p.h\
    contextkeymanager_p.h\
    qstartmenuaccessory_p.h\
    qcopenvelope_p.h\
    dbusipccommon_p.h\
    dbusapplicationchannel_p.h\
    qterminationhandler_p.h \
    qcontent_p.h\
    qhardwareinterface_p.h\
    qworldmap_sun_p.h\
    qworldmap_stylusnorm_p.h \
    qdateparser_p.h \
    qalarmserver_p.h

QTOPIA_SOURCES+=\
    qtopialog.cpp\
    qlog.cpp\
    qmimetype.cpp\
    qdocumentselector.cpp\
    qterminationhandler.cpp\
    qtopiaapplication.cpp\
    qtranslatablesettings.cpp\
    qtimezonewidget.cpp\
    qworldmap.cpp\
    qworldmap_sun.cpp\
    qalarmserver.cpp\
    qpassworddialog.cpp\
    qtimestring.cpp\
    qstorage.cpp\
    qcategoryselector.cpp\
    qwindowdecorationinterface.cpp\
    mediarecorderplugininterface.cpp\
    qtaskbarappletplugin.cpp\
    qapplicationplugin.cpp\
    qdawg.cpp\
    inputmethodinterface.cpp\
    qpluginmanager.cpp\
    quniqueid.cpp\
    qcategorymanager.cpp\
    qwaitwidget.cpp\
    qwizard.cpp\
    qtopianamespace.cpp\
    qtimezone.cpp\
    qthumbnail.cpp\
    qimagedocumentselector.cpp\
    qimagedocumentselector_p.cpp\
    thumbnailview_p.cpp\
    singleview_p.cpp\
    qactionconfirm.cpp\
    qtopiasql.cpp\
    qiconselector.cpp\
    qtopiaresource.cpp\
    qsoundcontrol.cpp\
    qpedecoration_qws.cpp\
    qtopiaipcenvelope.cpp\
    qtopiaabstractservice.cpp\
    qtopiaipcadaptor.cpp\
    qtopiaipcmarshal.cpp\
    qtopiachannel.cpp\
    qtopiaservices.cpp\
    qtopiaserviceselector.cpp\
    qsignalintercepter.cpp\
    qslotinvoker.cpp\
    qpowerstatus.cpp\
    qexportedbackground.cpp\
    qcolorselector.cpp\
    qdocumentproperties.cpp\
    qstartmenuaccessory.cpp\
    qstoragedeviceselector.cpp\
    qdevicebutton.cpp\
    qdevicebuttonmanager.cpp\
    qtopiastyle.cpp\
    qpdastyle.cpp\
    qdl.cpp\
    qdlbrowserclient.cpp\
    qdlclient.cpp\
    qdleditclient.cpp\
    qdllink.cpp\
    qdlsourceselector.cpp\
    qdlwidgetclient.cpp\
    qdsaction.cpp\
    qdsservices.cpp\
    qdsserviceinfo.cpp\
    qdsactionrequest.cpp\
    qdsdata.cpp\
    qtopianetwork.cpp\
    qtopianetworkinterfaceimpl.cpp\
    qtopiafeatures.cpp\
    qcopenvelope.cpp\
    qdeviceindicators.cpp\
    dbusipccommon_p.cpp \
    dbusapplicationchannel_p.cpp \
    qphoneprofile.cpp\
    qcontent_p.cpp\
    qabstractipcinterface.cpp\
    qabstractipcinterfacegroup.cpp\
    qabstractipcinterfacegroupmanager.cpp\
    qhardwareinterface.cpp\
    qhardwaremanager.cpp\
    qbatteryaccessory.cpp\
    qbootsourceaccessory.cpp\
    qhandsfreeaccessory.cpp\
    qhandsetaccessory.cpp\
    qheadsetaccessory.cpp\
    qinfraredaccessory.cpp\
    qserialcommaccessory.cpp\
    qvibrateaccessory.cpp\
    qspeakerphoneaccessory.cpp\
    qkeypadlightaccessory.cpp\
    qtopiasendvia.cpp\
    qanalogclock.cpp\
    qdateparser.cpp\
    qsignalsource.cpp

phone:QTOPIA_SOURCES+=\
    themedview.cpp\
    themedviewinterface.cpp

# Some code is in train for removal to the libQtExtensions library.
# If that library is being built, do not include them in libQtopia
enable_qtopiabase {
    cachedinfo($$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase/qtopiabase.pro,QTOPIABASE_HEADERS,QTOPIABASE_PRIVATE_HEADERS,QTOPIABASE_SOURCES)
    QTOPIA_HEADERS-=$$cache.QTOPIABASE_HEADERS
    QTOPIA_PRIVATE_HEADERS-=$$cache.QTOPIABASE_PRIVATE_HEADERS
    QTOPIA_SOURCES-=$$cache.QTOPIABASE_SOURCES
}


TRANSLATABLES+=themedview.cpp themedviewinterface.cpp

DOCAPI_HEADERS=\
    qcontent.h\
    qcontentset.h\
    qdrmcontent.h\
    qcontentplugin.h \
    qcontentfilter.h \
    qdrmcontentplugin.h \
    qdrmrights.h \
    qcontentfiltermodel.h \
    qcontentfilterselector.h

DOCAPI_PRIVATE_HEADERS=\
    contentlnksql_p.h\
    contentlnk_p.h\
    contentserverinterface_p.h\
    drmcontent_p.h \
    contentpluginmanager_p.h \
    qcontentfilterselector_p.h

DOCAPI_SOURCES=\
    qcontent.cpp\
    qcontentset.cpp\
    contentlnk_p.cpp\
    contentlnksql_p.cpp\
    contentserverinterface_p.cpp\
    qdrmcontent.cpp\
    drmcontent_p.cpp \
    qcontentplugin.cpp \
    contentpluginmanager_p.cpp \
    qcontentfilter.cpp \
    qdrmcontentplugin.cpp \
    qdrmrights.cpp \
    qcontentfiltermodel.cpp \
    qcontentfilterselector.cpp

QTOPIA_HEADERS+=$$DOCAPI_HEADERS
QTOPIA_PRIVATE_HEADERS+=$$DOCAPI_PRIVATE_HEADERS
QTOPIA_SOURCES+=$$DOCAPI_SOURCES

!enable_qtopiabase {
    INCLUDEPATH+=$$QPEDIR/src/libraries/qtopiabase
    VPATH+=$$QPEDIR/src/libraries/qtopiabase

    INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase
    VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase

    #These are being marked for future removal.
    #dealing with these should save 200k of the
    #stripped lib size.
    # contact - used by passwd to print contact info
    # event - used by datebookdb
    # datebookdb - used by settime to reset alarms
    # palmtoprecord - used by contact && event

    win32 { 
        QTOPIA_SOURCES+=qmemoryfile_win.cpp
        DEFINES+=\
            QTOPIA_INTERNAL_APPLNKASSIGN QTOPIA_INTERNAL_FSLP\
            QTOPIA_INTERNAL_PRELOADACCESS QTOPIA_INTERNAL_FD\
            QTOPIA_WIN32PROCESS_SUPPORT
    }

    TRANSLATABLES+=qmemoryfile_win.cpp

    unix {
        QTOPIA_SOURCES+=qmemoryfile_unix.cpp
        LIBS          +=-lm
        #!staticlib:LIBS+=$$QMAKE_LIBS_DYNLOAD
    }

    TRANSLATABLES+=qmemoryfile_unix.cpp

    QTOPIA_CUSTOM=$$QPEDIR/src/libraries/qtopia/custom-qtopia
    QTOPIA_HEADERS+=$${QTOPIA_CUSTOM}.h
    QTOPIA_SOURCES+=$${QTOPIA_CUSTOM}.cpp
}

phone {
    QTOPIA_SOURCES+=\
        phonedecoration.cpp\
        qphonestyle.cpp\
	ezxphonestyle.cpp\
        contextkeymanager.cpp\
        qsoftmenubar.cpp\
        qspeeddial.cpp
}

media {
    QTOPIA_SOURCES += contextkeymanager.cpp \
		qsoftmenubar.cpp
}

TRANSLATABLES+=\
        phonedecoration.cpp\
        qphonestyle.cpp\
	ezxphonestyle.cpp\
        contextkeymanager.cpp\
        qsoftmenubar.cpp\
        qspeeddial.cpp

depends(3rdparty/libraries/zlib)
depends(3rdparty/libraries/md5)
!enable_qtopiabase:depends(libraries/qtopiail)

RESOURCES=qtopia.qrc

PREFIX=QTOPIA
resolve_include()

CONFIG += qtopia_visibility

# We need to prevent some files from appearing in TRANSLATABLES
TRANSLATABLES*=$$FORMS $$HEADERS $$SOURCES
TRANSLATABLES-=$${QTOPIA_CUSTOM}.h $${QTOPIA_CUSTOM}.cpp
CONFIG+=no_auto_translatables

depends(tools/pngscale)
depends(tools/qt/lupdate)
depends(tools/qt/lrelease)
enable_sxe:depends(libraries/qtopiasecurity)
!enable_qtopiabase:enable_dbusipc:depends(3rdparty/libraries/qtdbus)
depends(3rdparty/libraries/sqlite)

sdk_qtopia_headers.files=$${QTOPIA_HEADERS}
sdk_qtopia_headers.path=/include/qtopia
sdk_qtopia_headers.hint=sdk headers

sdk_qtopia_private_headers.files=$${QTOPIA_PRIVATE_HEADERS}
sdk_qtopia_private_headers.path=/include/qtopia/private
sdk_qtopia_private_headers.hint=sdk headers

INSTALLS+=\
    sdk_qtopia_headers sdk_qtopia_private_headers

!enable_qtopiabase {
    sdk_qtopia_custom_headers.files=custom-*-*.h
    sdk_qtopia_custom_headers.path=/src/libraries/qtopia
    sdk_qtopia_custom_headers.hint=sdk
    INSTALLS+=sdk_qtopia_custom_headers
}

etc.files =\
    $$QTOPIA_DEPOT_PATH/etc/colors\
    $$QTOPIA_DEPOT_PATH/etc/mime.types
etc.path=/etc
bins.files=\
    $$QPEDIR/bin/qtopia-addmimetype
bins.path=/bin
bins.hint=script
!phone {
    quitdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/quit.desktop
    quitdesktop.path=/apps/Settings
    quitdesktop.hint=desktop
    INSTALLS+=quitdesktop
}

pics.files=$$QTOPIA_DEPOT_PATH/pics/icons
pics.path=/pics
pics.hint=pics
INSTALLS+=pics

drmpics.files=$$QTOPIA_DEPOT_PATH/pics/drm/*
drmpics.path=/pics/drm
drmpics.hint=pics
INSTALLS+=drmpics

# WorldTime conf is used by qtimezonewidget.
defaults.files=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/presstick.conf\
               $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/SpeedDial.conf\
               $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/WorldTime.conf\
               $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/Log.conf
defaults.path=/etc/default/Trolltech
INSTALLS+=\
    etc bins defaults

!enable_singleexec {
    textcodecs.files=$$QTOPIA_DEPOT_PATH/plugins/textcodecs/.directory
    textcodecs.path=/plugins/textcodecs/
    INSTALLS+=textcodecs

    imagecodecs.files=$$QTOPIA_DEPOT_PATH/plugins/imagecodecs/.directory
    imagecodecs.path=/plugins/imagecodecs/
    INSTALLS+=imagecodecs

    decorations.files=$$QTOPIA_DEPOT_PATH/plugins/decorations/.directory
    decorations.path=/plugins/decorations/
    INSTALLS+=decorations

    styles.files=$$QTOPIA_DEPOT_PATH/plugins/styles/.directory
    styles.path=/plugins/styles/
    INSTALLS+=styles
}

!isEmpty(STORAGE_CONF_FILE) {
    storage_conf.files=$$STORAGE_CONF_FILE
    storage_conf.path=/etc/default/Trolltech
    INSTALLS+=storage_conf
}
!isEmpty(MOTD_HTML_FILE) {
    motd_html.files=$$MOTD_HTML_FILE
    motd_html.path=/etc
    INSTALLS+=motd_html
}
zonetab.files=$$QTOPIA_DEPOT_PATH/etc/zoneinfo/zone.tab
zonetab.trtarget=timezone
# don't automatically install this
zonetab.CONFIG=no_path no_default_install
# We're really only interested in the translations for zone.tab
zonetab.hint=nct
INSTALLS+=zonetab

# the server does this for us
#categories.files=$$QTOPIA_DEPOT_PATH/etc/categories/qtopia.conf
#categories.trtarget=QtopiaCategories
#categories.hint=nct
#INSTALLS+=categories

# FIXME THIS IS EVIL!!!!!
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/server

qtopiatest {
    depends(libraries/qtopiatest/qtesttools/target)
    depends(libraries/qtopiatest/qsystemtestslave)
    depends(libraries/qtopiatest/qtopiasystemtestslave)
}

pkg.desc =Base Qtopia library.
pkg.domain=base

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
run_qdoc()

# TODO have this lib include libqtopia1 rather than requiring the projects to do it themselves
#
# equals(QTOPIA_TARGET,4.1) {
#     depend(libraries/qtopia1)
#     dep(DEPENDS-=libraries/qtopia)
# }
#
# equals(QTOPIA_TARGET,4.0)|equals(MULTI_LIB_QTOPIA,1) {
#     idep(LIBS+=-l$$TARGET)
# }
#
# then in libraries/qtopia1 have something like this
#
# depend(libraries/qtopia)
# dep(MULTI_LIB_QTOPIA=1)
