/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __SSLLIBRARY_H
#define __SSLLIBRARY_H

#include <QLibrary>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509.h>

#include "qtopiaglobal.h"

class QTOPIACOMM_EXPORT SSLLibrary
{
    public:
        explicit SSLLibrary(const QString& pathToLib);
        ~SSLLibrary();
        bool isLoaded();
        int SSL_library_init();
        void SSL_load_error_strings();
        int RAND_status();
        void RAND_seed(const void *buf,int num);
        X509 *SSL_get_certificate(const SSL *ssl);
        char * X509_NAME_oneline(X509_NAME *a,char *buf,int size);
        X509_NAME * X509_get_issuer_name(X509 *a);
        X509_NAME * X509_get_subject_name(X509 *a);
        void CRYPTO_free(void *a);
        void X509_free(X509* a);
        long ASN1_INTEGER_get(ASN1_INTEGER *a);
        X509* SSL_get_peer_certificate(const SSL *s);
        ASN1_INTEGER *   X509_get_serialNumber(X509 *x);
        SSL_CTX *SSL_CTX_new(SSL_METHOD *meth);
        SSL_METHOD *SSLv23_server_method();
        SSL_METHOD *SSLv23_client_method();
        long SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, void *parg);
        int SSL_CTX_set_cipher_list(SSL_CTX* ctx,const char *str);
        int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx);
        int SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type);
        int SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type);
        int SSL_CTX_check_private_key(const SSL_CTX *ctx);
        int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,const char *CApath);
        void SSL_CTX_set_verify_depth(SSL_CTX *ctx,int depth);
        void SSL_set_bio(SSL *s, BIO *rbio,BIO *wbio);
        void SSL_set_accept_state(SSL *s);
        void SSL_set_connect_state(SSL *s);
        int BIO_write(BIO *b, const void *data, int len);
        int SSL_read(SSL *ssl,void *buf,int num);
        int SSL_get_error(const SSL *s,int ret_code);
        int SSL_write(SSL *ssl,const void *buf,int num);
        long BIO_ctrl(BIO *bp,int cmd,long larg,void *parg);
        int BIO_read(BIO *b, void *data, int len);
        int SSL_accept(SSL *ssl);
        int SSL_connect(SSL *ssl);
        long SSL_get_verify_result(const SSL *ssl);
        unsigned long ERR_get_error();
        char *ERR_error_string(unsigned long e,char *buf);
        SSL * SSL_new(SSL_CTX *ctx);
        int SSL_clear(SSL *s);
        BIO_METHOD *BIO_s_mem(void);
        BIO * BIO_new(BIO_METHOD *type);
    private:
        QLibrary _lib;
};

#endif

