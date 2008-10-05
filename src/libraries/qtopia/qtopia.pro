!qbuild {
qtopia_project(qtopia core lib)
TARGET=qtopia
VERSION=4.0.0
UI_HEADERS_DIR=$$QPEDIR/include/qtopia/private

DEFINES+=QTOPIA_PAGE_SIZE=$$LITERAL_SQUOTE$$QTOPIA_PAGE_SIZE$$LITERAL_SQUOTE
DEFINES+=QTOPIA_PAGE_MASK=$$LITERAL_SQUOTE$$QTOPIA_PAGE_MASK$$LITERAL_SQUOTE

depends(3rdparty/libraries/zlib)
depends(3rdparty/libraries/md5)
depends(3rdparty/libraries/sqlite)
depends(libraries/qtopiagfx)

CONFIG += qtopia_visibility

enable_sxe:depends(libraries/qtopiasecurity)
}

# Needed for includes from qtextengine_p.h
INCLUDEPATH+=$$QT_DEPOT_PATH/src/3rdparty/harfbuzz/src

FORMS+=passwordbase_p.ui

HEADERS+=\
    qmimetype.h\
    qdocumentselector.h\
    qtopiaapplication.h\
    qterminationhandler.h\
    qtimezoneselector.h\
    qworldmapdialog.h\
    qworldmap.h\
    qpassworddialog.h\
    qtimestring.h\
    qcategorymanager.h\
    qcategoryselector.h\
    qwindowdecorationinterface.h\
    mediarecorderplugininterface.h\
    inputmethodinterface.h\
    qapplicationplugin.h\
    qpluginmanager.h\
    qwaitwidget.h\
    qtimezone.h\
    qthumbnail.h\
    qimagedocumentselector.h\
    qtopiasql.h\
    qiconselector.h\
    inheritedshareddata.h\
    qtopiaserviceselector.h\
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
    qtopianetworkinterface.h\
    qtopiastyle.h\
    qphonestyle.h\
    qsoftmenubar.h\
    qspeeddial.h\
    qfavoriteservicesmodel.h\
    qstoragedeviceselector.h\
    qconstcstring.h\
    qtopiafeatures.h\
    qdeviceindicators.h\
    themedview.h\
    qphoneprofile.h\
    qhardwareinterface.h\
    qhardwaremanager.h\
    qpowersource.h\
    qbootsourceaccessory.h\
    qvibrateaccessory.h\
    qkeypadlightaccessory.h\
    qscreeninformation.h\
    qtopiasendvia.h\
    qsignalsource.h \
    qanalogclock.h \
    qcontentsortcriteria.h \
    qsoftkeylabelhelper.h \
    qtextentryproxy.h \
    qtopiadocumentserver.h \
    qaudiosourceselector.h \
    qimagesourceselector.h \
    qcontent.h \
    qcontentset.h \
    qdrmcontent.h \
    qcontentplugin.h \
    qcontentfilter.h \
    qdrmcontentplugin.h \
    qdrmrights.h \
    qcontentfiltermodel.h \
    qcontentfilterselector.h\
    qperformancelog.h \
    qdocumentselectorservice.h \
    qtopiaitemdelegate.h\
    qtopiaservicehistorymodel.h\
    qexifimageheader.h \
    servertaskplugin.h\
    qsmoothlist.h\
    qcontentproperties.h\

PRIVATE_HEADERS+=\
    ezxphonestyle_p.h\
    qimagedocumentselector_p.h\
    thumbnailview_p.h\
    singleview_p.h\
    qtopiaresource_p.h\
    qdl_p.h\
    qdlsourceselector_p.h\
    qdlwidgetclient_p.h\
    qds_p.h\
    qdsserviceinfo_p.h\
    qdsactionrequest_p.h\
    qdsaction_p.h\
    qdsdata_p.h\
    qhardwareinterface_p.h\
    qworldmap_sun_p.h\
    qworldmap_stylusnorm_p.h \
    qtopiasql_p.h \
    qcontentengine_p.h \
    qcontentstore_p.h \
    qcontentsetengine_p.h \
    qfscontentengine_p.h \
    qsqlcontentsetengine_p.h \
    qdrmcontentengine_p.h \
    qcategorystore_p.h \
    qsqlcategorystore_p.h \
    qmimetypedata_p.h \
    qdocumentserverchannel_p.h \
    qdocumentservercategorystore_p.h \
    qcategorystoreserver_p.h \
    qdocumentservercontentstore_p.h \
    qdocumentservercontentsetengine_p.h \
    qcontentstoreserver_p.h \
    qdocumentservercontentengine_p.h \
    qsqlcontentstore_p.h \
    qsoftkeylabelhelper_p.h \
    resourcesourceselector_p.h \
    drmcontent_p.h \
    qdocumentselectorsocketserver_p.h \
    qcontentfilterselector_p.h \
    qsparselist_p.h \
    qtopiamessagehandler_p.h \
    qtopiaservicehistorymodel_p.h\
    qtagmap_p.h \
    keyboard_p.h

