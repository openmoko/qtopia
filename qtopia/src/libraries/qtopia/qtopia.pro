TEMPLATE	= lib

TARGET		= qpe
DESTDIR		= $$(QPEDIR)/lib
VERSION		= 1.5.3

CONFIG		+= qtopiainc

UI_HEADERS_DIR=$$(QPEDIR)/include/qtopia/private

include(qtopia.pri)

nocompat {
    include($$QTOPIA_DEPOT_PATH/src/libraries/qtopia1/qtopia1.pro)
    include($$QTOPIA_DEPOT_PATH/src/libraries/qtopia2/qtopia2.pro)
}

QTOPIA_CUSTOM = $$(QPEDIR)/src/libraries/qtopia/custom-qtopia.cpp
QTOPIA_SOURCES+= $${QTOPIA_CUSTOM}

# Qt/Embedded only
QTOPIA_HEADERS += fontmanager.h \
	fontdatabase.h \
	network.h \
	networkinterface.h \
	qcopenvelope_qws.h \
	power.h \
	ir.h

QTOPIA_PRIVATE_HEADERS += qpedecoration_p.h pluginloader_p.h bgexport_p.h
QTOPIA_PHONE:QTOPIA_PRIVATE_HEADERS += themedview_p.h themedviewinterface_p.h
QTOPIA_PHONE:QTOPIA_SOURCES += themedview.cpp

# Allow decorations our XPM's to be found
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/pics/qpe/


QTOPIA_SOURCES += fontmanager.cpp \
	fontdatabase.cpp \
	qpedecoration_qws.cpp \
	network.cpp \
	networkinterface.cpp \
	qcopenvelope_qws.cpp \
	power.cpp \
	ir.cpp \
	pluginloader_p.cpp \
	bgexport.cpp
        

contains(QTE_MAJOR_VERSION,2) {
    QTOPIA_HEADERS += quuid.h \
	    qcom.h \
	    qlibrary.h \
	    process.h

    QTOPIA_PRIVATE_HEADERS += qlibrary_p.h

    QTOPIA_SOURCES += quuid.cpp \
	qlibrary.cpp \
	process.cpp 

    unix:QTOPIA_SOURCES += process_unix.cpp qlibrary_unix.cpp
    win32:QTOPIA_SOURCES += process_win.cpp qlibrary_win.cpp

    LIBS += $${QMAKE_LIBS_DYNLOAD}
}
TRANSLATABLES += quuid.h \
		qcom.h \
		qlibrary.h \
		process.h \
		qlibrary_p.h \
		quuid.cpp \
		qlibrary.cpp \
		process.cpp \
		process_unix.cpp qlibrary_unix.cpp \
		process_win.cpp qlibrary_win.cpp

QTOPIA_PHONE {
    QTOPIA_HEADERS += phonestyle.h \
		contextbar.h \
		contextmenu.h

    QTOPIA_PRIVATE_HEADERS += phonedecoration_p.h contextkeymanager_p.h

    QTOPIA_SOURCES += phonedecoration.cpp \
		phonestyle.cpp \
		contextkeymanager.cpp \
		contextbar.cpp \
		contextmenu.cpp
}
TRANSLATABLES += phonestyle.h \
		contextbar.h \
		contextmenu.h \
                phonedecoration_p.h \
                contextkeymanager_p.h \
                phonedecoration.cpp \
		phonestyle.cpp \
		contextkeymanager.cpp \
		contextbar.cpp \
		contextmenu.cpp

HEADERS+=$${QTOPIA_HEADERS} $${QTOPIA_PRIVATE_HEADERS}\
    $${BACKEND_PRIVATE_HEADERS}
SOURCES+=$${QTOPIA_SOURCES} $${BACKEND_SOURCES}
INTERFACES+=$${QTOPIA_INTERFACES}
TRANSLATABLES*=$$INTERFACES $$HEADERS $$SOURCES
TRANSLATABLES-= $${QTOPIA_CUSTOM}

lupdate.command_override=\
    TRANSFILES=;\
    [ -z "$(TRANSLATABLES)" ] || for transfile in $(TRANSLATABLES); do\
        [ -f \$$transfile ] && TRANSFILES="\$$TRANSFILES \$$transfile";\
    done;\
    TRANSLATIONS="$$TRANSLATIONS"; for lang in \$$TRANSLATIONS; do\
	$${DQTDIR}/bin/lupdate \$$TRANSFILES $${QTOPIA_CUSTOM} -ts libqpe-\$$lang.ts;\
    done

sdk_qtopia_headers.files=$${QTOPIA_HEADERS} custom*.h
sdk_qtopia_headers.path=/include/qtopia
sdk_qtopia_headers.CONFIG += no_default_install

