CONFIG += qtopiaapp

HEADERS += pensettingswidget.h mainwindow.h charsetedit.h uniselect.h

INTERFACES += gprefbase.ui

INTERFACES += charseteditbase.ui

SOURCES += pensettingswidget.cpp main.cpp mainwindow.cpp charsetedit.cpp uniselect.cpp

TARGET = hwsettings

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

LIBS += -lqmstroke

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Handwriting.desktop
desktop.path=/apps/Settings
pics.files=$${QTOPIA_DEPOT_PATH}/pics/hwsettings/*.png
pics.path=/pics/hwsettings
help.files=$${QTOPIA_DEPOT_PATH}/help/html/hwsettings*.html
help.path=/help/html

i18n.path=$${INSTALL_PREFIX}/i18n
i18n.commands=$${COMMAND_HEADER}\
    [ -z "$$TRANSLATIONS" ] || for lang in $$TRANSLATIONS; do\
	for pkg in QtopiaHandwriting; do\
	    $${DQTDIR}/bin/lrelease $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/\$$pkg.ts \
		-qm $(INSTALL_ROOT)/i18n/\$$lang/\$$pkg.qm; \
	done; \
    done
 
INSTALLS+= desktop pics help
!isEmpty(DQTDIR):INSTALLS+=i18n
