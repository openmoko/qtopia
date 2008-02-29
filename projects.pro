qtopia_project(subdirs)

CONFIG+=ordered

SUBDIRS=src
build_qtopia:!enable_singleexec {
    # We only want make to do examples, not make install!
    defineTest(append_examples) {
        qtopia_all.commands+=$$esc(\n\t)@$$MAKE -C examples
        export(qtopia_all.commands)
    }
    runlast(append_examples())
}
build_qtopiadesktop:SUBDIRS+=src/qtopiadesktop

!isEmpty(EXTRA_SUBDIRS):SUBDIRS+=$$EXTRA_SUBDIRS

enable_singleexec:SUBDIRS+=src/server

!win {
    clean_qtopia.commands=$$COMMAND_HEADER\
        env QPEDIR=$$QPEDIR $$QTOPIA_DEPOT_PATH/scripts/clean_qtopia
    QMAKE_EXTRA_TARGETS+=clean_qtopia
    qtopia_distclean.depends+=clean_qtopia
}

# Since people expect 'make install' to do the right thing and since it
# doesn't always do the right thing (it's additive, not replacing), force
# 'make install' to remove the image, making it work like 'make cleaninstall'
win32:RMRF=rmdir /s /q
else:RMRF=rm -rf
cleanimage.commands=$$COMMAND_HEADER
PREFIXES=$(IMAGE) $(DIMAGE)
for(prefix,PREFIXES) {
    win32:cleanimage.commands+=if exist $$prefix
    cleanimage.commands+=$$RMRF $$prefix $$LINE_SEP
}
cleanimage.commands+=$$DUMMY_COMMAND
QMAKE_EXTRA_TARGETS+=cleanimage
qtopia_install.depends+=cleanimage

# I'm sure at least one person will need to force an additive install so
# give them a way to do it
runlast(append_install.commands=\$$qtopia_install.commands)
QMAKE_EXTRA_TARGETS+=append_install

!equals(QTOPIA_SDKROOT,$$QPEDIR) {
    # make install implies make sdk
    sdk_inst.commands=$(MAKE) sdk
    QMAKE_EXTRA_TARGETS+=sdk_inst
    qtopia_install.depends+=sdk_inst

    # make sdk implies make cleansdk
    cleansdk.commands=$$COMMAND_HEADER
    win32:cleansdk.commands+=if exist $$prefix
    cleansdk.commands+=$$RMRF $(SDKROOT)
    QMAKE_EXTRA_TARGETS+=cleansdk
    check_sdk.depends+=cleansdk
}

# Don't let common.prf put in the cleaninstall rule. Just make it call our install rule.
CONFIG+=no_cleaninstall
cleaninstall.commands=
cleaninstall.depends=qtopia_install
QMAKE_EXTRA_TARGETS+=cleaninstall
cinstall.commands=
cinstall.depends=qtopia_install
QMAKE_EXTRA_TARGETS+=cinstall