SEMI_PRIVATE_HEADERS+=\
    qterminationhandler_p.h\
    themedviewinterface_p.h\
    qcontent_p.h\
    contextkeymanager_p.h\
    contentpluginmanager_p.h \
    qsmoothlistwidget_p.h\
    qthumbstyle_p.h\
    pred_p.h\
    qtopiainputdialog_p.h\
    qtopiasqlmigrateplugin_p.h

SOURCES+=\
    qmimetype.cpp\
    qdocumentselector.cpp\
    qterminationhandler.cpp\
    qtopiaapplication.cpp\
    qtimezoneselector.cpp\
    qworldmapdialog.cpp\
    qworldmap.cpp\
    qworldmap_sun.cpp\
    qpassworddialog.cpp\
    qtimestring.cpp\
    qcategoryselector.cpp\
    qwindowdecorationinterface.cpp\
    mediarecorderplugininterface.cpp\
    qapplicationplugin.cpp\
    inputmethodinterface.cpp\
    qpluginmanager.cpp\
    qcategorymanager.cpp\
    qwaitwidget.cpp\
    qtimezone.cpp\
    qthumbnail.cpp\
    qimagedocumentselector.cpp\
    qimagedocumentselector_p.cpp\
    thumbnailview_p.cpp\
    singleview_p.cpp\
    qtopiasql.cpp\
    qiconselector.cpp\
    qtopiaresource.cpp\
    qtopiaserviceselector.cpp\
    qtopiaservicehistorymodel.cpp\
    qtopiaservicehistorymodel_p.cpp\
    qcolorselector.cpp\
    qdocumentproperties.cpp\
    qstoragedeviceselector.cpp\
    qdevicebutton.cpp\
    qdevicebuttonmanager.cpp\
    qtopiastyle.cpp\
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
    qtopiamessagehandler.cpp\
    qtopianetwork.cpp\
    qtopianetworkinterfaceimpl.cpp\
    qtopiafeatures.cpp\
    qdeviceindicators.cpp\
    qphoneprofile.cpp\
    qcontent_p.cpp\
    qhardwareinterface.cpp\
    qhardwaremanager.cpp\
    qpowersource.cpp\
    qbootsourceaccessory.cpp\
    qvibrateaccessory.cpp\
    qkeypadlightaccessory.cpp\
    qscreeninformation.cpp\
    qtopiasendvia.cpp\
    qanalogclock.cpp\
    qsignalsource.cpp \
    qtopiasql_p.cpp\
    qcontentengine.cpp \
    qcontentstore.cpp \
    qsqlcontentstore.cpp \
    qcontentsetengine.cpp \
    qsqlcontentsetengine.cpp \
    qdrmcontentengine.cpp \
    qcategorystore.cpp \
    qsqlcategorystore.cpp \
    qmimetypedata.cpp \
    qdocumentserverchannel.cpp \
    qdocumentservercategorystore.cpp \
    qcategorystoreserver.cpp \
    qdocumentservercontentstore.cpp \
    qdocumentservercontentsetengine.cpp \
    qcontentstoreserver.cpp \
    qdocumentservercontentengine.cpp \
    qcontentsortcriteria.cpp \
    qfscontentengine.cpp \
    qsoftkeylabelhelper.cpp \
    qtextentryproxy.cpp \
    qtopiadocumentserver.cpp \
    qaudiosourceselector.cpp \
    qimagesourceselector.cpp \
    resourcesourceselector.cpp \
    themedview.cpp \
    themedviewinterface.cpp \
    qcontent.cpp \
    qcontentset.cpp \
    qdrmcontent.cpp \
    drmcontent_p.cpp \
    qcontentplugin.cpp \
    contentpluginmanager_p.cpp \
    qcontentfilter.cpp \
    qdrmcontentplugin.cpp \
    qdrmrights.cpp \
    qcontentfiltermodel.cpp \
    qcontentfilterselector.cpp \
    qcontentproperties.cpp \
    contextkeymanager.cpp \
    qsoftmenubar.cpp \
    qphonestyle.cpp\
    ezxphonestyle.cpp\
    qspeeddial.cpp\
    qfavoriteservicesmodel.cpp\
    qperformancelog.cpp \
    qdocumentselectorservice.cpp \
    qdocumentselectorsocketserver.cpp \
    qtopiaitemdelegate.cpp \
    qthumbstyle_p.cpp \
    servertaskplugin.cpp \
    qexifimageheader.cpp \
    qsmoothlist.cpp \
    qsmoothlistwidget_p.cpp \
    qtopiainputdialog_p.cpp\
    pred_p.cpp\
    keyboard_p.cpp\
    qtopiasqlmigrateplugin.cpp

