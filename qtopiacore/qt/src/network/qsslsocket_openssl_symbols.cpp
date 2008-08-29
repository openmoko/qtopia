/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** In addition, as a special exception, Trolltech gives permission to
** link the code of its release of Qt with the OpenSSL project's
** "OpenSSL" library (or modified versions of it that use the same
** license as the "OpenSSL" library), and distribute the linked
** executables.  You must comply with the GNU General Public License
** version 2 or the GNU General Public License version 3 in all
** respects for all of the code used other than the "OpenSSL" code.
** If you modify this file, you may extend this exception to your
** version of the file, but you are not obligated to do so.  If you do
** not wish to do so, delete this exception statement from your
** version of this file.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qsslsocket_openssl_symbols_p.h"

#include <QtCore/qlibrary.h>
#include <QtCore/qmutex.h>
#include <private/qmutexpool_p.h>

#ifdef SSLEAY_MACROS
DEFINEFUNC3(void *, ASN1_dup, i2d_of_void *a, a, d2i_of_void *b, b, char *c, c, return 0, return)
#endif
DEFINEFUNC(unsigned char *, ASN1_STRING_data, ASN1_STRING *a, a, return 0, return)
DEFINEFUNC(int, ASN1_STRING_length, ASN1_STRING *a, a, return 0, return)
DEFINEFUNC4(long, BIO_ctrl, BIO *a, a, int b, b, long c, c, void *d, d, return -1, return)
DEFINEFUNC(int, BIO_free, BIO *a, a, return 0, return)
DEFINEFUNC(BIO *, BIO_new, BIO_METHOD *a, a, return 0, return)
DEFINEFUNC2(BIO *, BIO_new_mem_buf, void *a, a, int b, b, return 0, return)
DEFINEFUNC3(int, BIO_read, BIO *a, a, void *b, b, int c, c, return -1, return)
DEFINEFUNC(BIO_METHOD *, BIO_s_mem, void, DUMMYARG, return 0, return)
DEFINEFUNC3(int, BIO_write, BIO *a, a, const void *b, b, int c, c, return -1, return)
DEFINEFUNC(int, BN_num_bits, const BIGNUM *a, a, return 0, return)
DEFINEFUNC(int, CRYPTO_num_locks, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, CRYPTO_set_locking_callback, void (*a)(int, int, const char *, int), a, return, DUMMYARG)
DEFINEFUNC(void, CRYPTO_set_id_callback, unsigned long (*a)(), a, return, DUMMYARG)
DEFINEFUNC(void, CRYPTO_free, void *a, a, return, DUMMYARG)
DEFINEFUNC(void, DSA_free, DSA *a, a, return, DUMMYARG)
#if  OPENSSL_VERSION_NUMBER < 0x00908000L
DEFINEFUNC3(X509 *, d2i_X509, X509 **a, a, unsigned char **b, b, long c, c, return 0, return)
#else // 0.9.8 broke SC and BC by changing this signature.
DEFINEFUNC3(X509 *, d2i_X509, X509 **a, a, const unsigned char **b, b, long c, c, return 0, return)
#endif
DEFINEFUNC2(char *, ERR_error_string, unsigned long a, a, char *b, b, return 0, return)
DEFINEFUNC(unsigned long, ERR_get_error, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const EVP_CIPHER *, EVP_des_ede3_cbc, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC3(int, EVP_PKEY_assign, EVP_PKEY *a, a, int b, b, char *c, c, return -1, return)
DEFINEFUNC(void, EVP_PKEY_free, EVP_PKEY *a, a, return, DUMMYARG)
DEFINEFUNC(DSA *, EVP_PKEY_get1_DSA, EVP_PKEY *a, a, return 0, return)
DEFINEFUNC(RSA *, EVP_PKEY_get1_RSA, EVP_PKEY *a, a, return 0, return)
DEFINEFUNC(EVP_PKEY *, EVP_PKEY_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(int, EVP_PKEY_type, int a, a, return NID_undef, return)
DEFINEFUNC2(int, i2d_X509, X509 *a, a, unsigned char **b, b, return -1, return)
DEFINEFUNC(const char *, OBJ_nid2sn, int a, a, return 0, return)
DEFINEFUNC(int, OBJ_obj2nid, const ASN1_OBJECT *a, a, return NID_undef, return)
#ifdef SSLEAY_MACROS
DEFINEFUNC6(void *, PEM_ASN1_read_bio, d2i_of_void *a, a, const char *b, b, BIO *c, c, void **d, d, pem_password_cb *e, e, void *f, f, return 0, return)
DEFINEFUNC6(void *, PEM_ASN1_write_bio, d2i_of_void *a, a, const char *b, b, BIO *c, c, void **d, d, pem_password_cb *e, e, void *f, f, return 0, return)
#else
DEFINEFUNC4(DSA *, PEM_read_bio_DSAPrivateKey, BIO *a, a, DSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC4(RSA *, PEM_read_bio_RSAPrivateKey, BIO *a, a, RSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC7(int, PEM_write_bio_DSAPrivateKey, BIO *a, a, DSA *b, b, const EVP_CIPHER *c, c, unsigned char *d, d, int e, e, pem_password_cb *f, f, void *g, g, return 0, return)
DEFINEFUNC7(int, PEM_write_bio_RSAPrivateKey, BIO *a, a, RSA *b, b, const EVP_CIPHER *c, c, unsigned char *d, d, int e, e, pem_password_cb *f, f, void *g, g, return 0, return)
#endif
DEFINEFUNC4(DSA *, PEM_read_bio_DSA_PUBKEY, BIO *a, a, DSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC4(RSA *, PEM_read_bio_RSA_PUBKEY, BIO *a, a, RSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC2(int, PEM_write_bio_DSA_PUBKEY, BIO *a, a, DSA *b, b, return 0, return)
DEFINEFUNC2(int, PEM_write_bio_RSA_PUBKEY, BIO *a, a, RSA *b, b, return 0, return)
DEFINEFUNC2(void, RAND_seed, const void *a, a, int b, b, return, DUMMYARG)
DEFINEFUNC(int, RAND_status, void, DUMMYARG, return -1, return)
DEFINEFUNC(void, RSA_free, RSA *a, a, return, DUMMYARG)
DEFINEFUNC(void, sk_free, STACK *a, a, return, DUMMYARG)
DEFINEFUNC(int, sk_num, STACK *a, a, return -1, return)
DEFINEFUNC2(char *, sk_value, STACK *a, a, int b, b, return 0, return)
DEFINEFUNC(int, SSL_accept, SSL *a, a, return -1, return)
DEFINEFUNC(int, SSL_clear, SSL *a, a, return -1, return)
DEFINEFUNC3(char *, SSL_CIPHER_description, SSL_CIPHER *a, a, char *b, b, int c, c, return 0, return)
DEFINEFUNC(int, SSL_connect, SSL *a, a, return -1, return)
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
// 0.9.8 broke SC and BC by changing this function's signature.
DEFINEFUNC(int, SSL_CTX_check_private_key, const SSL_CTX *a, a, return -1, return)
#else
DEFINEFUNC(int, SSL_CTX_check_private_key, SSL_CTX *a, a, return -1, return)
#endif
DEFINEFUNC4(long, SSL_CTX_ctrl, SSL_CTX *a, a, int b, b, long c, c, void *d, d, return -1, return)
DEFINEFUNC(void, SSL_CTX_free, SSL_CTX *a, a, return, DUMMYARG)
DEFINEFUNC(SSL_CTX *, SSL_CTX_new, SSL_METHOD *a, a, return 0, return)
DEFINEFUNC2(int, SSL_CTX_set_cipher_list, SSL_CTX *a, a, const char *b, b, return -1, return)
DEFINEFUNC(int, SSL_CTX_set_default_verify_paths, SSL_CTX *a, a, return -1, return)
DEFINEFUNC2(int, SSL_CTX_use_certificate, SSL_CTX *a, a, X509 *b, b, return -1, return)
DEFINEFUNC3(int, SSL_CTX_use_certificate_file, SSL_CTX *a, a, const char *b, b, int c, c, return -1, return)
DEFINEFUNC2(int, SSL_CTX_use_PrivateKey, SSL_CTX *a, a, EVP_PKEY *b, b, return -1, return)
DEFINEFUNC2(int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *a, a, RSA *b, b, return -1, return)
DEFINEFUNC3(int, SSL_CTX_use_PrivateKey_file, SSL_CTX *a, a, const char *b, b, int c, c, return -1, return)
DEFINEFUNC(void, SSL_free, SSL *a, a, return, DUMMYARG)
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
// 0.9.8 broke SC and BC by changing this function's signature.
DEFINEFUNC(STACK_OF(SSL_CIPHER) *, SSL_get_ciphers, const SSL *a, a, return 0, return)
#else
DEFINEFUNC(STACK_OF(SSL_CIPHER) *, SSL_get_ciphers, SSL *a, a, return 0, return)
#endif
DEFINEFUNC(SSL_CIPHER *, SSL_get_current_cipher, SSL *a, a, return 0, return)
DEFINEFUNC2(int, SSL_get_error, SSL *a, a, int b, b, return -1, return)
DEFINEFUNC(STACK_OF(X509) *, SSL_get_peer_cert_chain, SSL *a, a, return 0, return)
DEFINEFUNC(X509 *, SSL_get_peer_certificate, SSL *a, a, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
// 0.9.8 broke SC and BC by changing this function's signature.
DEFINEFUNC(long, SSL_get_verify_result, const SSL *a, a, return -1, return)
#else
DEFINEFUNC(long, SSL_get_verify_result, SSL *a, a, return -1, return)
#endif
DEFINEFUNC(int, SSL_library_init, void, DUMMYARG, return -1, return)
DEFINEFUNC(void, SSL_load_error_strings, void, DUMMYARG, return, DUMMYARG)
DEFINEFUNC(SSL *, SSL_new, SSL_CTX *a, a, return 0, return)
DEFINEFUNC3(int, SSL_read, SSL *a, a, void *b, b, int c, c, return -1, return)
DEFINEFUNC3(void, SSL_set_bio, SSL *a, a, BIO *b, b, BIO *c, c, return, DUMMYARG)
DEFINEFUNC(void, SSL_set_accept_state, SSL *a, a, return, DUMMYARG)
DEFINEFUNC(void, SSL_set_connect_state, SSL *a, a, return, DUMMYARG)
DEFINEFUNC(int, SSL_shutdown, SSL *a, a, return -1, return)
DEFINEFUNC(SSL_METHOD *, SSLv2_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, SSLv3_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, SSLv23_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, TLSv1_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, SSLv2_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, SSLv3_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, SSLv23_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, TLSv1_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC3(int, SSL_write, SSL *a, a, const void *b, b, int c, c, return -1, return)
DEFINEFUNC2(int, X509_cmp, X509 *a, a, X509 *b, b, return -1, return)
#ifndef SSLEAY_MACROS
DEFINEFUNC(X509 *, X509_dup, X509 *a, a, return 0, return)
#endif
DEFINEFUNC(ASN1_OBJECT *, X509_EXTENSION_get_object, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(void, X509_free, X509 *a, a, return, DUMMYARG)
DEFINEFUNC2(X509_EXTENSION *, X509_get_ext, X509 *a, a, int b, b, return 0, return)
DEFINEFUNC(int, X509_get_ext_count, X509 *a, a, return 0, return)
DEFINEFUNC4(void *, X509_get_ext_d2i, X509 *a, a, int b, b, int *c, c, int *d, d, return 0, return)
DEFINEFUNC(X509_NAME *, X509_get_issuer_name, X509 *a, a, return 0, return)
DEFINEFUNC(X509_NAME *, X509_get_subject_name, X509 *a, a, return 0, return)
DEFINEFUNC(int, X509_verify_cert, X509_STORE_CTX *a, a, return -1, return)
DEFINEFUNC3(char *, X509_NAME_oneline, X509_NAME *a, a, char *b, b, int c, c, return 0, return)
DEFINEFUNC(EVP_PKEY *, X509_PUBKEY_get, X509_PUBKEY *a, a, return 0, return)
DEFINEFUNC(void, X509_STORE_free, X509_STORE *a, a, return, DUMMYARG)
DEFINEFUNC(X509_STORE *, X509_STORE_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC2(int, X509_STORE_add_cert, X509_STORE *a, a, X509 *b, b, return 0, return)
DEFINEFUNC(void, X509_STORE_CTX_free, X509_STORE_CTX *a, a, return, DUMMYARG)
DEFINEFUNC4(int, X509_STORE_CTX_init, X509_STORE_CTX *a, a, X509_STORE *b, b, X509 *c, c, STACK_OF(X509) *d, d, return -1, return)
DEFINEFUNC2(int, X509_STORE_CTX_set_purpose, X509_STORE_CTX *a, a, int b, b, return -1, return)
DEFINEFUNC(X509_STORE_CTX *, X509_STORE_CTX_new, DUMMYARG, DUMMYARG, return 0, return)
#ifdef SSLEAY_MACROS
DEFINEFUNC2(int, i2d_DSAPrivateKey, const DSA *a, a, unsigned char **b, b, return -1, return)
DEFINEFUNC2(int, i2d_RSAPrivateKey, const RSA *a, a, unsigned char **b, b, return -1, return)
DEFINEFUNC3(RSA *, d2i_RSAPrivateKey, RSA **a, a, unsigned char **b, b, long c, c, return 0, return)
DEFINEFUNC3(DSA *, d2i_DSAPrivateKey, DSA **a, a, unsigned char **b, b, long c, c, return 0, return)
#endif

#ifdef Q_OS_WIN
#define RESOLVEFUNC(func) \
    if (!(_q_##func = _q_PTR_##func(ssleay32.resolve(#func)))     \
        && !(_q_##func = _q_PTR_##func(libeay32.resolve(#func)))) \
        qWarning("QSslSocket: cannot resolve "#func);
#else
#define RESOLVEFUNC(func) \
    if (!(_q_##func = _q_PTR_##func(libssl.resolve(#func)))     \
        && !(_q_##func = _q_PTR_##func(libcrypto.resolve(#func)))) \
        qWarning("QSslSocket: cannot resolve "#func);
#endif

#ifdef QT_SHARED

#ifdef QT_NO_LIBRARY
bool q_resolveOpenSslSymbols()
{
    qWarning("QSslSocket: unable to resolve symbols. "
             "QT_NO_LIBRARY is defined which means runtime resolving of "
             "libraries won't work.");
    qWarning("Either compile Qt staticly or with support for runtime resolving "
             "of libraries.");
    return false;
}
#else

bool q_resolveOpenSslSymbols()
{
    static volatile bool symbolsResolved = false;
    static volatile bool triedToResolveSymbols = false;
#ifndef QT_NO_THREAD
    QMutexLocker locker(QMutexPool::globalInstanceGet((void *)&q_SSL_library_init));
#endif
    if (symbolsResolved)
        return true;
    if (triedToResolveSymbols)
        return false;
    triedToResolveSymbols = true;

#ifdef Q_OS_WIN
    QLibrary ssleay32(QLatin1String("ssleay32"));
    if (!ssleay32.load()) {
        // Cannot find ssleay32.dll
        qWarning("QSslSocket: cannot find ssleay32 library: %s.",
                 qPrintable(ssleay32.errorString()));
        return false;
    }

    QLibrary libeay32(QLatin1String("libeay32"));
    if (!libeay32.load()) {
        // Cannot find libeay32.dll
        qWarning("QSslSocket: cannot find libeay32 library: %s.",
                 qPrintable(libeay32.errorString()));
        return false;
    }
#else
    QLibrary libssl(QLatin1String("ssl"));
    if (!libssl.load()) {
        // Cannot find libssl
        qWarning("QSslSocket: cannot find ssl library: %s.",
                 qPrintable(libssl.errorString()));
        return false;
    }

    QLibrary libcrypto(QLatin1String("crypto"));
    if (!libcrypto.load()) {
        // Cannot find libcrypto
        qWarning("QSslSocket: cannot find crypto library: %s.",
                 qPrintable(libcrypto.errorString()));
        return false;
    }
#endif

#ifdef SSLEAY_MACROS
    RESOLVEFUNC(ASN1_dup)
#endif
    RESOLVEFUNC(ASN1_STRING_data)
    RESOLVEFUNC(ASN1_STRING_length)
    RESOLVEFUNC(BIO_ctrl)
    RESOLVEFUNC(BIO_free)
    RESOLVEFUNC(BIO_new)
    RESOLVEFUNC(BIO_new_mem_buf)
    RESOLVEFUNC(BIO_read)
    RESOLVEFUNC(BIO_s_mem)
    RESOLVEFUNC(BIO_write)
    RESOLVEFUNC(BN_num_bits)
    RESOLVEFUNC(CRYPTO_free)
    RESOLVEFUNC(CRYPTO_num_locks)
    RESOLVEFUNC(CRYPTO_set_id_callback)
    RESOLVEFUNC(CRYPTO_set_locking_callback)
    RESOLVEFUNC(DSA_free)
    RESOLVEFUNC(ERR_error_string)
    RESOLVEFUNC(ERR_get_error)
    RESOLVEFUNC(EVP_des_ede3_cbc)
    RESOLVEFUNC(EVP_PKEY_assign)
    RESOLVEFUNC(EVP_PKEY_free)
    RESOLVEFUNC(EVP_PKEY_get1_DSA)
    RESOLVEFUNC(EVP_PKEY_get1_RSA)
    RESOLVEFUNC(EVP_PKEY_new)
    RESOLVEFUNC(EVP_PKEY_type)
    RESOLVEFUNC(OBJ_nid2sn)
    RESOLVEFUNC(OBJ_obj2nid)
#ifdef SSLEAY_MACROS // ### verify
    RESOLVEFUNC(PEM_ASN1_read_bio)
#else
    RESOLVEFUNC(PEM_read_bio_DSAPrivateKey)
    RESOLVEFUNC(PEM_read_bio_RSAPrivateKey)
    RESOLVEFUNC(PEM_write_bio_DSAPrivateKey)
    RESOLVEFUNC(PEM_write_bio_RSAPrivateKey)
#endif
    RESOLVEFUNC(PEM_read_bio_DSA_PUBKEY)
    RESOLVEFUNC(PEM_read_bio_RSA_PUBKEY)
    RESOLVEFUNC(PEM_write_bio_DSA_PUBKEY)
    RESOLVEFUNC(PEM_write_bio_RSA_PUBKEY)
    RESOLVEFUNC(RAND_seed)
    RESOLVEFUNC(RAND_status)
    RESOLVEFUNC(RSA_free)
    RESOLVEFUNC(sk_free)
    RESOLVEFUNC(sk_num)
    RESOLVEFUNC(sk_value)
    RESOLVEFUNC(SSL_CIPHER_description)
    RESOLVEFUNC(SSL_CTX_check_private_key)
    RESOLVEFUNC(SSL_CTX_ctrl)
    RESOLVEFUNC(SSL_CTX_free)
    RESOLVEFUNC(SSL_CTX_new)
    RESOLVEFUNC(SSL_CTX_set_cipher_list)
    RESOLVEFUNC(SSL_CTX_set_default_verify_paths)
    RESOLVEFUNC(SSL_CTX_use_certificate)
    RESOLVEFUNC(SSL_CTX_use_certificate_file)
    RESOLVEFUNC(SSL_CTX_use_PrivateKey)
    RESOLVEFUNC(SSL_CTX_use_RSAPrivateKey)
    RESOLVEFUNC(SSL_CTX_use_PrivateKey_file)
    RESOLVEFUNC(SSL_accept)
    RESOLVEFUNC(SSL_clear)
    RESOLVEFUNC(SSL_connect)
    RESOLVEFUNC(SSL_free)
    RESOLVEFUNC(SSL_get_ciphers)
    RESOLVEFUNC(SSL_get_current_cipher)
    RESOLVEFUNC(SSL_get_error)
    RESOLVEFUNC(SSL_get_peer_cert_chain)
    RESOLVEFUNC(SSL_get_peer_certificate)
    RESOLVEFUNC(SSL_get_verify_result)
    RESOLVEFUNC(SSL_library_init)
    RESOLVEFUNC(SSL_load_error_strings)
    RESOLVEFUNC(SSL_new)
    RESOLVEFUNC(SSL_read)
    RESOLVEFUNC(SSL_set_accept_state)
    RESOLVEFUNC(SSL_set_bio)
    RESOLVEFUNC(SSL_set_connect_state)
    RESOLVEFUNC(SSL_shutdown)
    RESOLVEFUNC(SSL_write)
    RESOLVEFUNC(SSLv2_client_method)
    RESOLVEFUNC(SSLv3_client_method)
    RESOLVEFUNC(SSLv23_client_method)
    RESOLVEFUNC(TLSv1_client_method)
    RESOLVEFUNC(SSLv2_server_method)
    RESOLVEFUNC(SSLv3_server_method)
    RESOLVEFUNC(SSLv23_server_method)
    RESOLVEFUNC(TLSv1_server_method)
    RESOLVEFUNC(X509_NAME_oneline)
    RESOLVEFUNC(X509_PUBKEY_get)
    RESOLVEFUNC(X509_STORE_free)
    RESOLVEFUNC(X509_STORE_new)
    RESOLVEFUNC(X509_STORE_add_cert)
    RESOLVEFUNC(X509_STORE_CTX_free)
    RESOLVEFUNC(X509_STORE_CTX_init)
    RESOLVEFUNC(X509_STORE_CTX_new)
    RESOLVEFUNC(X509_STORE_CTX_set_purpose)
    RESOLVEFUNC(X509_cmp)
#ifndef SSLEAY_MACROS
    RESOLVEFUNC(X509_dup)
#endif
    RESOLVEFUNC(X509_EXTENSION_get_object)
    RESOLVEFUNC(X509_free)
    RESOLVEFUNC(X509_get_ext)
    RESOLVEFUNC(X509_get_ext_count)
    RESOLVEFUNC(X509_get_ext_d2i)
    RESOLVEFUNC(X509_get_issuer_name)
    RESOLVEFUNC(X509_get_subject_name)
    RESOLVEFUNC(X509_verify_cert)
    RESOLVEFUNC(d2i_X509)
    RESOLVEFUNC(i2d_X509)
#ifdef SSLEAY_MACROS
    RESOLVEFUNC(i2d_DSAPrivateKey)
    RESOLVEFUNC(i2d_RSAPrivateKey)
    RESOLVEFUNC(d2i_DSAPrivateKey)
    RESOLVEFUNC(d2i_RSAPrivateKey)
#endif
    symbolsResolved = true;
    return true;
}
#endif // QT_NO_LIBRARY

#else
bool q_resolveOpenSslSymbols()
{
#ifdef QT_NO_SSL
    return false;
#endif
    return true;
}
#endif

//==============================================================================
// contributed by Jay Case of Sarvega, Inc.; http://sarvega.com/
// Based on X509_cmp_time() for intitial buffer hacking.
//==============================================================================
time_t q_getTimeFromASN1(const ASN1_TIME *aTime)
{
    time_t lResult = 0;

    char lBuffer[24];
    char *pBuffer = lBuffer;

    size_t lTimeLength = aTime->length;
    char *pString = (char *) aTime->data;

    if (aTime->type == V_ASN1_UTCTIME) {
        if ((lTimeLength < 11) || (lTimeLength > 17))
            return 0;

        memcpy(pBuffer, pString, 10);
        pBuffer += 10;
        pString += 10;
    } else {
        if (lTimeLength < 13)
            return 0;

        memcpy(pBuffer, pString, 12);
        pBuffer += 12;
        pString += 12;
    }

    if ((*pString == 'Z') || (*pString == '-') || (*pString == '+')) {
        *pBuffer++ = '0';
        *pBuffer++ = '0';
    } else {
        *pBuffer++ = *pString++;
        *pBuffer++ = *pString++;
        // Skip any fractional seconds...
        if (*pString == '.') {
            pString++;
            while ((*pString >= '0') && (*pString <= '9'))
                pString++;
        }
    }

    *pBuffer++ = 'Z';
    *pBuffer++ = '\0';

    time_t lSecondsFromUCT;
    if (*pString == 'Z') {
        lSecondsFromUCT = 0;
    } else {
        if ((*pString != '+') && (pString[5] != '-'))
            return 0;

        lSecondsFromUCT = ((pString[1] - '0') * 10 + (pString[2] - '0')) * 60;
        lSecondsFromUCT += (pString[3] - '0') * 10 + (pString[4] - '0');
        if (*pString == '-')
            lSecondsFromUCT = -lSecondsFromUCT;
    }

    tm lTime;
    lTime.tm_sec = ((lBuffer[10] - '0') * 10) + (lBuffer[11] - '0');
    lTime.tm_min = ((lBuffer[8] - '0') * 10) + (lBuffer[9] - '0');
    lTime.tm_hour = ((lBuffer[6] - '0') * 10) + (lBuffer[7] - '0');
    lTime.tm_mday = ((lBuffer[4] - '0') * 10) + (lBuffer[5] - '0');
    lTime.tm_mon = (((lBuffer[2] - '0') * 10) + (lBuffer[3] - '0')) - 1;
    lTime.tm_year = ((lBuffer[0] - '0') * 10) + (lBuffer[1] - '0');
    if (lTime.tm_year < 50)
        lTime.tm_year += 100; // RFC 2459
    lTime.tm_wday = 0;
    lTime.tm_yday = 0;
    lTime.tm_isdst = 0;  // No DST adjustment requested

    lResult = mktime(&lTime);
    if ((time_t)-1 != lResult) {
        if (0 != lTime.tm_isdst)
            lResult -= 3600;  // mktime may adjust for DST  (OS dependent)
        lResult += lSecondsFromUCT;
    } else {
        lResult = 0;
    }

    return lResult;
}
