qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=target
dir=src/$$TARGET
depends(libraries/qtopiacore/corelib)
depends(libraries/qtopiacore/gui)
depends(libraries/qtopiacore/xml)
