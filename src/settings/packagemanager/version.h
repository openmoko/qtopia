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

#ifndef VERSION_H
#define VERSION_H

#include <qstring.h>

class Version
{
    public:
        Version( const QString & );
        bool operator< ( const Version &other ) const;
        bool operator== ( const Version &other ) const;
        bool operator<= ( const Version &other ) const;

    private:
        QString m_version;

};

class VersionUtil
{
public:
    static bool checkVersion( const QString &versionRanges );
private:
    static bool checkVersionItem( const QString &versionItem );
};


#endif
