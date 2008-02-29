TEMPLATE	= lib

TARGET		= qpepim
DESTDIR		= $$(QPEDIR)/lib

CONFIG		+= qtopia
VERSION         = 1.6.3

include(qtopiapim.pri)

nocompat {
    include($$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim1/qtopiapim1.pro)
}

HEADERS+=$$QTOPIAPIM_HEADERS $$QTOPIAPIM_PRIVATE_HEADERS
SOURCES+=$$QTOPIAPIM_SOURCES
TRANSLATABLES*=$$INTERFACES $$HEADERS $$SOURCES

sdk_qtopiapim_headers.files=$${QTOPIAPIM_HEADERS}
sdk_qtopiapim_headers.path=/include/qtopia/pim
sdk_qtopiapim_headers.CONFIG+=no_default_install

sdk_qtopiapim_private_headers.files=$${QTOPIAPIM_PRIVATE_HEADERS}
sdk_qtopiapim_private_headers.path=/include/qtopia/pim/private
sdk_qtopiapim_private_headers.CONFIG+=no_default_install

devsdk_qtopiapim_sources.files=$${QTOPIAPIM_SOURCES} $$sdk_qtopiapim_headers.files $$sdk_qtopiapim_private_headers.files
devsdk_qtopiapim_sources.path=/src/libraries/qtopiapim
devsdk_qtopiapim_sources.CONFIG+=no_default_install

INSTALLS+=sdk_qtopiapim_headers sdk_qtopiapim_private_headers devsdk_qtopiapim_sources
sdk.depends+=install_sdk_qtopiapim_headers install_sdk_qtopiapim_private_headers
devsdk.depends+=install_devsdk_qtopiapim_sources

PACKAGE_NAME = qpe-pim
PACKAGE_DESCRIPTION = PIM Data access library
PACKAGE_DEPENDS = qpe-libqtopia
