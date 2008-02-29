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

#include "qkey.h"

#include <QDebug>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/dh.h>

class QKeyPrivate
{
public:
    QKeyPrivate()
        : type( QKey::Unknown ), keySize( 0 ), embeddedKey( false )
    {
    }
    virtual ~QKeyPrivate() {}

    bool parsePrivateKey()
    {
        FILE* fp = fopen( keyFile.file().toLocal8Bit().constData(), "r" );
        if ( !fp ) {
            perror("QKeyPrivate::parsePrivateKey()");
            return false;
        }

        EVP_PKEY* key = PEM_read_PrivateKey( fp, NULL, 0, NULL );
        fclose( fp );
        if ( key == NULL )
            return false;

        switch( EVP_PKEY_type(key->type) ) {
            case EVP_PKEY_RSA:
                algorithm = "RSA";
                break;
            case EVP_PKEY_DSA:
                algorithm = "DSA";
                break;
            case EVP_PKEY_DH:
                algorithm = "DH";
                break;
            case EVP_PKEY_EC:
                algorithm = "EC";
                break;
            default:
                qWarning() << "Unknown type: " << key->type;
                EVP_PKEY_free( key );
                return false;
        }
        keySize = EVP_PKEY_bits( key );
        type = QKey::Private;
        EVP_PKEY_free( key );
        return true;
    }

    bool parseRSAPublicKey()
    {
        FILE* fp = fopen( keyFile.file().toLocal8Bit().constData(), "r" );
        if ( !fp ) {
            perror("QKeyPrivate::parseRSAPublicKey()");
            return false;
        }
        RSA* pubKey = PEM_read_RSAPublicKey( fp, NULL, 0, NULL );
        fclose( fp );
        if ( pubKey == NULL )
            return false;

        keySize = RSA_size( pubKey );
        type = QKey::Public;
        algorithm = "RSA";
        RSA_free( pubKey );
        return true;
    }

    bool parseDHParameter()
    {
        FILE* fp = fopen( keyFile.file().toLocal8Bit().constData(), "r" );
        if ( !fp ) {
            perror("QKeyPrivate::parseDHParameter()");
            return false;
        }
        DH* params = PEM_read_DHparams( fp, NULL, 0, NULL );
        fclose( fp );
        if ( params == NULL )
            return false;

        keySize = BN_num_bits( params->p );
        type = QKey::Parameter;
        algorithm = "DH";
        DH_free( params );

        return true;
    }

    bool parseDSAParameter()
    {
#ifndef OPENSSL_NO_DSA
        BIO* bin = BIO_new( BIO_s_file() );
        if ( bin == NULL ) {
            perror("QKeyPrivate::parseDSAParamter");
            return false;
        }
        if ( BIO_read_filename( bin, keyFile.file().toLocal8Bit().constData() ) <= 0 ) {
            BIO_free( bin );
            return false;
        }
        DSA* params = PEM_read_bio_DSAparams( bin, NULL, 0, NULL );
        if ( params == NULL )
            return false;

        keySize = BN_num_bits( params->p );
        type = QKey::Parameter;
        algorithm = "DSA";
        DSA_free( params );
        BIO_free( bin );

        return true;
#else
        return false;
#endif
    }



    QKey::Type type;
    int keySize;
    QContent keyFile;
    QString algorithm;
    bool embeddedKey;
};

QKey::QKey( QObject* parent )
    : QObject( parent )
{
    d = new QKeyPrivate;
}

QKey::QKey( const QContent& c, QObject* parent )
    : QObject( parent )
{
    d = new QKeyPrivate;
    d->keyFile = c;

    if ( d->parsePrivateKey() )
        return;

    if ( d->parseRSAPublicKey() )
        return;

    if ( d->parseDHParameter() )
        return;

    if ( d->parseDSAParameter() )
        return;

}

QKey::QKey( const QKey& other )
    : QObject()
{
    d = new QKeyPrivate;
    (*this) = other; //use assignment operator
}

QKey::~QKey()
{
    delete d;
    d = 0;
}

QKey& QKey::operator=( const QKey& other )
{
    d->keySize = other.d->keySize;
    d->keyFile = other.d->keyFile;
    d->algorithm = other.d->algorithm;
    d->type = other.d->type;
    d->embeddedKey = other.d->embeddedKey;
    return (*this);
}

QString QKey::algorithm() const
{
    return d->algorithm;
}

int QKey::keySize() const
{
    return d->keySize;
}

QKey::Type QKey::type() const
{
    return d->type;
}

QContent QKey::file() const
{
    return d->keyFile;
}

//==============================================================================
// contributed by Jay Case of Sarvega, Inc.; http://sarvega.com/
// Based on X509_cmp_time() for intitial buffer hacking.
//==============================================================================
static time_t getTimeFromASN1(const ASN1_TIME *aTime)
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

