/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#include "domaininfo.h"

#include <QCoreApplication>
#include <QStringList>

#ifdef QTOPIA_PHONE
const char *DomainInfo::domainStrings[] = {
    "base",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\"><b>new system for your Qtopia phone</b></font>" ),
    "libs",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\"><b>new components for your Qtopia phone</b></font>" ),
    "phonecomm",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">use <b>phone hardware</b></font>" ),
    "docapi",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">access and modify <b>documents</b></font>" ),
    "pim",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">access or modify <b>personal addresses and info</b></font>" ),
    "msg",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">send <b>sms and email</b></font>" ),
    "net",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">download/send over your <b>phones internet service</b></font>" ),
    "netconfig",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">start, stop and configure <b>network interfaces</b></font>" ),
    "alarm",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">create, delete and check status of <b>alarms</b></font>" ),
    "beaming",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">send and receive files over <b>infrared interfaces</b></font>" ),
    "bluetooth",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">connect, pair, send and receive files over <b>bluetooth interfaces</b></font>" ),
    "cardreader",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">read and write data to/from <b>memory cards</b></font>" ),
    "camera",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">capture images using <b>camera devices</b></font>" ),
    "datetime",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">set <b>date, time and timezone</b></font>" ),
    "drm",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">download, use and check the rights on <b>DRM</b> enabled files</font>" ),
    "handwriting",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">allow <b>handwriting</b> as an input method</font>" ),
    "helix",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">play media files using <b>helix</b></font>" ),
    "language",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">change system <b>language</b></font>" ),
    "lightandpower",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\"><b>power management and lighting settings</b></font>" ),
    "mediarecorder",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\"><b>record sound files</b></font>" ),
    "pictures",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">view and edit <b>image files</b></font>" ),
    "qdl",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\"><b>link to documents/objects</b></font>" ),
    "quicklauncher",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">run applications using <b>quicklauncher</b></font>" ),
    "ringprofiles",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">change <b>Profile, Appearance and Light and Power settings</b></font>" ),
    "screensaver",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">set <b>screensaver</b></font>" ),
    "security",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">change phone and sim <b>security settings</b></font>" ),
    "sxemonitor",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">issue security alerts using <b>sxemonitor</b></font>" ),
    "words",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\"><b>add words to dictionary</b></font>" ),
    "window",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">display <b>text and buttons on screen</b></font>" ),
    "graphics",

    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">use your <b>whole screen</b></font>" ),
    "theme",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">provide a <b>new theme for your phone</b></font>" ),
    "none",
    QT_TRANSLATE_NOOP( "PackageView", "not <font color=\"#66CC00\">access any resources on your phone</font>" ),
    0,
    0
};
#else
const char *DomainInfo::domainStrings[] = {
    "base",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\"><b>new system for your Qtopia device</b></font>" ),
    "libs",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\"><b>new components for your Qtopia device</b></font>" ),
    "phonecomm",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">use <b>device hardware</b></font>" ),
    "docapi",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">access and modify <b>documents</b></font>" ),
    "pim",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">access or modify <b>personal addresses and info</b></font>" ),
    "msg",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">send <b>sms and email</b></font>" ),
    "net",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">download/send over your <b>devices internet service</b></font>" ),
    "window",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">display <b>text and buttons on screen</b></font>" ),
    "graphics",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#FF9900\">use your <b>whole screen</b></font>" ),
    "theme",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">provide a <b>new theme</b> for your device</font>" ),
    "none",
    QT_TRANSLATE_NOOP( "PackageView", "not <font color=\"#66CC00\">access any resources on your device</font>" ),
    0,
    0
};
#endif

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
    QString html = QObject::tr( "%1 can", "[%1 = Package] is able to [for example access documents]" )
        .arg( packageName );
    QString conjunction = " ";
    bool emptyDomTextDisplayed = false;
    QString badDomainWarning;
    for ( int i = 0; i < domList.count(); i++ )
    {
        if ( domList[i].isEmpty() ) continue;
        if ( domList[i] == "none" )
            emptyDomTextDisplayed = true;
        QString res = getWarningResource( domList[i] );
        if ( res.isEmpty() )
        {
            badDomainWarning = QObject::tr( " (\"%1\" domain not known)" ).arg( domList[i] );
            continue;
        }
        html += conjunction + res;
        conjunction = " and ";
    }
    return html + badDomainWarning;
}

QString DomainInfo::getWarningResource( const QString &url )
{
    const char **key = domainStrings;
    for ( ; *key; key++, key++ )
        if ( url == *key )
            return QCoreApplication::translate( "PackageView", *++key );
    return QString();
}
