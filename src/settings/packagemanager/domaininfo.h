/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef DOMAININFO_H
#define DOMAININFO_H

#include <QString>
#include <QStringList>

class DomainInfo
{
    private:
        DomainInfo();
        QStringList m_domainList;
        QStringList m_sensitiveDomains;
        QString m_defaultDomain;
        static const char *domainStrings[];
        static DomainInfo& getInstance();

public:
    static QString defaultDomain();
    static bool isDomainValid( const QString & );
    static bool hasSensitiveDomains( const QString & );
    static QString explain( const QString &dom, const QString &packageName );
    static QString getWarningResource( const QString & );
};

#endif
