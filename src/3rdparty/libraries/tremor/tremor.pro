!qbuild{
qtopia_project(external lib)
TARGET          = vorbisidec
VERSION         = 1.0.2
license(BSD GPL_COMPATIBLE)
depends(3rdparty/libraries/pthread)
}

HEADERS+=\
    codebook.h\
    misc.h\
    mdct_lookup.h\
    os.h\
    mdct.h\
    ivorbisfile.h\
    lsp_lookup.h\
    window_lookup.h\
    codec_internal.h\
    ogg.h\
    asm_arm.h\
    ivorbiscodec.h

SOURCES+=\
    mdct.c\
    dsp.c\
    info.c\
    framing2.c\
    misc.c\
    floor_lookup.c\
    floor1.c\
    floor0.c\
    vorbisfile.c\
    res012.c\
    mapping0.c\
    codebook.c\
    bitwise.c

DEFINES+=\
    REENTRANT\
    _LOW_ACCURANCY_\
    LIMIT_TO_64kHz\
    USE_MEMORY_H\
    STDC_HEADERS=1\
    HAVE_SYS_TYPES_H=1\
    HAVE_SYS_STAT_H=1\
    HAVE_STDLIB_H=1\
    HAVE_STRING_H=1\
    HAVE_MEMORY_H=1\
    HAVE_STRINGS_H=1\
    HAVE_INTTYPES_H=1\
    HAVE_STDINT_H=1\
    HAVE_UNISTD_H=1\
    HAVE_DLFCN_H=1\
    HAVE_ALLOCA_H=1\
    HAVE_ALLOCA=1

qbuild:MKSPEC.CFLAGS+=-fsigned-char
else:QMAKE_CFLAGS+=-fsigned-char

!qbuild{
headers.files=$$HEADERS
CONFIG+=syncqtopia
headers.path=include/tremor
headers.hint=non_qt_headers
INSTALLS+=headers

qt_inc(tremor)
idep(LIBS+=-l$$TARGET)
}
