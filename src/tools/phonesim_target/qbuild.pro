TEMPLATE=app
CONFIG+=qt embedded
TARGET=phonesim_target

MODULES*=\
    qtopiaphone::headers\
    qtopiacomm::headers\
    qtopia::headers\
    qtopiabase::headers\
    phonesim::headers
QT*=xml

SOURCEPATH+=\
    ../phonesim\
    ../phonesim/lib\
    /src/libraries/qtopiacomm\
    /src/libraries/qtopiaphone\
    /src/libraries/qtopiabase

include(phonesim_target.pro)

