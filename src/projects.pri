# Setup the PROJECTS variables

# Global stuff
PROJECTS*=\
    tools/pngscale

# Qt files
DQT_PROJECTS=\
    tools/qt/moc\
    tools/qt/uic\
    tools/qt/uic3\
    tools/qt/rcc\
    libraries/qt/corelib\
    libraries/qt/gui\
    libraries/qt/network\
    libraries/qt/sql\
    libraries/qt/xml\
    libraries/qt/qt3support\
    plugins/qt\
    tools/qt/lupdate\
    tools/qt/lrelease\
    tools/qt/assistant\
    tools/qt/assistant/lib\
    tools/qt/designer/src/uitools\
    tools/qt/designer/src/lib\
    tools/qt/designer/src/components\
    tools/qt/designer/src/designer\
    tools/qt/designer/src/plugins\
    tools/qt/linguist
!win:DQT_PROJECTS+=tools/qt/qvfb
contains(DQT_CONFIG,opengl):DQT_PROJECTS+=libraries/qt/opengl
!equals(DQT_MINOR_VERSION,1):DQT_PROJECTS+=libraries/qt/svg
qtopia_depot:DQT_PROJECTS*=tools/qt/qdoc3
win32:DQT_PROJECTS*=\
    libraries/qt/winmain\
    libraries/qt/activeqt\
    tools/qt/dumpcpp
PROJECTS*=$$DQT_PROJECTS
# When building Qtopia and skipping Qt, build QVFb anyway
build_qtopia:!build_dqt:DQT_PROJECTS-=tools/qt/qvfb

build_qtopia {
    include(general.pri)
    !free_package|free_plus_binaries:include(commercial.pri)
    # Load a device-specific file (if it exists)
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/projects.pri):include($$DEVICE_CONFIG_PATH/projects.pri)
    include(custom.pri)
    include($$QPEDIR/src/local.pri)
}

