# Setup the PROJECTS variables

qbuild:PROJECTS*=\
    server\
    libraries/qt\
    libraries/qtopiacore

# Global stuff
PROJECTS*=\
    tools/pngscale\
    tools/svgtopicture

# Qt files
DQT_PROJECTS=\
    libraries/qt/tools/bootstrap\
    libraries/qt/tools/moc\
    libraries/qt/tools/uic\
    libraries/qt/tools/rcc\
    libraries/qt/corelib\
    libraries/qt/gui\
    libraries/qt/network\
    libraries/qt/qtestlib\
    libraries/qt/sql\
    libraries/qt/xml\
    libraries/qt/svg\
    libraries/qt/script\
    plugins/qt\
    tools/qt/assistant\
    tools/qt/assistant/lib\
    tools/qt/assistant/lib/fulltextsearch\
    tools/qt/linguist\
    tools/qt/lupdate\
    tools/qt/lrelease\
    tools/qt/designer/src/uitools\
    tools/qt/designer/src/lib\
    tools/qt/designer/src/components\
    tools/qt/designer/src/designer\
    tools/qt/designer/src/plugins
!win32:DQT_PROJECTS+=tools/qt/qvfb
contains(DQT_CONFIG,qt3support):DQT_PROJECTS+=\
    libraries/qt/qt3support\
    libraries/qt/tools/uic3
contains(DQT_CONFIG,opengl):DQT_PROJECTS+=libraries/qt/opengl
qtopia_depot:DQT_PROJECTS*=tools/qt/qdoc3
win32:DQT_PROJECTS*=\
    libraries/qt/winmain
!qbuild:PROJECTS*=$$DQT_PROJECTS
# When building Qtopia and skipping Qt, build QVFb anyway
build_qtopia:!build_dqt:DQT_PROJECTS-=tools/qt/qvfb
# When using QBuild, we alays build QVFb.
qbuild {
    DQT_PROJECTS-=tools/qt/qvfb
    enable_qvfb {
        PROJECTS*=tools/qt/qvfb
    }
}

# These projects are just stubs for system libraries
PROJECTS*=\
    3rdparty/libraries/alsa\
    3rdparty/libraries/crypt\
    3rdparty/libraries/pthread\
    3rdparty/libraries/dbus\

qbuild:PROJECTS*=\
    3rdparty/libraries/realtime\
    3rdparty/libraries/resolv\
    3rdparty/libraries/mathlib\
    3rdparty/libraries/Xtst\

build_qtopia {
    # Qtopia Core files
    QTE_PROJECTS=\
        libraries/qtopiacore/tools/bootstrap\
        libraries/qtopiacore/tools/moc\
        libraries/qtopiacore/tools/uic\
        libraries/qtopiacore/tools/rcc\
        libraries/qtopiacore/corelib\
        libraries/qtopiacore/gui\
        libraries/qtopiacore/network\
        libraries/qtopiacore/sql\
        libraries/qtopiacore/xml\
        libraries/qtopiacore/svg\
        plugins/qtopiacore
    contains(QTE_CONFIG,opengl):QTE_PROJECTS+=libraries/qtopiacore/opengl
    contains(QTE_CONFIG,xmlpatterns):QTE_PROJECTS+=libraries/qtopiacore/xmlpatterns
    contains(QTE_CONFIG,phonon):QTE_PROJECTS+=libraries/qtopiacore/phonon
    contains(QTE_CONFIG,webkit):QTE_PROJECTS+=libraries/qtopiacore/webkit
    contains(CONFIG,script):QTE_PROJECTS+=libraries/qtopiacore/script

    !qbuild:PROJECTS*=$$QTE_PROJECTS

    qbuild {
        projects_include_modules()
    } else {
        for(m,QTOPIA_MODULES) {
            exists(module_$${m}.pri):eval(include(module_$${m}.pri))
            else:error(Missing module_$${m}.pri)
        }
    }

    # Load a device-specific file (if it exists)
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/projects.pri):include($$DEVICE_CONFIG_PATH/projects.pri)
    # custom projects (not for device builds)
    isEmpty(DEVICE_CONFIG_PATH):include(custom.pri)
    else:exists($$DEVICE_CONFIG_PATH/custom.pri):include($$DEVICE_CONFIG_PATH/custom.pri)
    # local projects
    exists($$QPEDIR/src/local.pri):include($$QPEDIR/src/local.pri)
} else {
    PROJECTS*=\
        3rdparty/libraries/pthread
}

