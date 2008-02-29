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

#include "qx500identity.h"
#include <QObject>

#include <qcontent.h>

class QCertificate;
class QKeyPrivate;
class QKey : QObject
{
    Q_OBJECT //for tr
public:
    enum Type {
        Private = 0,
        Public = 1,
        Parameter = 2,
        Unknown = 50
    };
    explicit QKey( QObject* parent = 0 );
    QKey( const QKey& other );
    explicit QKey( const QContent& c, QObject* parent = 0 );
    virtual ~QKey();

    QString algorithm() const;
    int keySize() const;
    Type type() const;
    QContent file() const;
    QKey& operator=( const QKey& other );

private:
    friend class QCertificatePrivate;
    QKeyPrivate* d;
};

class QCertificatePrivate;
class QCertificate : public QObject {
    Q_OBJECT //for tr()
public:
    explicit QCertificate( const QContent& c, QObject* parent = 0 );
    QCertificate( const QCertificate& other );
    virtual ~QCertificate();

    QCertificate& operator=( const QCertificate& other );

    bool checkValidity( const QDateTime& dt ) const;
    QDateTime notValidAfter() const;
    QDateTime notValidBefore() const;

    QX500Identity issuer() const;
    QX500Identity subject() const;

    QString certificateType() const;
    QKey publicKey() const;
    QContent file() const;
    bool isValid() const;

protected:
    QCertificatePrivate* d;
};
