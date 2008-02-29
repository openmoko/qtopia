CONFIG += qtopiaplugin

TARGET=conversion

# Input
HEADERS = conversion.h  conversionfactory.h
SOURCES = conversion.cpp conversionfactory.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}
LIBS   +=-lqtopiacalc

i18n.path=$${INSTALL_PREFIX}/i18n
i18n.commands=$${COMMAND_HEADER}\
    [ -z "$$TRANSLATIONS" ] || for lang in $$TRANSLATIONS; do \
	for pkg in CalcDistConv CalcAreaConv CalcWeightConv CalcVolConv; do \
	    $${DQTDIR}/bin/lrelease $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/\$$pkg.ts \
		-qm $(INSTALL_ROOT)/i18n/\$$lang/\$$pkg.qm; \
	done; \
    done
    
unitconfs.files=$${QTOPIA_DEPOT_PATH}/etc/calculator/*.conf
unitconfs.path=/etc/calculator/
INSTALLS+=unitconfs
!isEmpty(DQTDIR):INSTALLS+=i18n
