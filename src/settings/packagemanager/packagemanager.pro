!qbuild {
qtopia_project(qtopia app)
TARGET=packagemanager
CONFIG+=qtopia_main
depends(3rdparty/libraries/tar)
depends(3rdparty/libraries/md5)
enable_sxe:depends(libraries/qtopiasecurity)
INCLUDEPATH+=$$QT_DEPOT_PATH/src/3rdparty/md5
}

# Give us a direct connection to the document system
DEFINES+=QTOPIA_DIRECT_DOCUMENT_SYSTEM_CONNECTION

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
                    packageversion.h \
                    utils.h

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
                    packageversion.cpp \
                    utils.cpp

enable_sxe {
    SOURCES+=domaininfo.cpp
    HEADERS+=domaininfo.h
}

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=packagemanager*
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/packagemanager.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/packagemanager/*
pics.path=/pics/packagemanager
pics.hint=pics
INSTALLS+=pics
            
secsettings.files=$$device_overrides(/etc/default/Trolltech/PackageManager.conf)
secsettings.path=/etc/default/Trolltech
INSTALLS+=secsettings
packagemanagerservice.files=$$QTOPIA_DEPOT_PATH/services/PackageManager/packagemanager
packagemanagerservice.path=/services/PackageManager
INSTALLS+=packagemanagerservice
qdspackagemanagerservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/PackageManager
qdspackagemanagerservice.path=/etc/qds
INSTALLS+=qdspackagemanagerservice

packages_category.files=$$QTOPIA_DEPOT_PATH/apps/Packages/.directory
packages_category.path=/apps/Packages
packages_category.hint=desktop prep_db
MODULES*=qtopia::prep_db
INSTALLS+=packages_category

pkg.desc=Safely download and install programs for Qtopia.
pkg.domain=trusted
