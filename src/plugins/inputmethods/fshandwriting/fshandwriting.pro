qtopia_project(qtopia plugin)
TARGET=qfshandwriting

HEADERS	    = fshandwritingimpl.h qfsinput.h charmatch.h
SOURCES	    = fshandwritingimpl.cpp qfsinput.cpp charmatch.cpp

depends(libraries/handwriting)

# if commercial only?
depends(3rdparty/libraries/inputmatch)

etc.files = $$QTOPIA_DEPOT_PATH/etc/qimpen/fstext.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fsnum.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fscombining.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fs.conf
etc.path=/etc/qimpen/
pics.files=$$QTOPIA_DEPOT_PATH/pics/fshw/*.png
pics.path=/pics/fshw
pics.hint=pics
INSTALLS+=etc pics

pkg.name=qpe-fshandwriting
pkg.domain=libs
