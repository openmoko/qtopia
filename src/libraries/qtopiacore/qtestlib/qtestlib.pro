qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=lib
qt=target
dir=tools/$$TARGET
depends(libraries/qtopiacore/corelib)

# Prevent potential aliasing problem caused by reinterpret_cast
# in qtestkeyboard.h
idep(QMAKE_CXXFLAGS+=-fno-strict-aliasing -Wno-strict-aliasing)
