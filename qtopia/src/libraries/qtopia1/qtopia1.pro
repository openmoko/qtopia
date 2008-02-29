# nocompat means this file is included from src/libraries/qtopia.pro
!nocompat {
    TEMPLATE	= lib

    TARGET		= qtopia
    DESTDIR		= $$(QPEDIR)/lib

    CONFIG		+= qtopiainc
    VERSION             = 1.6.3
    LIBS                += -lqpe
}

include(qtopia1.pri)

QTOPIA1_HEADERS += services.h \
	devicebuttonmanager.h \
	devicebutton.h \
	qwizard.h \
	locationcombo.h \
	docproperties.h \
	pluginloader.h

QTOPIA1_SOURCES += services.cpp \
	devicebuttonmanager.cpp \
	devicebutton.cpp \
	fileselector1.cpp \
	qwizard.cpp \
	locationcombo.cpp \
	docproperties.cpp \
	pluginloader.cpp \
	pluginloaderlib.cpp 


contains(QTE_MAJOR_VERSION,2) {
    QTOPIA1_SOURCES+=quuid1.cpp qprocess.cpp
	QTOPIA1_HEADERS+=qprocess.h
	unix { 
	    SOURCES += qprocess_unix.cpp
	}
    win32 { 
	QTOPIA1_SOURCES += qprocess_win.cpp
	    LIBS += rpcrt4.lib
    }
}
TRANSLATABLES   += quuid1.cpp \
                    qprocess.cpp \
                    qprocess_win.cpp \
                    qprocess.h \
                    qprocess_unix.cpp

HEADERS += $${QTOPIA1_HEADERS} $$QTOPIA1_PRIVATE_HEADERS
SOURCES += $${QTOPIA1_SOURCES}
TRANSLATABLES*=$$INTERFACES $$HEADERS $$SOURCES

sdk_qtopia1_headers.files=$${QTOPIA1_HEADERS}
sdk_qtopia1_headers.path=/include/qtopia
sdk_qtopia1_headers.CONFIG += no_default_install

sdk_qtopia1_private_headers.files=$${QTOPIA1_PRIVATE_HEADERS}
sdk_qtopia1_private_headers.path=/include/qtopia/private
sdk_qtopia1_private_headers.CONFIG += no_default_install

devsdk_qtopia1_sources.files=$${QTOPIA1_SOURCES} $${sdk_qtopia1_headers.files} $$sdk_qtopia1_private_headers.files
devsdk_qtopia1_sources.path=/src/libraries/qtopia1
devsdk_qtopia1_sources.CONFIG += no_default_install

INSTALLS+=sdk_qtopia1_headers sdk_qtopia1_private_headers devsdk_qtopia1_sources 
sdk.depends+=install_sdk_qtopia1_headers install_sdk_qtopia1_private_headers
devsdk.depends+=install_devsdk_qtopia1_sources

NON_CODE_TRANSLATABLES=$${QTOPIA_DEPOT_PATH}/etc/zoneinfo/zone.tab
NON_CODE_TRTARGETS=timezone

!nocompat {
    PACKAGE_NAME = qpe-libqtopia
    PACKAGE_DESCRIPTION = Second Qtopia Library.
    PACKAGE_DEPENDS = qpe-base
}

