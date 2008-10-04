!qbuild:qtopia_project(qtopia unittest)
TARGET=tst_Space

VPATH      +=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager

# Input
FORMS += \
    packagedetails.ui \
    serveredit.ui

HEADERS += \
    domaininfo.h \
    httpfetcher.h \
    installedpackagescanner.h \
    md5file.h \
    packageinformationreader.h \
    packagemanagerservice.h \
    packagemodel.h \
    packageversion.h \
    packageview.h \
    sandboxinstall.h \
    serveredit.h \
    targz.h \
    installcontrol.h \
    packagecontroller.h \
    utils.h


SOURCES +=  \
    domaininfo.cpp \
    httpfetcher.cpp \
    installcontrol.cpp \
    installedpackagescanner.cpp \
    md5file.cpp \
    packagecontroller.cpp \
    packageinformationreader.cpp \
    packagemanagerservice.cpp \
    packagemodel.cpp \
    packageversion.cpp \
    packageview.cpp \
    sandboxinstall.cpp \
    serveredit.cpp \
    targz.cpp \
    tst_space.cpp \
    utils.cpp

!qbuild:depends(libraries/qtopia)
!qbuild:depends(3rdparty/libraries/tar)
!qbuild:INCLUDEPATH+=$$QT_DEPOT_PATH/src/3rdparty/md5
DEFINES+=PACKAGEMANAGER_UTILS_TEST
