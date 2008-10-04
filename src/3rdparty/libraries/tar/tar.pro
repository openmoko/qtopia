!qbuild{
qtopia_project(external lib)
license(BSD GPL_COMPATIBLE)
TARGET		=   tar
VERSION		=   1.1.2
CONFIG+=staticlib
CONFIG -= warn_on
}

equals(arch,x86_64) {
    # FIXME this is because CFLAGS_SHLIB isn't being passed (this is a static lib, but it links to a dynamic lib!)
    qbuild:MKSPEC.CFLAGS+=-fPIC
    else:QMAKE_CFLAGS+=-fPIC
}

HEADERS=\
    libtar.h\
    libtar_listhash.h

SOURCES=\
    append.c\
    block.c\
    decode.c\
    encode.c\
    extract.c\
    handle.c\
    libtar_hash.c\
    libtar_list.c\
    output.c\
    strlcat.c\
    strlcpy.c\
    strmode.c\
    util.c\
    wrapper.c

DEFINES+=HAVE_LCHOWN HAVE_STRFTIME

pkg.desc=tar library

!qbuild{
headers.files=$$HEADERS
CONFIG+=syncqtopia
headers.path=/include/tar
headers.hint=non_qt_headers
INSTALLS+=headers

qt_inc(tar)
idep(LIBS+=-l$$TARGET)
}
