qtopia_project(qtopia app)
TARGET=packagemanager
CONFIG+=qtopia_main

FORMS           = serveredit.ui \
                    packagedetails.ui
HEADERS         = packageview.h \
                    packagemodel.h \
                    packagecontroller.h \
                    packageinformationreader.h \
                    installcontrol.h \
                    serveredit.h \
                    httpfetcher.h \
                    installedpackagescanner.h\
                    targz.h \
                    sandboxinstall.h \
                    md5file.h \
                    packagemanagerservice.h \
                    version.h

SOURCES         = main.cpp \
                    packageview.cpp \
                    packagemodel.cpp \
                    packagecontroller.cpp \
                    packageinformationreader.cpp \
                    installcontrol.cpp \
                    serveredit.cpp \
                    httpfetcher.cpp \
                    installedpackagescanner.cpp\
                    targz.cpp \
                    sandboxinstall.cpp \
                    md5file.cpp \
                    packagemanagerservice.cpp \
                    version.cpp

INCLUDEPATH+=$$QT_DEPOT_PATH/src/3rdparty/md5

depends(3rdparty/libraries/tar)
depends(3rdparty/libraries/md5)
enable_sxe:depends(libraries/qtopiasecurity)

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=packagemanager*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/PackageManager.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/packagemanager/*
pics.path=/pics/packagemanager
pics.hint=pics
            
!isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/etc/default/Trolltech/PackageServers.conf) {
    secsettings.files+=$$DEVICE_CONFIG_PATH/etc/default/Trolltech/PackageServers.conf
} else {
    secsettings.files+=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/PackageServers.conf
}
secsettings.path=/etc/default/Trolltech
secsettings.hint=secsettings

packagemanagerservice.files=$$QTOPIA_DEPOT_PATH/services/PackageManager/packagemanager
packagemanagerservice.path=/services/PackageManager
qdspackagemanagerservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/PackageManager
qdspackagemanagerservice.path=/etc/qds
INSTALLS+=packagemanagerservice qdspackagemanagerservice
INSTALLS+=help desktop pics secsettings

enable_sxe {
    SOURCES+=domaininfo.cpp
    HEADERS+=domaininfo.h
}

pkg.desc=Safely download and install programs for Qtopia.
pkg.domain=window,cardreader,docapi,policy,launcher,prefix
