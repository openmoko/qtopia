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

#include <QMap>

QX500Identity::QX500Identity( const QString& identifier )
{
    parseX500( identifier );
}

QString QX500Identity::value( Attribute tag ) const
{
    if ( tag == OID )
        return oids;
    return map.value( tag );
}

QString QX500Identity::toString() const
{
    QString result;
    static QMap<Attribute,QString> atts;
    if ( !atts.count() ) {
        atts.insert( CountryName, "C=");
        atts.insert( StateOrProvinceName, "ST=" );
        atts.insert( LocalityName, "L=" );
        atts.insert( OrganizationName, "O=" );
        atts.insert( OrganizationalUnitName, "OU=" );
        atts.insert( CommonName, "CN=" );
        atts.insert( StreetAddress, "STREET=" );
        atts.insert( DomainComponent, "DC=" );
        atts.insert( UserID, "UID=" );
    }

    QString value;
    QList<Attribute> types = atts.keys();
    foreach ( Attribute a, types ) {
        value = map.value( a );
        if ( value.isEmpty() )
            continue;
        if ( !result.isEmpty() )
            result.append(",");
        result.append( atts.value( a )+ value );
    }

    if ( !oids.isEmpty() ) {
        result.append(","+oids);
    }

    return result;
}



void QX500Identity::parseX500( const QString& ident )
{
    //very very limited x500 parser
    QList<QString> attributes;
    bool inQuote = false;
    int tokenStart=0;
    for( int i = 0; i< ident.length(); i++ ) {
        if ( ( ident[i] ) == QChar('"') ) {
            inQuote = !inQuote;
            continue;
        }
        if ( inQuote )  //ignore quoted chars
            continue;
        if ( ident[i] == QChar(',') ) {
            attributes.append( ident.mid( tokenStart, i-tokenStart ) );
            tokenStart = i+1;
        }
    }

    foreach( QString attr, attributes ) {
        int equalsPos = attr.indexOf(QChar('='));
        if ( equalsPos == -1 )
            continue;
        QString type = attr.left( equalsPos );
        QString value = attr.mid( equalsPos+1 );

        if ( type == "CN" ) {
            map.insert( CommonName, value );
        } else if ( type == "L") {
            map.insert( LocalityName, value );
        } else if ( type == "ST") {
            map.insert( StateOrProvinceName, value );
        } else if ( type == "O") {
            map.insert( OrganizationName, value );
        } else if ( type == "OU") {
            map.insert( OrganizationalUnitName, value );
        } else if ( type == "C") {
            map.insert( CountryName, value );
        } else if ( type == "STREET") {
            map.insert( StreetAddress, value );
        } else if ( type == "DC") {
            map.insert( DomainComponent, value );
        } else if ( type == "UID") {
            map.insert( UserID, value );
        } else {  //must be oid
            if ( !oids.isEmpty() )
                oids.append(",");
            oids.append(attr);
        }
    }
}

QX500Identity::QX500Identity( const QX500Identity& other )
{
    (*this) = other; //use assignment operator
}

QX500Identity& QX500Identity::operator=( const QX500Identity& other )
{
    if ( this == &other )
        return *this;

    oids = other.oids;
    map = other.map;
    return (*this);
}

