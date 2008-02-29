TARGET=printserver
#CONFIG+=no_tr

enable_singleexec {
    qtopia_project(qtopia qtopia app)
    instSymlink.path=$$bindir
    instSymlink.commands=$$COMMAND_HEADER\
        rm -f $(INSTALL_ROOT)$${instSymlink.path}/$${TARGET} $$LINE_SEP_VERBOSE\
        ln -sf qpe $(INSTALL_ROOT)$${instSymlink.path}/$${TARGET}
    INSTALLS+=instSymlink
} else {
    qtopia_project(qtopia qtopia app)
}

HEADERS+= printserver.h

SOURCES+= main.cpp \
          printserver.cpp

service.files=$$QTOPIA_DEPOT_PATH/services/Print/printserver
service.path=/services/Print

INSTALLS+=service

depends(libraries/qtopiaprinting)

pkg.desc=Print server for mobile printing
pkg.domain=window,print,phonecomm,cardreader
