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
#include "version.h"
#include <qstringlist.h>
#include <qtopianamespace.h>

Version::Version( const QString &version )
{
    m_version = version;
}

bool Version::operator< ( const Version& other ) const
{
    int thisV = 0, otherV = 0, thisD, otherD;
    int thisPtr = 0, otherPtr = 0;
    int majority = 3;

    bool hitNumThis = false, hitNumOther = false;
    while ( majority > 0 )
    {
        if( !hitNumThis )
        {
            while ( thisPtr < m_version.count() && ( thisD = m_version[thisPtr++].digitValue() ) != -1 )
            {
                thisV = thisV * 10 + thisD;
                hitNumThis = true;
            }
            if ( thisPtr == m_version.count() )
                hitNumThis = true;
        }
        if (!hitNumOther )
        {
            while ( otherPtr < other.m_version.count() && ( otherD = other.m_version[otherPtr++].digitValue() ) != -1 )
            {
                otherV = otherV * 10 + otherD;
                hitNumOther = true;
            }
            if ( otherPtr == other.m_version.count() )
                hitNumOther = true;
        }

        if ( hitNumThis && hitNumOther )
        {
            if ( thisV != otherV )
                return ( thisV < otherV );
            
            majority --;
            thisV = otherV = 0; 
            hitNumThis = hitNumOther = false;
        }
    }
    return false;
}

bool Version::operator== ( const Version &other ) const
{
    if ( (*this).m_version == other.m_version ) 
        return true;
    else //TODO: comparison could be optimised 
        return ( !(*this < other) &&  !( other < *this ) ); 
}

bool Version::operator<=( const Version &other ) const
{
    return ( (*this == other) || (*this < other) );     
}

bool VersionUtil::checkVersion( const QString &versionRanges)
{
    QString versRange (versionRanges );
    versRange.remove(" ");
    QStringList versionItems = versRange.split(",");

    foreach( QString versionItem, versionItems )
    {
        if ( checkVersionItem( versionItem ) )
            return true;
    }
    return false;
}

bool VersionUtil::checkVersionItem( const QString &versionItem )
{
    QStringList versionElements;
    Version qtopiaCurVersion( Qtopia::version() );
    if ( versionItem.contains("-") )
    {
        versionElements = versionItem.split("-");
        if ( versionElements.count() != 2 )
            return false;
        else
        {
            Version minVersion( versionElements[0] );
            Version maxVersion( versionElements[1] );

            if ( minVersion <= qtopiaCurVersion && qtopiaCurVersion <= maxVersion )
                return true;
            else
                return false;
        }
    }
    else
        return ( qtopiaCurVersion == versionItem );
        
}


