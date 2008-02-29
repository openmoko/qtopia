qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=ignore_errors app rpath
qt=host
dir=tools/designer/src/$$TARGET
depends(libraries/qt/*)
depends(tools/qt/designer/src/lib)
depends(tools/qt/designer/src/components)
depends(tools/qt/assistant/lib)
