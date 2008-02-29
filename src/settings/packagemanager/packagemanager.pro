qtopia_project(qtopia app)
TARGET=packagemanager

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
                    sandboxinstall.h

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
                    sandboxinstall.cpp

depends(3rdparty/libraries/tar)
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
secsettings.files=$$QTOPIA_DEPOT_PATH/etc/default/Trolltech/PackageServers.conf
secsettings.path=/etc/default/Trolltech
secsettings.hint=secsettings
INSTALLS+=content_installer
INSTALLS+=help desktop pics secsettings

enable_sxe {
    SOURCES+=domaininfo.cpp
    HEADERS+=domaininfo.h
}

pkg.desc=Safely download and install programs for Qtopia.
pkg.domain=window,cardreader,docapi
