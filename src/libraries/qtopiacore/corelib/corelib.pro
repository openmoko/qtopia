qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=target
dir=src/$$TARGET
depends(tools/qtopiacore/moc)
depends(tools/qtopiacore/rcc)
depends(tools/qtopiacore/uic)
!enable_singleexec {
    ipatchqt.commands=$$COMMAND_HEADER\
        $$fixpath($$QPEDIR/bin/patchqt) $$fixpath($(INSTALL_ROOT)/lib/libQtCore.so) $$QTOPIA_PREFIX
    ipatchqt.CONFIG=no_path
    ipatchqt.depends=install_qtcore
    INSTALLS+=ipatchqt
}

