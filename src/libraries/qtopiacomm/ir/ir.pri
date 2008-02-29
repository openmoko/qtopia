PREFIX=IR
VPATH+=ir

IR_HEADERS+=\
    qiriasdatabase.h\
    qirlocaldevice.h\
    qirnamespace.h\
    qirobexserver.h\
    qirobexsocket.h\
    qirremotedevice.h \
    qirremotedevicewatcher.h

IR_PRIVATE_HEADERS+=\
    qirnamespace_p.h

IR_SOURCES+=\
    qiriasdatabase.cpp\
    qirlocaldevice.cpp\
    qirnamespace.cpp\
    qirobexserver.cpp\
    qirobexsocket.cpp\
    qirremotedevice.cpp \
    qirremotedevicewatcher.cpp

sdk_ir_headers.files=$$IR_HEADERS
sdk_ir_headers.path=/include/qtopia/comm
sdk_ir_headers.hint=sdk headers
INSTALLS+=sdk_ir_headers

sdk_ir_private_headers.files=$$IR_PRIVATE_HEADERS
sdk_ir_private_headers.path=/include/qtopiacomm/private
sdk_ir_private_headers.hint=sdk headers
INSTALLS+=sdk_ir_private_headers

