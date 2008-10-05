qtopia_project(qtopia lib)
TARGET=qtopiacomm

include(qtopiacomm.pri)

sdk_qtopiacomm_headers.files=$${HEADERS}
sdk_qtopiacomm_headers.path=/include/qtopia/comm
sdk_qtopiacomm_headers.hint=sdk headers
INSTALLS+=sdk_qtopiacomm_headers

defineTest(setupTranslatables) {
    file=$$1
    include($$file)
    VARS=HEADERS SOURCES FORMS PRIVATE_HEADERS
    for(v,VARS) {
        eval(TRANSLATABLES+=\$${$$v})
    }
    export(TRANSLATABLES)
    export(VPATH)
}

defineTest(projectEnabled) {
    PROJECTS=
    include($$QTOPIA_DEPOT_PATH/src/projects.pri)
    contains(PROJECTS,$$1):return(1)
    else:return(0)
}

projectEnabled(3rdparty/libraries/openobex) {
    depends(3rdparty/libraries/openobex)
    include(obex/obex.pri)
    resolve_include()
} else {
    setupTranslatables(obex/obex.pri)
}

enable_bluetooth {
    depends(3rdparty/libraries/qtdbus)
    include(bluetooth/bluetooth.pri)
    resolve_include()
} else {
    setupTranslatables(bluetooth/bluetooth.pri)
}

enable_infrared {
    include(ir/ir.pri)
    resolve_include()
}
else {
    setupTranslatables(ir/ir.pri)
}

include(network/network.pri)
resolve_include()
include(serial/serial.pri)
resolve_include()
enable_vpn {
    include(vpn/vpn.pri)
    resolve_include()
} else {
    setupTranslatables(vpn/vpn.pri)
}

include(usb/usb.pri)
resolve_include()

CONFIG += qtopia_visibility

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)

