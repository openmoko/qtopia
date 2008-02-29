qtopia_project(subdirs)

CONFIG+=ordered

SUBDIRS=src
build_qtopiadesktop:SUBDIRS+=src/qtopiadesktop

!isEmpty(EXTRA_SUBDIRS):SUBDIRS+=$$EXTRA_SUBDIRS

enable_singleexec:SUBDIRS+=src/server

!win {
    clean_qtopia.commands=$$COMMAND_HEADER\
        env QPEDIR=$$QPEDIR $$QTOPIA_DEPOT_PATH/scripts/clean_qtopia
    QMAKE_EXTRA_TARGETS+=clean_qtopia
    qtopia_distclean.depends+=clean_qtopia
}

!equals(QTOPIA_SDKROOT,$$QPEDIR) {
    sdk_inst.commands=$(MAKE) sdk
    QMAKE_EXTRA_TARGETS+=sdk_inst
    qtopia_install.depends+=sdk_inst
}

