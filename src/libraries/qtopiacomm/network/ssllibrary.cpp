/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <ssllibrary.h>
#ifdef SINGLE_EXEC
// OpenSSL includes
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

SSLLibrary::SSLLibrary(const QString& path)
#ifndef SINGLE_EXEC
    :
    _lib(path)
{
#else
{
    Q_UNUSED(path);
#endif
}

SSLLibrary::~SSLLibrary()
{
#ifndef SINGLE_EXEC
    _lib.unload();
#endif
}

bool SSLLibrary::isLoaded()
{
#ifdef SINGLE_EXEC
    return true;
#else
    return _lib.isLoaded();
#endif
}

int SSLLibrary::SSL_library_init()
{
#ifdef SINGLE_EXEC
    return ::SSL_library_init();
#else
    typedef int(*Func)(void);
    Func f =(Func) _lib.resolve("SSL_library_init");
    Q_ASSERT(f);
    return f();
#endif
}

void SSLLibrary::SSL_load_error_strings()
{
#ifdef SINGLE_EXEC
    ::SSL_load_error_strings();
#else
    typedef void(*Func)(void);
    Func f =(Func) _lib.resolve("SSL_load_error_strings");
    Q_ASSERT(f);
    f();
#endif
}

int SSLLibrary::RAND_status()
{
#ifdef SINGLE_EXEC
    return ::RAND_status();
#else
    typedef int(*Func)(void);
    Func f =(Func) _lib.resolve("RAND_status");
    Q_ASSERT(f);
    return f();
#endif
}

void SSLLibrary::RAND_seed(const void *buf,int num)
{
#ifdef SINGLE_EXEC
    ::RAND_seed(buf,num);
#else
    typedef void(*Func)(const void *,int);
    Func f =(Func) _lib.resolve("RAND_seed");
    Q_ASSERT(f);
    f(buf,num);
#endif
}

X509 *SSLLibrary::SSL_get_certificate(const SSL *ssl)
{
#ifdef SINGLE_EXEC
    return ::SSL_get_certificate(ssl);
#else
    typedef X509*(*Func)(const SSL *);
    Func f =(Func) _lib.resolve("SSL_get_certificate");
    Q_ASSERT(f);
    return f(ssl);
#endif
}

char * SSLLibrary::X509_NAME_oneline(X509_NAME *a,char *buf,int size)
{
#ifdef SINGLE_EXEC
    return ::X509_NAME_oneline(a,buf,size);
#else
    typedef char*(*Func)(X509_NAME *,char *,int);
    Func f =(Func) _lib.resolve("X509_NAME_oneline");
    Q_ASSERT(f);
    return f(a,buf,size);
#endif
}

X509_NAME * SSLLibrary::X509_get_issuer_name(X509 *a)
{
#ifdef SINGLE_EXEC
    return ::X509_get_issuer_name(a);
#else
    typedef X509_NAME*(*Func)(X509 *);
    Func f =(Func) _lib.resolve("X509_get_issuer_name");
    Q_ASSERT(f);
    return f(a);
#endif
}

X509_NAME * SSLLibrary::X509_get_subject_name(X509 *a)
{
#ifdef SINGLE_EXEC
    return ::X509_get_subject_name(a);
#else
    typedef X509_NAME*(*Func)(X509 *);
    Func f =(Func) _lib.resolve("X509_get_subject_name");
    Q_ASSERT(f);
    return f(a);
#endif
}

void SSLLibrary::CRYPTO_free(void* a)
{
#ifdef SINGLE_EXEC
    ::CRYPTO_free(a);
#else
    typedef void(*Func)(void*);
    Func f =(Func) _lib.resolve("CRYPTO_free");
    Q_ASSERT(f);
    f(a);
#endif
}

void SSLLibrary::X509_free(X509* a)
{
#ifdef SINGLE_EXEC
    ::X509_free(a);
#else
    typedef void(*Func)(X509*);
    Func f =(Func) _lib.resolve("X509_free");
    Q_ASSERT(f);
    f(a);
#endif
}

long SSLLibrary::ASN1_INTEGER_get(ASN1_INTEGER *a)
{
#ifdef SINGLE_EXEC
    return ::ASN1_INTEGER_get(a);
#else
    typedef long(*Func)(ASN1_INTEGER*);
    Func f =(Func)_lib.resolve("ASN1_INTEGER_get");
    Q_ASSERT(f);
    return f(a);
#endif
}

X509* SSLLibrary::SSL_get_peer_certificate(const SSL *s)
{
#ifdef SINGLE_EXEC
    return ::SSL_get_peer_certificate(s);
#else
    typedef X509*(*Func)(const SSL *);
    Func f =(Func)_lib.resolve("SSL_get_peer_certificate");
    Q_ASSERT(f);
    return f(s);
#endif
}

ASN1_INTEGER * SSLLibrary::X509_get_serialNumber(X509 *x)
{
#ifdef SINGLE_EXEC
    return ::X509_get_serialNumber(x);
#else
    typedef ASN1_INTEGER*(*Func)(X509 *);
    Func f =(Func)_lib.resolve("X509_get_serialNumber");
    Q_ASSERT(f);
    return f(x);
#endif
}

SSL_CTX *SSLLibrary::SSL_CTX_new(SSL_METHOD *meth)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_new(meth);
#else
    typedef SSL_CTX*(*Func)(SSL_METHOD *);
    Func f =(Func)_lib.resolve("SSL_CTX_new");
    Q_ASSERT(f);
    return f(meth);
#endif
}

