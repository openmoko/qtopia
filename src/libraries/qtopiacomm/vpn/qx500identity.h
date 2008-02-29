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

#ifndef Q_X500_IDENTITY_H
#define Q_X500_IDENTITY_H

#include <QString>
#include <QHash>
#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT QX500Identity {
public:
    enum Attribute {            // see RFC 1779
        CommonName = 0,         // CN
        LocalityName,           // L
        StateOrProvinceName,    // ST
        OrganizationName,       // O
        OrganizationalUnitName, // OU
        CountryName,            // C
        StreetAddress,          // STREET
        DomainComponent,        // DC
        UserID,                 // UID
        OID                     // OID
    };

    explicit QX500Identity( const QString& identifier = QString() );
    QX500Identity( const QX500Identity& other );

    QX500Identity& operator=( const QX500Identity& other );

    QString value( Attribute tag ) const;
    QString toString() const;

private:
    void parseX500( const QString& ident );

    QString oids;
    QHash<Attribute, QString> map;
};

#endif //Q_X500_IDENTITY_H
