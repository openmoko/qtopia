!qbuild:include($$QTOPIA_DEPOT_PATH/tests/shared/qfuturesignal.pri)

!qbuild:qtopia_project(integrationtest)
SOURCES*=tst_qcontactsimcontext.cpp
!qbuild:depends(src/libraries/qtopiapim)
