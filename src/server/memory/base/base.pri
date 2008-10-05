HEADERS+=\
    oommanager.h \
    lowmemorytask.h \
    memorymonitor.h

SOURCES+=\
    oommanager.cpp \
    lowmemorytask.cpp \
    memorymonitor.cpp

oomconf.files=$$device_overrides(/etc/default/Trolltech/oom.conf)
!isEmpty(oomconf.files) {
    oomconf.path=/etc/default/Trolltech
    INSTALLS+=oomconf
}

