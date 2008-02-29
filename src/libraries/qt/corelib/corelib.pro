qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=host
dir=src/$$TARGET
depends(tools/qt/moc)
depends(tools/qt/rcc)
depends(tools/qt/uic)
