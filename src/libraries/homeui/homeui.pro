!qbuild{
qtopia_project(qtopia lib)
TARGET=homeui
CONFIG+=qtopia_visibility no_tr
depends(libraries/qtopiapim)
}

HEADERS=\
    qribbonselector.h

SEMI_PRIVATE_HEADERS=\
    homewidgets_p.h

SOURCES=\
    homewidgets_p.cpp \
    qribbonselector.cpp

pics.files=$$QTOPIA_DEPOT_PATH/pics/home/*
pics.path=/pics/home
pics.hint=pics
INSTALLS+=pics

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/homeui
headers.hint=headers sdk
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopia/homeui/private
pheaders.hint=sdk headers
INSTALLS+=pheaders

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
