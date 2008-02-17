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

#include "domaininfo.h"

#include <QCoreApplication>
#include <QSettings>
#include <Qtopia>

//all installable domains MUST be placed here.
const char *DomainInfo::domainStrings[] = {
    "trusted",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">requests <b>unrestricted access</b> on your device</font>" ),
    "untrusted",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">requests <b>minimal access priveleges</b> on your device</font>" ),
    0,
    0
};

/*! \internal
  Initializes the list of senstive domains and the default domain
*/
DomainInfo::DomainInfo()
{
    const char **key = domainStrings;
    for ( ; *key; key++,key++ )
        m_domainList.append( QString((char*)(*key)));

    QSettings conf( Qtopia::qtopiaDir() + "etc/default/Trolltech/PackageManager.conf", QSettings::IniFormat );
    conf.beginGroup("Configuration");

    if ( conf.value("SensitiveDomains","none").toString() != "none" )
        m_sensitiveDomains = conf.value("SensitiveDomains").toStringList();

    m_defaultDomain = conf.value("DefaultDomain","untrusted").toString();
}

DomainInfo& DomainInfo::getInstance()
{
    static DomainInfo domainInfo;
    return domainInfo;
}

QString DomainInfo::defaultDomain()
{
    return DomainInfo::getInstance().m_defaultDomain;
}

/*! \internal
  Returns whether the given \a domain is recognised.
  A sensitive domain is condsidered a valid domain,
  even though it cannot be installed by packagemanager.
*/
bool DomainInfo::isDomainValid( const QString &domain )
{
    return DomainInfo::getInstance().m_domainList.contains( domain );
}

/*! \internal
  Returns whether the list of \a domains contains any sensitive
  domains.  A downloaded application which requests a
  sensitive domain cannot be installed.
*/
bool DomainInfo::hasSensitiveDomains( const QString &domains )
{
    QStringList doms = domains.split( QLatin1Char( ',' ), QString::SkipEmptyParts );
    foreach( QString dom, doms )
    {
        if ( DomainInfo::getInstance().m_sensitiveDomains.contains(dom) )
            return true;
    }
    return false;
}


/*! \internal
  Returns a full explanation of what \a packageName is capable of
  doing based on the domain(s) specified in \a dom
*/
QString DomainInfo::explain( const QString &dom, const QString &packageName )
{
    QStringList domList = dom.split( ',', QString::SkipEmptyParts );
    if ( dom.isEmpty() )
        domList << "none";
    if ( packageName.isEmpty() )
    {
        qWarning( "Tried to post domain update %s, but current row invalid",
                qPrintable(dom) );
        return QString();
    }
    QString html = packageName;
    QString conjunction = " ";
    QString badDomainWarning;
    for ( int i = 0; i < domList.count(); i++ )
    {
        if ( domList[i].isEmpty() ) continue;
        QString res = getWarningResource( domList[i] );
        if ( res.isEmpty() )
        {
            badDomainWarning = QLatin1String(" ") +  QObject::tr( "(\"%1\" domain not known)" ).arg( domList[i] );
            continue;
        }
        html += conjunction + res;
        conjunction = QLatin1String(" ") + QObject::tr("and") + QLatin1String(" ");
    }
    return html + badDomainWarning;
}

/*! \internal
  Returns a warning that specifies what an application with the \a domain
  is capable of.
*/
QString DomainInfo::getWarningResource( const QString &domain )
{
    const char **key = domainStrings;
    for ( ; *key; key++, key++ )
        if ( domain == *key )
            return QCoreApplication::translate( "PackageView", *++key );
    return QString();
}
