!qbuild:qtopia_project(unittest)
SOURCES *= tst_dummypresenceservice.cpp
!qbuild:depends(src/libraries/qtopiacollective)
!qbuild:include($$QTOPIA_DEPOT_PATH/tests/shared/qfuturesignal.pri)