sdk_qtopia_private_headers.files=$${QTOPIA_PRIVATE_HEADERS}
sdk_qtopia_private_headers.path=/include/qtopia/private
sdk_qtopia_private_headers.CONFIG += no_default_install

devsdk_qtopia_sources.files=$${QTOPIA_SOURCES} custom*.cpp $$sdk_qtopia_headers.files $$sdk_qtopia_private_headers.files
devsdk_qtopia_sources.path=/src/libraries/qtopia
devsdk_qtopia_sources.CONFIG += no_default_install

sdk_qtopia_backend_headers.files=$${BACKEND_PRIVATE_HEADERS}
sdk_qtopia_backend_headers.path=/include/qtopia/private
sdk_qtopia_backend_headers.CONFIG += no_default_install

devsdk_qtopia_backend_sources.files=$${BACKEND_SOURCES} $$sdk_qtopia_backend_headers.files
devsdk_qtopia_backend_sources.path=/src/libraries/qtopia/backend
devsdk_qtopia_backend_sources.CONFIG += no_default_install

sdk_qtopia_compat_link.commands=ln -s qtopia $(INSTALL_ROOT)/include/qpe
sdk_qtopia_compat_link.CONFIG += no_path no_default_install

INSTALLS+=sdk_qtopia_headers sdk_qtopia_private_headers devsdk_qtopia_sources\
    sdk_qtopia_backend_headers devsdk_qtopia_backend_sources\
    sdk_qtopia_compat_link

# TODO this could be specialised for just the generated files that are required
sdk_uicdecls.depends=$(UICDECLS)
sdk_uicdecls.path=/include/qtopia/private
sdk_uicdecls.files=$(UICDECLS)
QMAKE_EXTRA_UNIX_TARGETS+=sdk_uicdecls

devsdk_uicimpls.depends=$(UICIMPLS)
devsdk_uicimpls.path=/src/libraries/qtopia/$${UI_SOURCES_DIR}
devsdk_uicimpls.files=$(UICIMPLS)
QMAKE_EXTRA_UNIX_TARGETS+=devsdk_uicimpls

sdk.depends+=install_sdk_qtopia_headers install_sdk_qtopia_private_headers\
    install_sdk_qtopia_backend_headers\
    sdk_uicdecls install_sdk_qtopia_compat_link
devsdk.depends+=install_devsdk_qtopia_sources install_devsdk_qtopia_backend_sources devsdk_uicimpls

etc.files =\
$${QTOPIA_DEPOT_PATH}/etc/colors\
$${QTOPIA_DEPOT_PATH}/etc/mime.types
etc.path=/etc
bins.files=$${QTOPIA_DEPOT_PATH}/bin/qpe-reorgfiles\
$${QTOPIA_DEPOT_PATH}/bin/qtopia-addmimetype
bins.path=/bin
settingsdirectory.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/.directory
!QTOPIA_PHONE:settingsdirectory.files+=$${QTOPIA_DEPOT_PATH}/apps/Settings/quit.desktop
settingsdirectory.path=/apps/Settings
gamesdirectory.files=$${QTOPIA_DEPOT_PATH}/apps/Games/.directory
gamesdirectory.path=/apps/Games
applicationsdirectory.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/.directory
applicationsdirectory.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/icons
pics.path=/pics
presstick.files=$${QTOPIA_DEPOT_PATH}/etc/default/presstick.conf
presstick.path=/etc/default
textcodecs.files=$${QTOPIA_DEPOT_PATH}/plugins/textcodecs/.directory
textcodecs.path=/plugins/textcodecs/
imagecodecs.files=$${QTOPIA_DEPOT_PATH}/plugins/imagecodecs/.directory
imagecodecs.path=/plugins/imagecodecs/
decorations.files=$${QTOPIA_DEPOT_PATH}/plugins/decorations/.directory
decorations.path=/plugins/decorations/
styles.files=$${QTOPIA_DEPOT_PATH}/plugins/styles/.directory
styles.path=/plugins/styles/
fontfactories.files=$${QTOPIA_DEPOT_PATH}/plugins/fontfactories/.directory
fontfactories.path=/plugins/fontfactories/
INSTALLS+=etc bins settingsdirectory gamesdirectory applicationsdirectory \
	    presstick textcodecs imagecodecs decorations styles fontfactories
PICS_INSTALLS+=pics

PACKAGE_NAME = qpe-base
PACKAGE_DESCRIPTION = Base Qtopia environment.
PACKAGE_DEPENDS=qpe-taskbar qpe-sounds atd
!buildSingleexec:PACKAGE_DEPENDS+=qt-embedded (>=$${QTE_VERSION}) qpe-qcop

