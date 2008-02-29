VPATH+=$$dir
INCLUDEPATH+=$$dir
include($$dir/$$tail($$dir).pro)
# Fix any "relative" includes
for(inc,INCLUDEPATH) {
    !containsre($$inc,^/) {
        INCLUDEPATH-=$$inc
        INCLUDEPATH+=$$dir/$$inc
    }
}
REQUIRES=
DESTDIR=
target.path=
INSTALLS-=target
qtopia_project(embedded qtopia core plugin)
CONFIG+=no_tr
SOURCES-=main.cpp
SOURCES+=../wrapmain.cpp
