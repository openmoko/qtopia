TEMPLATE = lib
TARGET = spygrind

CONFIG += qtopia

HEADERS +=                  \
    method_p.h              \
    qsignalspycallback_p.h  \
    qsignalspycollector.h   \
    stamp_p.h

SOURCES +=                  \
    hook.cpp                \
    method.cpp              \
    qsignalspycollector.cpp

enable_malloc_hook {
    SOURCES += mallochook.cpp
    DEFINES += SPYGRIND_MALLOC_HOOK
}

unix {
    HEADERS+=qunixsignalnotifier_p.h
    SOURCES+=qunixsignalnotifier.cpp
}

requires(!enable_singleexec)

