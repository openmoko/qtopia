# nocompat means this file is included from src/libraries/qtopiapim/qtopiapim.pro
!nocompat {
    TEMPLATE	= lib

    TARGET		= qpepim1
    DESTDIR		= $$(QPEDIR)/lib

    CONFIG		+= qtopiainc
    VERSION             = 2.1.0
    LIBS                += -lqpepim
}

include(qtopiapim1.pri)

HEADERS+=$$QTOPIAPIM1_HEADERS
SOURCES+=$$QTOPIAPIM1_SOURCES
TRANSLATABLES*=$$INTERFACES $$HEADERS $$SOURCES

# No headers in qtopiapim1
#sdk_qtopiapim1_headers.files=$${QTOPIAPIM1_HEADERS}
#sdk_qtopiapim1_headers.path=/include/qtopia/pim
#sdk_qtopiapim1_headers.CONFIG+=no_default_install

devsdk_qtopiapim1_sources.files=$${QTOPIAPIM1_SOURCES} $$sdk_qtopiapim1_headers.files
devsdk_qtopiapim1_sources.path=/src/libraries/qtopiapim1
devsdk_qtopiapim1_sources.CONFIG+=no_default_install

INSTALLS+=devsdk_qtopiapim1_sources
devsdk.depends+=install_devsdk_qtopiapim1_sources

!nocompat {
    PACKAGE_NAME=qpe-libqtopiapim1
    PACKAGE_DEPENDS=qpe-pim qpe-libqtopia2
    PACKAGE_DESCRIPTION = Second PIM Data access library
}

