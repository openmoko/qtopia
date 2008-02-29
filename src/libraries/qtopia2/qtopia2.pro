# nocompat means this file is included from src/libraries/qtopia.pro
!nocompat {
    TEMPLATE	= lib

    TARGET		= qtopia2
    DESTDIR		= $$(QPEDIR)/lib

    CONFIG		+= qtopiainc
    VERSION             = 2.1.0
    LIBS                += -lqtopia
}

include (qtopia2.pri)

QTOPIA2_PHONE_HEADERS+=\
    speeddial.h
QTOPIA2_PHONE_SOURCES+=\
    speeddial.cpp
TRANSLATABLES*=$$QTOPIA2_PHONE_HEADERS $$QTOPIA2_PHONE_SOURCES

QTOPIA_PHONE {
    QTOPIA2_HEADERS+=$$QTOPIA2_PHONE_HEADERS
    QTOPIA2_SOURCES+=$$QTOPIA2_PHONE_SOURCES
}

QTOPIA2_HEADERS += imageselector.h \
	colorselector.h

QTOPIA2_PRIVATE_HEADERS += imageselector_p.h \
     thumbnailview_p.h \
    singleview_p.h \
    imagecollection_p.h

QTOPIA2_SOURCES += imageselector.cpp \
       imageselector_p.cpp \
       thumbnailview_p.cpp \
       singleview_p.cpp \
       imagecollection_p.cpp \
       colorselector.cpp

QTOPIA_DATA_LINKING {
    QTOPIA2_HEADERS += qdl.h
    QTOPIA2_PRIVATE_HEADERS += qdl_p.h
    QTOPIA2_SOURCES += qdl.cpp
}
TRANSLATABLES += qdl.h qdl_p.h qdl.cpp

QTOPIA_SQL:qte2:unix:LIBS += -lqtopiasql

HEADERS+=$$QTOPIA2_HEADERS $$QTOPIA2_PRIVATE_HEADERS
SOURCES+=$$QTOPIA2_SOURCES
TRANSLATABLES*=$$INTERFACES $$HEADERS $$SOURCES

sdk_qtopia2_headers.files=$${QTOPIA2_HEADERS}
sdk_qtopia2_headers.path=/include/qtopia
sdk_qtopia2_headers.CONFIG += no_default_install

sdk_qtopia2_private_headers.files=$${QTOPIA2_PRIVATE_HEADERS}
sdk_qtopia2_private_headers.path=/include/qtopia/private
sdk_qtopia2_private_headers.CONFIG += no_default_install

devsdk_qtopia2_sources.files=$${QTOPIA2_SOURCES} $${sdk_qtopia2_headers.files} $${sdk_qtopia2_private_headers.files}
devsdk_qtopia2_sources.path=/src/libraries/qtopia2
devsdk_qtopia2_sources.CONFIG += no_default_install

INSTALLS+=sdk_qtopia2_headers sdk_qtopia2_private_headers devsdk_qtopia2_sources
sdk.depends+=install_sdk_qtopia2_headers install_sdk_qtopia2_private_headers
devsdk.depends+=install_devsdk_qtopia2_sources

!nocompat {
    PACKAGE_NAME = qpe-libqtopia2
    PACKAGE_DESCRIPTION = Third Qtopia Library.
    PACKAGE_DEPENDS = qpe-libqtopia
}

