qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=host
dir=src/$$TARGET
depends(libraries/qt/tools/moc)
depends(libraries/qt/tools/uic)
depends(libraries/qt/tools/rcc)
win32:depends(libraries/qt/winmain)
