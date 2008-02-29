qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=ignore_errors lib
qt=host
dir=tools/designer/src/$$TARGET
depends(libraries/qt/*)
depends(tools/qt/designer/src/uitools)
