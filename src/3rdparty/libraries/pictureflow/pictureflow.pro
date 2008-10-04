!qbuild{
qtopia_project(embedded lib)
TARGET          = pictureflow
VERSION         = 0.1.0
license(MIT)
CONFIG         -= warn_off
CONFIG+=no_tr
}

HEADERS = pictureflow.h \
          pictureflowview.h
SOURCES = pictureflow.cpp \
          pictureflowview.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/pictureflow
headers.hint=non_qt_headers
INSTALLS+=headers

qt_inc($$TARGET)
idep(LIBS+=-l$$TARGET)
}


