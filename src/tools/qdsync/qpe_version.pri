ver=$$QPE_VERSION
ver~=s/\./ /g
ver=$$ver
!contains(QMAKE_BEHAVIORS,var_split) {
    ver=$$split(ver," ")
}
for(v,ver) {
    isEmpty(QTOPIA_VERSION_MAJOR) {
        QTOPIA_VERSION_MAJOR=0$$v
        QTOPIA_VERSION_MAJOR~=s/.*(\d\d)$/\1/
        next()
    }
    isEmpty(QTOPIA_VERSION_MINOR) {
        QTOPIA_VERSION_MINOR=0$$v
        QTOPIA_VERSION_MINOR~=s/.*(\d\d)$/\1/
        next()
    }
    isEmpty(QTOPIA_VERSION_PATCH) {
        QTOPIA_VERSION_PATCH=0$$v
        QTOPIA_VERSION_PATCH~=s/.*(\d\d)$/\1/
        next()
    }
}
DEFINES+=QTOPIA_VERSION=0x$$QTOPIA_VERSION_MAJOR$$QTOPIA_VERSION_MINOR$$QTOPIA_VERSION_PATCH
DEFINES+=QTOPIA_VERSION_STR=$$LITERAL_ESCAPED_QUOTE$$QPE_VERSION$$LITERAL_ESCAPED_QUOTE