SSL_METHOD *SSLLibrary::SSLv23_server_method()
{
#ifdef SINGLE_EXEC
    return ::SSLv23_server_method();
#else
    typedef SSL_METHOD*(*Func)(void);
    Func f =(Func)_lib.resolve("SSLv23_server_method");
    Q_ASSERT(f);
    return f();
#endif
}

SSL_METHOD *SSLLibrary::SSLv23_client_method()
{
#ifdef SINGLE_EXEC
    return ::SSLv23_client_method();
#else
    typedef SSL_METHOD*(*Func)(void);
    Func f =(Func)_lib.resolve("SSLv23_client_method");
    Q_ASSERT(f);
    return f();
#endif
}

long SSLLibrary::SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, void *parg)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_ctrl(ctx,cmd,larg,parg);
#else
    typedef long(*Func)(SSL_CTX *ctx,int cmd, long larg, void *parg);
    Func f =(Func)_lib.resolve("SSL_CTX_ctrl");
    Q_ASSERT(f);
    return f(ctx,cmd,larg,parg);
#endif
}

int SSLLibrary::SSL_CTX_set_cipher_list(SSL_CTX * ctx,const char *str)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_set_cipher_list(ctx,str);
#else
    typedef int(*Func)(SSL_CTX *,const char *);
    Func f =(Func)_lib.resolve("SSL_CTX_set_cipher_list");
    Q_ASSERT(f);
    return f(ctx,str);
#endif
}

int SSLLibrary::SSL_CTX_set_default_verify_paths(SSL_CTX *ctx)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_set_default_verify_paths(ctx);
#else
    typedef int(*Func)(SSL_CTX *);
    Func f =(Func)_lib.resolve("SSL_CTX_set_default_verify_paths");
    Q_ASSERT(f);
    return f(ctx);
#endif
}

int SSLLibrary::SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_use_certificate_file(ctx,file,type);
#else
    typedef int(*Func)(SSL_CTX *, const char *, int);
    Func f =(Func)_lib.resolve("SSL_CTX_use_certificate_file");
    Q_ASSERT(f);
    return f(ctx,file,type);
#endif
}

int SSLLibrary::SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_use_PrivateKey_file(ctx,file,type);
#else
    typedef int(*Func)(SSL_CTX *, const char *, int);
    Func f =(Func)_lib.resolve("SSL_CTX_use_PrivateKey_file");
    Q_ASSERT(f);
    return f(ctx,file,type);