class QCertificatePrivate
{
public:
    QCertificatePrivate()
        : validCertificate( false )
    {
    }

    virtual ~QCertificatePrivate() {}

    bool parseX509()
    {
        FILE* fp = fopen( certFile.file().toLocal8Bit().constData(), "r" );
        if ( !fp ) {
            perror("QCertificatePrivate::parseX509()");
            return false;
        }

        X509* cert = PEM_read_X509(fp, NULL, 0, NULL );
        fclose( fp );
        if ( !cert )
            return false;

        char * subjectName = X509_NAME_oneline( X509_get_subject_name(cert), 0, 0 );
        QString subject = subjectName;
        subject.remove(0,1); // format returned by openssl not rfc2253 complaint: /C=NO/L=Oslo
        subject.replace(QChar('/'),QChar(','));
        issuedTo = QX500Identity( subject );
        OPENSSL_free( subjectName );

        char *issuerName = X509_NAME_oneline( X509_get_issuer_name(cert), 0, 0 );
        QString issuer = issuerName;
        issuer.remove(0,1);
        issuer.replace(QChar('/'),QChar(','));
        issuedBy = QX500Identity( issuer );
        OPENSSL_free( issuerName );

        notbefore.setTime_t( getTimeFromASN1( X509_get_notBefore( cert ) ) );
        notafter.setTime_t( getTimeFromASN1( X509_get_notAfter( cert ) ) );

        X509_free( cert );
        validCertificate = true;
        return true;
    }

    QKey publicKey()
    {
        QKey result;
        FILE* fp = fopen( certFile.file().toLocal8Bit().constData(), "r" );
        if ( !fp ) {
            perror("QCertificatePrivate::publicKey()");
            return result;
        }

        X509* cert = PEM_read_X509(fp, NULL, 0, NULL );
        fclose( fp );
        if ( !cert )
            return result;

        bool error = false;
        QString algo;
        EVP_PKEY* key = X509_get_pubkey( cert );
        if ( key != NULL ) {
             switch( EVP_PKEY_type(key->type) ) {
                case EVP_PKEY_RSA:
                    algo = "RSA";
                    break;
                case EVP_PKEY_DSA:
                    algo = "DSA";
                    break;
                case EVP_PKEY_DH:
                    algo = "DH";
                    break;
                case EVP_PKEY_EC:
                    algo = "EC";
                    break;
                default:
                    qWarning() << "QCertificatePrivate::parseX509(): Unknown type " << key->type;
                    error = true;
            }
            result.d->keySize = EVP_PKEY_bits( key );
            result.d->algorithm = algo;
            result.d->embeddedKey = true;
            EVP_PKEY_free( key );
        }

        X509_free( cert );

        if ( !error ) {
            result.d->type = QKey::Public;
            result.d->keyFile = certFile;
        }
        return result;
    }

    QContent certFile;
    QX500Identity issuedBy;
    QX500Identity issuedTo;
    QDateTime notbefore;
    QDateTime notafter;
    bool validCertificate;
};

QCertificate::QCertificate( const QContent& c, QObject* parent )
    : QObject( parent )
{
    d = new QCertificatePrivate;
    d->certFile = c;

    d->parseX509();
}

QCertificate::QCertificate( const QCertificate& other )
    : QObject()
{
    d = new QCertificatePrivate;
    (*this) = other; //use assignment operator
}

QCertificate::~QCertificate()
{
    delete d;
    d = 0;
}

QCertificate& QCertificate::operator=( const QCertificate& other )
{
    d->certFile = other.d->certFile;
    d->issuedBy = other.d->issuedBy;
    d->issuedTo = other.d->issuedTo;
    d->notbefore = other.d->notbefore;
    d->notafter = other.d->notafter;
    d->validCertificate = other.d->validCertificate;
    return (*this);
}

bool QCertificate::checkValidity( const QDateTime& dt ) const
{
    if ( dt < d->notafter && dt > d->notbefore )
        return true;

    return false;
}

QDateTime QCertificate::notValidAfter() const
{
    return d->notafter;
}

QDateTime QCertificate::notValidBefore() const
{
    return d->notbefore;
}

QString QCertificate::certificateType() const
{
    return QLatin1String("X509");
}

/*!
  Returns the public key attached to the certificate.
  By default this certificate is the parent for the returned QObject.
  */
QKey QCertificate::publicKey() const
{
    if ( !isValid() )
        return QKey();

    return d->publicKey();
}

bool QCertificate::isValid() const {
    return d->validCertificate;
}

QX500Identity QCertificate::issuer() const
{
    return d->issuedBy;
}

QX500Identity QCertificate::subject() const
{
    return d->issuedTo;
}
