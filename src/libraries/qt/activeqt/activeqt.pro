qtopia_project(qtopiacore)
TARGET=container
QTOPIACORE_CONFIG=lib
qt=host
dir=src/activeqt/$$TARGET
depends(libraries/qt/corelib)
depends(libraries/qt/gui)
depends(tools/qt/moc)
