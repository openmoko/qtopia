#This file contains projects that make up the Qtopia Test module.

!x11 {
    # QtUitest core
    !qbuild:PROJECTS*=libraries/qtopiacore/qtestlib
    # QtUitest plugin interfaces
    PROJECTS*=\
        libraries/qtuitest

    # QtUitest reference implementation (plugins)
    PROJECTS*=\
        plugins/qtuitest/server \
        plugins/qtuitest/widgets_qt \
        plugins/qtuitest/widgets_qtopia
    qbuild {
        PROJECTS*=plugins/qtuitest/application/qtuitest_appslave
    } else {
        PROJECTS*=plugins/qtuitest/application
    }

    # QtUitest script interpreter
    PROJECTS*=\
        tools/qtuitestrunner/liboverrides \
        tools/qtuitestrunner
    qbuild {
        PROJECTS*=tools/qtuitestrunner/lib_qtopia/qtuitestrunner
    } else {
        PROJECTS*=tools/qtuitestrunner/lib_qtopia
    }

    # performance test helpers
    PROJECTS*=\
        plugins/qtopiacore/gfxdrivers/perftestqvfb \
        plugins/qtopiacore/gfxdrivers/perftestlinuxfb
}
