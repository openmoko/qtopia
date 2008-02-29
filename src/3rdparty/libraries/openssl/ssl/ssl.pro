qtopia_project(external lib)
license(OpenSSL)
TARGET=ssl
VERSION=0.9.8
CONFIG+=syncqtopia

INCLUDEPATH+=$$QPEDIR/include/openssl

PUBLIC_HEADERS+=\
    dtls1.h\
    kssl.h\
    ssl.h\
    ssl2.h\
    ssl23.h\
    ssl3.h\
    tls1.h

HEADERS+=\
    kssl_lcl.h\
    ssl_locl.h

SOURCES+=\
    s2_meth.c\
    s2_srvr.c\
    s2_clnt.c\
    s2_lib.c\
    s2_enc.c\
    s2_pkt.c\
    s3_meth.c\
    s3_srvr.c\
    s3_clnt.c\
    s3_lib.c\
    s3_enc.c\
    s3_pkt.c\
    s3_both.c\
    s23_meth.c\
    s23_srvr.c\
    s23_clnt.c\
    s23_lib.c\
    s23_pkt.c\
    t1_meth.c\
    t1_srvr.c\
    t1_clnt.c\
    t1_lib.c\
    t1_enc.c\
    d1_meth.c\
    d1_srvr.c\
    d1_clnt.c\
    d1_lib.c\
    d1_pkt.c\
    d1_both.c\
    d1_enc.c\
    ssl_lib.c\
    ssl_err2.c\
    ssl_cert.c\
    ssl_sess.c\
    ssl_ciph.c\
    ssl_stat.c\
    ssl_rsa.c\
    ssl_asn1.c\
    ssl_txt.c\
    ssl_algs.c\
    bio_ssl.c\
    ssl_err.c\
    kssl.c

HEADERS+=$$PUBLIC_HEADERS

depends(3rdparty/libraries/openssl/crypto)

certs.files=../certs/*
certs.path=/etc/ssl/certs
INSTALLS+=certs

oconfig.files=../openssl.cnf
oconfig.path=/etc/ssl/
INSTALLS+=oconfig

idep(LIBS+=-l$$TARGET)

sdk_crypto_headers.files=$$PUBLIC_HEADERS
sdk_crypto_headers.path=/include/openssl
sdk_crypto_headers.hint=non_qt_headers
INSTALLS+=sdk_crypto_headers

