qtopia_project(embedded lib)
QMAKE=$$QT_DEPOT_PATH/tools/qtestlib/src
VPATH+=$$QMAKE
include($$QMAKE/src.pro)
INSTALLS=
CONFIG+=no_tr
CONFIG-=create_libtool create_pc explicitlib
# force a release (optimized) build
#CONFIG+=release
# fix the include path
tmp=$$INCPATH
INCPATH=
for(p,tmp) {
    exists($$p) {
        INCPATH+=$$p
    }
    !exists($$p):exists($$QMAKE/$$p) {
        INCPATH+=$$QMAKE/$$p
    }
}
#INCPATH+=$$QTDIR/src/corelib/global
# fix the destpath
DESTDIR=$$QPEDIR/lib

idep(LIBS+=-l$$TARGET)

