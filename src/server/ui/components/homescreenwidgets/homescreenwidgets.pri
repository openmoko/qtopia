HEADERS+=\
        homescreenwidgets.h

SOURCES+=\
        homescreenwidgets.cpp

hswidgets.files=$$device_overrides(/etc/default/Trolltech/HomeScreenWidgets.conf)
!isEmpty(hswidgets.files) {
    hswidgets.path=/etc/default/Trolltech
    INSTALLS+=hswidgets
}

