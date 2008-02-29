qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=app rpath ignore_errors
qt=host
dir=tools/linguist/$$TARGET
depends(libraries/qt/*)
