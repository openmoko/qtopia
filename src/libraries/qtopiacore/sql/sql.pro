qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=target
dir=src/$$TARGET
depends(libraries/qtopiacore/corelib)
depends(3rdparty/libraries/sqlite,fake)
