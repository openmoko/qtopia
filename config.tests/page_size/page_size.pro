CONFIG-=qt
TEMPLATE=app
TARGET=page_size
SOURCES=main.cpp
kernel:QMAKE_CXXFLAGS+=-D__KERNEL__
preprocessor.commands=$$QMAKE_CXX $$QMAKE_CXXFLAGS -E $$PWD/main.cpp -o $$OUT_PWD/preprocessor.out
QMAKE_EXTRA_TARGETS+=preprocessor