#endif
}

int SSLLibrary::SSL_CTX_check_private_key(const SSL_CTX *ctx)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_check_private_key(ctx);
#else
    typedef int(*Func)(const SSL_CTX *);
    Func f =(Func)_lib.resolve("SSL_CTX_check_private_key");
    Q_ASSERT(f);
    return f(ctx);
#endif
}

int SSLLibrary::SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,const char *CApath)
{
#ifdef SINGLE_EXEC
    return ::SSL_CTX_load_verify_locations(ctx,CAfile,CApath);
#else
    typedef int(*Func)(SSL_CTX *, const char *,const char *);
    Func f =(Func)_lib.resolve("SSL_CTX_load_verify_locations");
    Q_ASSERT(f);
    return f(ctx,CAfile,CApath);
#endif
}

void SSLLibrary::SSL_CTX_set_verify_depth(SSL_CTX *ctx,int depth)
{
#ifdef SINGLE_EXEC
    ::SSL_CTX_set_verify_depth(ctx,depth);
#else
    typedef void(*Func)(SSL_CTX *,int);
    Func f =(Func)_lib.resolve("SSL_CTX_set_verify_depth");
    Q_ASSERT(f);
    f(ctx,depth);
#endif
}

void SSLLibrary::SSL_set_bio(SSL *s, BIO *rbio,BIO *wbio)
{
#ifdef SINGLE_EXEC
    ::SSL_set_bio(s,rbio,wbio);
#else
    typedef void(*Func)(SSL *, BIO *,BIO *);
    Func f =(Func)_lib.resolve("SSL_set_bio");
    Q_ASSERT(f);
    f(s,rbio,wbio);
#endif
}

void SSLLibrary::SSL_set_accept_state(SSL *s)
{
#ifdef SINGLE_EXEC
    ::SSL_set_accept_state(s);
#else
    typedef void(*Func)(SSL *);
    Func f =(Func)_lib.resolve("SSL_set_accept_state");
    Q_ASSERT(f);
    f(s);
#endif
}

void SSLLibrary::SSL_set_connect_state(SSL *s)
{
#ifdef SINGLE_EXEC
    ::SSL_set_connect_state(s);
#else
    typedef void(*Func)(SSL *);
    Func f =(Func)_lib.resolve("SSL_set_connect_state");
    Q_ASSERT(f);
    f(s);
#endif
}

int SSLLibrary::BIO_write(BIO *b, const void *data, int len)
{
#ifdef SINGLE_EXEC
    return ::BIO_write(b,data,len);
#else
    typedef int(*Func)(BIO *, const void *, int);
    Func f =(Func)_lib.resolve("BIO_write");
    Q_ASSERT(f);
    return f(b,data,len);
#endif
}

int SSLLibrary::SSL_read(SSL *ssl,void *buf,int num)
{
#ifdef SINGLE_EXEC
    return ::SSL_read(ssl,buf,num);
#else
    typedef int(*Func)(SSL *,void *,int);
    Func f =(Func)_lib.resolve("SSL_read");
    Q_ASSERT(f);
    return f(ssl,buf,num);
#endif
}

int SSLLibrary::SSL_get_error(const SSL *s,int ret_code)
{
#ifdef SINGLE_EXEC
    return ::SSL_get_error(s,ret_code);
#else
    typedef int(*Func)(const SSL *,int);
    Func f =(Func)_lib.resolve("SSL_get_error");
    Q_ASSERT(f);
    return f(s,ret_code);
#endif
}

int SSLLibrary::SSL_write(SSL *ssl,const void *buf,int num)
{
#ifdef SINGLE_EXEC
    return ::SSL_write(ssl,buf,num);
#else
    typedef int(*Func)(SSL *,const void *,int);
    Func f =(Func)_lib.resolve("SSL_write");
    Q_ASSERT(f);
    return f(ssl,buf,num);
#endif
}

