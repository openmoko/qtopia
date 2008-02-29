TEMPLATE=subdirs

buildQtopia {
    include($${QTOPIA_DEPOT_PATH}/src/projects.pri)
    CONFIG+=ordered
    SUBDIRS+=$${LIBRARY_PROJECTS} src-components.pro
}

buildQtopiadesktop:SUBDIRS+=qtopiadesktop

nonpropackages.commands=$${COMMAND_HEADER}\
mkdir -p $$(QPEDIR)/ipkg;\
cd $$(QPEDIR)/ipkg;\
for i in \
`find $${QTOPIA_DEPOT_PATH}/src/qt -name '*.control'` \
`find $${QTOPIA_DEPOT_PATH}/services -name '*.control'` \
`find $${QTOPIA_DEPOT_PATH}/sounds -name '*.control'` \
`find $${QTOPIA_DEPOT_PATH}/misc -name '*.control'`; do\
    echo Packaging \$$i;\
    $${QTOPIA_DEPOT_PATH}/bin/mkipks \$$i;\
done;

nonpropackages.CONFIG+=phony no_path
buildQtopia {
    QMAKE_EXTRA_UNIX_TARGETS += nonpropackages
    packages.depends = nonpropackages
}

nonpropackagesinstall.commands=$${COMMAND_HEADER}\
for i in \
    `find $${QTOPIA_DEPOT_PATH}/services -name '*.control'` \
    `find $${QTOPIA_DEPOT_PATH}/sounds -name '*.control'` \
    `find $${QTOPIA_DEPOT_PATH}/misc -name '*.control'` \
;do \
    echo Installing files listed in \$$i;\
    FILELIST=`grep "^Files:" \$$i | sed -e 's/^Files://'`;\
    [ -z "\$$FILELIST" ] || for j in \$$FILELIST; do \
	dir=`dirname \$$j`;\
	mkdir -p "$(INSTALL_ROOT)/\$$dir";\
	eval install -c "$$(QPEDIR)/\$$j" "$(INSTALL_ROOT)/\$$dir";\
    done;\
done;\

nonpropackagesinstall.CONFIG+=no_path
buildQtopia:INSTALLS+=nonpropackagesinstall

# Fonts...
contains(QTOPIA_DISP_ROTS,0):fonts.rotations=$${QTOPIA_FONT_STYLES}
contains(QTOPIA_DISP_ROTS,90):fonts.rotations+=$${QTOPIA_FONT_STYLES}_t5
contains(QTOPIA_DISP_ROTS,180):fonts.rotations+=$${QTOPIA_FONT_STYLES}_t15
contains(QTOPIA_DISP_ROTS,270):fonts.rotations+=$${QTOPIA_FONT_STYLES}_t10
buildQtopia {
    fonts.sizes=$$QTOPIA_FONT_SIZES
    fonts.families=$$QTOPIA_FONT_FAMILIES
    fonts.path=/lib/fonts
    fonts.commands=$${COMMAND_HEADER}\
	for rt in $${fonts.rotations}; do\
	    for f in $${fonts.families}; do\
		for file in $${QTEDIR}/lib/fonts/\$${f}_$${fonts.sizes}_\$${rt}.qpf; do\
		    if [ -f \$$file ]; then\
			install -c \$$file $(INSTALL_ROOT)$${fonts.path};\
		    fi;\
		done;\
	    done;\
	done
    INSTALLS+=fonts
}

# Stuff for the SDK and DEVSDK
!win32:!mac {
    sdk_qmake.files=\
	$$(QPEDIR)/bin/qmake\
	$$(QPEDIR)/bin/nct_lupdate\
	$$(QPEDIR)/bin/installpic\
	$$(QPEDIR)/bin/mkipks\
	$$(QPEDIR)/bin/setupfeatures
    sdk_qmake.path=/bin
    sdk_qmake.CONFIG=no_default_install

    sdk_mkspecs.files=$${QTOPIA_DEPOT_PATH}/mkspecs
    sdk_mkspecs.path=/
    sdk_mkspecs.CONFIG=no_default_install

    sdk_qt_includes.files=$${QTEDIR}/include/*
    sdk_qt_includes.path=/include
    sdk_qt_includes.CONFIG=no_default_install

    sdk_pri.files=\
	$${QTOPIA_DEPOT_PATH}/src/config.pri\
	$${QTOPIA_DEPOT_PATH}/src/global.pri\
	$${QTOPIA_DEPOT_PATH}/src/projects.pri\
	$${QTOPIA_DEPOT_PATH}/src/general.pri\
	$${QTOPIA_DEPOT_PATH}/src/ipk_groups.pri\
	$$(QPEDIR)/src/.qmake.cache
    !free_package:sdk_pri.files+=$${QTOPIA_DEPOT_PATH}/src/commercial.pri
    sdk_pri.path=/src
    sdk_pri.CONFIG=no_default_install

    sdk_examples.files=$${QTOPIA_DEPOT_PATH}/examples
    sdk_examples.path=/
    sdk_examples.CONFIG=no_default_install

    INSTALLS+=sdk_qmake sdk_mkspecs sdk_qt_includes sdk_pri sdk_examples
    sdk.depends+=install_sdk_qmake install_sdk_mkspecs install_sdk_qt_includes install_sdk_pri install_sdk_examples

    !isEmpty(QTEDIR) {
	sdk_qtutils.files=\
	    $${QTEDIR}/bin/moc\
	    $${QTEDIR}/bin/uic\
	    $${QTEDIR}/bin/designer\
	    $${QTEDIR}/bin/qvfb
	# alternate location on some Qt installs
	!exists($${QTEDIR}/bin/qvfb):sdk_qtutils.files+=$${QTEDIR}/tools/qvfb/qvfb
	QTOPIA_PHONE:sdk_qtutils.files+=\
	    $${QTEDIR}/tools/qvfb/SmartPhone.skin\
	    $${QTEDIR}/tools/qvfb/TouchscreenPhone.skin
	!QTOPIA_PHONE:sdk_qtutils.files+=\
	    $${QTEDIR}/tools/qvfb/QtopiaPDA.skin
	sdk_qtutils.path=/bin
	sdk_qtutils.CONFIG=no_default_install

	INSTALLS+=sdk_qtutils
	sdk.depends+=install_sdk_qtutils
    }
    !isEmpty(DQTDIR) {
	sdk_qt3utils.files=\
	    $${DQTDIR}/bin/lrelease\
	    $${DQTDIR}/bin/lupdate\
	    $${DQTDIR}/bin/linguist
	sdk_qt3utils.path=/bin
	sdk_qt3utils.CONFIG=no_default_install

	INSTALLS+=sdk_qt3utils
	sdk.depends+=install_sdk_qt3utils
    }
}
