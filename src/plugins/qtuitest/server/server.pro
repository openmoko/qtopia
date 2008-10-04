!qbuild{
    qtopia_project(qtopia plugin)
    CONFIG += no_tr qtestlib no_link_test
    plugin_type = qtuitest_server
    TARGET=qtuitest_qpeslave
}

HEADERS +=                      \
        qmemorysampler.h        \
        qtopiaservertestslave.h \
        qtuitestlogreader_p.h
SOURCES +=                          \
        qmemorysampler.cpp          \
        qtopiaservertestslave.cpp   \
        qtuitestlogreader.cpp

MOC_COMPILE_EXCEPTIONS=qtuitestlogreader_p.h

INCLUDEPATH += \
    $$QTOPIA_DEPOT_PATH/src/plugins/qtuitest/slave_qt \
    $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

!qbuild{
    depends(plugins/qtuitest/application)
}