long SSLLibrary::BIO_ctrl(BIO *bp,int cmd,long larg,void *parg)
{
#ifdef SINGLE_EXEC
    return ::BIO_ctrl(bp,cmd,larg,parg);
#else
    typedef long(*Func)(BIO *,int,long,void *);
    Func f =(Func)_lib.resolve("BIO_ctrl");
    Q_ASSERT(f);
    return f(bp,cmd,larg,parg);
#endif
}

int SSLLibrary::BIO_read(BIO *b, void *data, int len)
{
#ifdef SINGLE_EXEC
    return ::BIO_read(b,data,len);
#else
    typedef int(*Func)(BIO *, void *, int);
    Func f =(Func)_lib.resolve("BIO_read");
    Q_ASSERT(f);
    return f(b,data,len);
#endif
}

int SSLLibrary::SSL_accept(SSL *ssl)
{
#ifdef SINGLE_EXEC
    return ::SSL_accept(ssl);
#else
    typedef int(*Func)(SSL *);
    Func f =(Func)_lib.resolve("SSL_accept");
    Q_ASSERT(f);
    return f(ssl);
#endif
}
int SSLLibrary::SSL_connect(SSL *ssl)
{
#ifdef SINGLE_EXEC
    return ::SSL_connect(ssl);
#else
    typedef int(*Func)(SSL *);
    Func f =(Func)_lib.resolve("SSL_connect");
    Q_ASSERT(f);
    return f(ssl);
#endif
}

long SSLLibrary::SSL_get_verify_result(const SSL *ssl)
{
#ifdef SINGLE_EXEC
    return ::SSL_get_verify_result(ssl);
#else
    typedef long(*Func)(const SSL *);
    Func f =(Func)_lib.resolve("SSL_get_verify_result");
    Q_ASSERT(f);
    return f(ssl);
#endif
}

unsigned long SSLLibrary::ERR_get_error()
{
#ifdef SINGLE_EXEC
    return ::ERR_get_error();
#else
    typedef unsigned long(*Func)(void);
    Func f =(Func)_lib.resolve("ERR_get_error");
    Q_ASSERT(f);
    return f();
#endif
}

char *SSLLibrary::ERR_error_string(unsigned long e,char *buf)
{
#ifdef SINGLE_EXEC
    return ::ERR_error_string(e,buf);
#else
    typedef char*(*Func)(unsigned long,char *);
    Func f =(Func)_lib.resolve("ERR_error_string");
    Q_ASSERT(f);
    return f(e,buf);
#endif
}

SSL * SSLLibrary::SSL_new(SSL_CTX *ctx)
{
#ifdef SINGLE_EXEC
    return ::SSL_new(ctx);
#else
    typedef SSL*(*Func)(SSL_CTX *);
    Func f =(Func)_lib.resolve("SSL_new");
    Q_ASSERT(f);
    return f(ctx);
#endif
}

int SSLLibrary::SSL_clear(SSL *s)
{
#ifdef SINGLE_EXEC
    return :: SSL_clear(s);
#else
    typedef int(*Func)(SSL *);
    Func f =(Func)_lib.resolve("SSL_clear");
    Q_ASSERT(f);
    return f(s);
#endif
}

BIO_METHOD *SSLLibrary::BIO_s_mem(void)
{
#ifdef SINGLE_EXEC
    return ::BIO_s_mem();
#else
    typedef BIO_METHOD*(*Func)(void);
    Func f =(Func)_lib.resolve("BIO_s_mem");
    Q_ASSERT(f);
    return f();
#endif
}

BIO * SSLLibrary::BIO_new(BIO_METHOD *type)
{
#ifdef SINGLE_EXEC
    return ::BIO_new(type);
#else
    typedef BIO *(*Func)(BIO_METHOD *);
    Func f =(Func)_lib.resolve("BIO_new");
    Q_ASSERT(f);
    return f(type);
#endif
}