HEADERS+=\
    qexportedbackground.h
!x11 {
    PRIVATE_HEADERS+=\
        qpedecoration_p.h\
        phonedecoration_p.h
    SOURCES+=\
        qpedecoration_qws.cpp\
        qexportedbackground.cpp\
        phonedecoration.cpp
} else {
    SOURCES+=\
        qexportedbackground_x11.cpp
}

RESOURCES=qtopia.qrc

pkg.desc=Base Qtopia library.
pkg.domain=trusted

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia
headers.hint=sdk headers
INSTALLS+=headers
pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopia/private
pheaders.hint=sdk headers
INSTALLS+=pheaders

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)

# for clock_gettime
LIBS+=-lrt
idep(LIBS+=-lrt)
}

etc.files=$$QTOPIA_DEPOT_PATH/etc/mime.types
etc.path=/etc
INSTALLS+=etc

colors.files=$$QTOPIA_DEPOT_PATH/etc/colors/*
colors.path=/etc/colors
INSTALLS+=colors

bins.files=$$QTOPIA_DEPOT_PATH/bin/qtopia-addmimetype
bins.path=/bin
bins.hint=script
INSTALLS+=bins

equals(QTOPIA_UI,platform) {
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

worldtimepics.files=$$QTOPIA_DEPOT_PATH/pics/worldtime/*
worldtimepics.path=/pics/worldtime
worldtimepics.hint=pics
INSTALLS+=worldtimepics

# WorldTime conf is used by qtimezoneselector.
settings.files=$$device_overrides(/etc/default/Trolltech/presstick.conf)\
               $$device_overrides(/etc/default/Trolltech/SpeedDial.conf)\
               $$device_overrides(/etc/default/Trolltech/WorldTime.conf)\
               $$device_overrides(/etc/default/Trolltech/Log.conf)\
               $$device_overrides(/etc/default/Trolltech/Log2.conf)\
               $$device_overrides(/etc/default/Trolltech/Storage.conf)\
               $$device_overrides(/etc/default/Trolltech/PhoneProfile.conf)
settings.path=/etc/default/Trolltech
INSTALLS+=settings

zonetab.files=$$QTOPIA_DEPOT_PATH/etc/zoneinfo/zone.tab
zonetab.trtarget=timezone
# don't automatically install this
zonetab.CONFIG=no_path no_default_install
# We're really only interested in the translations for zone.tab
zonetab.hint=nct
INSTALLS+=zonetab

# This is here so that these get installed before any .desktop files
mainapps_category.files=$$QTOPIA_DEPOT_PATH/apps/MainApplications/.directory
mainapps_category.path=/apps/MainApplications
mainapps_category.hint=desktop prep_db

app_categories.files=$$QTOPIA_DEPOT_PATH/apps/Applications/.directory
app_categories.path=/apps/Applications
app_categories.hint=desktop prep_db
app_categories.depends=install_docapi_mainapps_category

settings_category.files=$$QTOPIA_DEPOT_PATH/apps/Settings/.directory
settings_category.path=/apps/Settings
settings_category.hint=desktop prep_db
settings_category.depends=install_docapi_mainapps_category

INSTALLS+=mainapps_category app_categories settings_category

equals(QTOPIA_UI,home) {
    deskphonepics.files=$$QTOPIA_DEPOT_PATH/pics/qpe/deskphone/*
    deskphonepics.path=/pics/deskphone
    deskphonepics.hint=pics
    INSTALLS+=deskphonepics

    deskphonelauncher_category.files=$$QTOPIA_DEPOT_PATH/apps/DeskphoneLauncher/.directory
    deskphonelauncher_category.path=/apps/DeskphoneLauncher
    deskphonelauncher_category.hint=desktop prep_db
    INSTALLS+=deskphonelauncher_category
}

!platform {
    games_category.files=$$QTOPIA_DEPOT_PATH/apps/Games/.directory
    games_category.path=/apps/Games
    games_category.hint=desktop prep_db
    games_category.depends=install_docapi_mainapps_category
    INSTALLS+=games_category
}

pkg.desc =Base Qtopia library.
pkg.domain=trusted

