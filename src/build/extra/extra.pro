qtopia_project(stub)

QMAKE_STRIP=

defineReplace(maketarget) {
    target=$$1
    target~=s,/,_,g
    target~=s,\.\._,,g
    return($$target)
}

# Depend on everything so we can guarantee that this directory is processed last
for(p,PROJECTS) {
    depends($$p,fake)
}

# Stuff for the SDK
unix {
    # Qtopia files
    bins.files=\
        $$QPEDIR/bin/checktrans\
        $$QPEDIR/bin/phonesim\
        $$QPEDIR/bin/pngscale\
        $$QPEDIR/bin/qdawggen\
        $$QPEDIR/bin/content_installer\
        $$QPEDIR/bin/sxe_installer\
        $$QTOPIA_DEPOT_PATH/bin/Qtopia\
        $$QTOPIA_DEPOT_PATH/bin/Hash\
        $$QTOPIA_DEPOT_PATH/bin/installhelp\
        $$QTOPIA_DEPOT_PATH/bin/installpic\
        $$QTOPIA_DEPOT_PATH/bin/linstall\
        $$QTOPIA_DEPOT_PATH/bin/mkpkg\
        $$QTOPIA_DEPOT_PATH/bin/nct_lupdate\
        $$QTOPIA_DEPOT_PATH/bin/patchqt\
        $$QTOPIA_DEPOT_PATH/bin/printdot\
        $$QTOPIA_DEPOT_PATH/bin/qtopiamake\
        $$QTOPIA_DEPOT_PATH/bin/runwithvars.sh\
        $$QTOPIA_DEPOT_PATH/bin/syncqtopia\
        $$QTOPIA_DEPOT_PATH/bin/write_config_pri\
        $$QTOPIA_DEPOT_PATH/bin/assistant
    bins.path=/bin
    bins.hint=sdk
    INSTALLS+=bins

    phonesim.files=\
        $$QTOPIA_DEPOT_PATH/src/tools/phonesim/*.xml
    phonesim.path=/src/tools/phonesim
    phonesim.hint=sdk
    INSTALLS+=phonesim

    scripts.files=\
        $$QTOPIA_DEPOT_PATH/scripts/runqvfb\
        $$QTOPIA_DEPOT_PATH/scripts/runqpe\
        $$QTOPIA_DEPOT_PATH/scripts/runqtopia\
        $$QTOPIA_DEPOT_PATH/scripts/getpaths.sh\
        $$QTOPIA_DEPOT_PATH/scripts/dumpsec.pl
    scripts.path=/scripts
    scripts.hint=sdk
    INSTALLS+=scripts

    build.files=\
        $$QTOPIA_DEPOT_PATH/src/build
    build.path=/src
    build.hint=sdk
    INSTALLS+=build

    qtopialibs.commands=$$COMMAND_HEADER\
        for file in $$QPEDIR/lib/*; do\
            cp -aRpf \$$file $(INSTALL_ROOT)/lib;\
        done
    qtopialibs.path=/lib
    qtopialibs.hint=sdk
    INSTALLS+=qtopialibs

    qtopiahostlibs.commands=$$COMMAND_HEADER\
        for file in $$QPEDIR/lib/host/*; do\
            if [ $${LITERAL_QUOTE}\$$file$${LITERAL_QUOTE} != $${LITERAL_QUOTE}$$QPEDIR/lib/host/*$${LITERAL_QUOTE} ]; then\
                cp -aRpf \$$file $(INSTALL_ROOT)/lib/host;\
            fi;\
        done
    qtopiahostlibs.path=/lib/host
    qtopiahostlibs.hint=sdk
    INSTALLS+=qtopiahostlibs

    qtopiadocs.commands=$$COMMAND_HEADER
    qtopia_depot:qtopiadocs.commands+=$$QPEDIR/scripts/mkdocs -qpedir $$QPEDIR -force $$LINE_SEP
    qtopiadocs.commands+=cp -Rf $$QPEDIR/doc/html $(INSTALL_ROOT)/doc
    qtopiadocs.path=/doc
    qtopiadocs.hint=sdk
    INSTALLS+=qtopiadocs

    for(pr,PROJECT_ROOTS) {
        alt=$$pr
        alt~=s,$$QTOPIA_DEPOT_PATH,$$QPEDIR,q
        offset=$$pr
        offset~=s,$$QTOPIA_DEPOT_PATH,,q
        isEmpty(offset):offset=/
        !equals(offset,$$pr) {
            target=root$$maketarget($$offset)
            files=\
                $$alt/.qmake.cache\
                $$pr/tree_config.pri\
                $$pr/features
            eval($${target}.files=\$$files)
            eval($${target}.path=\$$offset)
            eval($${target}.hint=sdk)
            INSTALLS+=$$target
        }
    }

    for(pr,PROJECTS) {
        target=proj_$$maketarget($$pr)
        files=$$QTOPIA_DEPOT_PATH/src/$$pr/$$tail($$pr).pro
        !exists($$files):next()
        path=/src/$$pr
        eval($${target}.files=\$$files)
        eval($${target}.path=\$$path)
        eval($${target}.hint=sdk)
        INSTALLS+=$$target
    }

    # Qt files
    dqtbins.files=\
        $$DQTDIR/bin/qmake\
        $$DQTDIR/bin/moc\
        $$DQTDIR/bin/uic\
        $$DQTDIR/bin/rcc\
        $$QPEDIR/bin/qvfb\
        $$DQTDIR/bin/assistant\
        $$DQTDIR/bin/designer\
        $$DQTDIR/bin/linguist\
        $$DQTDIR/bin/lrelease\
        $$DQTDIR/bin/lupdate
    dqtbins.path=/qtopiacore/host/bin
    dqtbins.hint=sdk
    INSTALLS+=dqtbins

    qtebins.files=\
        $$QTEDIR/bin/qmake\
        $$QTEDIR/bin/moc\
        $$QTEDIR/bin/uic\
        $$QTEDIR/bin/rcc
    qtebins.path=/qtopiacore/target/bin
    qtebins.hint=sdk
    INSTALLS+=qtebins

    dqtbinsyms.commands=$$COMMAND_HEADER\
        # Symlink these binaries somewhere useful ($QPEDIR/bin)
        for file in qvfb designer linguist lrelease lupdate; do\
            ln -sf $(INSTALL_ROOT)/qtopiacore/host/bin/\$$file $(INSTALL_ROOT)/bin/\$$file;\
        done
    dqtbinsyms.path=/bin
    dqtbinsyms.hint=sdk
    INSTALLS+=dqtbinsyms

    mkspecs.files=$$QT_DEPOT_PATH/mkspecs
    mkspecs.path=/qtopiacore/qt
    mkspecs.hint=sdk
    INSTALLS+=mkspecs

    mkspecs_symlinks.commands=$$COMMAND_HEADER\
        mkdir -p $(INSTALL_ROOT)/qtopiacore/host/mkspecs $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/qtopiacore/target/mkspecs $$LINE_SEP_VERBOSE\
        ln -sf $(INSTALL_ROOT)/qtopiacore/qt/mkspecs $(INSTALL_ROOT)/qtopiacore/host/mkspecs $$LINE_SEP_VERBOSE\
        ln -sf $(INSTALL_ROOT)/qtopiacore/qt/mkspecs $(INSTALL_ROOT)/qtopiacore/target/mkspecs
    mkspecs_symlinks.CONFIG=no_path
    mkspecs_symlinks.hint=sdk
    INSTALLS+=mkspecs_symlinks

    qvfbmk.files=$$DQTDIR/tools/qvfb/Makefile*
    qvfbmk.path=/qtopiacore/host/tools/qvfb
    qvfbmk.hint=sdk
    INSTALLS+=qvfbmk

    qvfbskins.files=$$QT_DEPOT_PATH/tools/qvfb/*.skin
    qvfbskins.path=/qtopiacore/qt/tools/qvfb
    qvfbskins.hint=sdk
    INSTALLS+=qvfbskins

    dqt_qmakecache.files=$$DQTDIR/.qmake.cache
    dqt_qmakecache.path=/qtopiacore/host
    dqt_qmakecache.hint=sdk
    INSTALLS+=dqt_qmakecache

    qte_qmakecache.files=$$QTEDIR/.qmake.cache
    qte_qmakecache.path=/qtopiacore/target
    qte_qmakecache.hint=sdk
    INSTALLS+=qte_qmakecache

    qtdocs.commands=$$COMMAND_HEADER\
        rm -rf $(INSTALL_ROOT)/qtopiacore/qt/doc $$LINE_SEP\
        ln -s $(INSTALL_ROOT)/doc $(INSTALL_ROOT)/qtopiacore/qt/doc
    qtdocs.path=/qtopiacore/qt
    qtdocs.hint=sdk
    INSTALLS+=qtdocs

    qconfig.files=$$QTOPIA_DEPOT_PATH/qtopiacore/qconfig-qpe.h
    qconfig.path=/qtopiacore
    qconfig.hint=sdk
    INSTALLS+=qconfig

    qtdocsyms.commands=$$COMMAND_HEADER\
        mkdir -p $(INSTALL_ROOT)/qtopiacore/target $$LINE_SEP\
        ln -sf $(INSTALL_ROOT)/qtopiacore/qt/doc $(INSTALL_ROOT)/qtopiacore/target/doc $$LINE_SEP\
        mkdir -p $(INSTALL_ROOT)/qtopiacore/host $$LINE_SEP\
        ln -sf $(INSTALL_ROOT)/qtopiacore/qt/doc $(INSTALL_ROOT)/qtopiacore/host/doc
    qtdocsyms.CONFIG=no_path
    qtdocsyms.hint=sdk
    INSTALLS+=qtdocsyms

    co=sdk
    phone:co+=phone
    else:pda:co+=pda
    configureoptions.commands=$$COMMAND_HEADER\
        echo $$co >$(INSTALL_ROOT)/.configureoptions
    configureoptions.CONFIG=no_path
    configureoptions.hint=sdk
    INSTALLS+=configureoptions

    # device directory
    !isEmpty(DEVICE_CONFIG_PATH) {
        device.files=$$DEVICE_CONFIG_PATH
        device.path=/devices/$$tail($$DEVICE_CONIG_PATH)
        device.hint=sdk
        INSTALLS+=device
    }

    # fix up the config.cache file and then generate a new config.pri
    configpri.commands=$$COMMAND_HEADER\
        echo Fixing paths $$LINE_SEP\
        $$QTOPIA_DEPOT_PATH/bin/sdkcache $(INSTALL_ROOT) $$LINE_SEP\
        $(INSTALL_ROOT)/bin/write_config_pri
    configpri.path=/src
    configpri.hint=sdk
    configpri.depends=
    # Go last or we clobber the real config.pri!
    for(i,INSTALLS) {
        configpri.depends+=install_$$i
    }
    INSTALLS+=configpri
}

listcomponents.commands=$$COMMAND_HEADER\
    echo PROJECTS = $$PROJECTS $$LINE_SEP\
    echo THEMES = $$THEMES
QMAKE_EXTRA_TARGETS+=listcomponents

enable_dbusipc {
    convert_services.commands=$$QPEDIR/bin/convert_services_to_dbus $(INSTALL_ROOT)
    convert_services.CONFIG=no_path
    INSTALLS+=convert_services
}
