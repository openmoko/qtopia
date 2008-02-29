/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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
#include <QStringList>

const char *DomainInfo::domainStrings[] = {
    "accessories",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">use <b>accessories</b> such as keypad lights</font>" ),
    "alarm",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">create, delete and check status of <b>alarms</b></font>" ),
    "audio-policy",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">route <b>audio</b> between audio devices</font>" ),
    "base",
    QT_TRANSLATE_NOOP( "PackageView", "is a <font color=\"#CC0000\"><b>new system for your Qtopia device</b></font>" ),
    "beaming",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">send and receive files over <b>infrared interfaces</b></font>" ),
    "bluetooth",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">connect, pair, send and receive files over <b>bluetooth interfaces</b></font>" ),
    "callhistory",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">display <b>call history</b></font>" ),
    "camera",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">capture images via the <b>camera application</b></font>" ),
    "cameraservice",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">directly access the <b>camera device</b></font>" ),
    "categories",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF99C00\">edit system and user<b>categories</b></font>" ),
    "datetime",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">set <b>date, time and timezone</b></font>" ),
    "directaudio",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">directly access <b>audio hardware</b></font>" ),
    "directvideo",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">directly access <b>video hardware</b></font>" ),
    "doc_open",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">read <b>user documents</b> that you specifically choose</font>" ),
    "doc_read",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">read <b>any user document</b></font>" ),
    "doc_save",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>write</b> to <b>user documents</b> that you specifically choose</font>" ),
    "doc_server",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">search <b>user documents</b></font>" ),
    "doc_write",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\"><b>write</b> to <b>any user document</b></font>" ),
    "graphics",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">use your <b>whole screen</b></font>" ),
    "handwriting",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">allow <b>handwriting</b> as an input method</font>" ),
    "language",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change system <b>language</b></font>" ),
    "lightandpower",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>power management and lighting settings</b></font>" ),
    "mediarecorder",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>record sound files</b></font>" ),
    "mediasession",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>play media files</b></font>" ),
    "msg",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">start the mail application</font>" ),
    "net",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">download/send over your <b>internet service</b></font>" ),
    "netconfig",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">start, stop and configure <b>network interfaces</b></font>" ),
    "pictures",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">view and edit <b>image files</b></font>" ),
    "pim",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">access or modify <b>personal addresses and info</b></font>" ),
    "print",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>print</b> files</font>" ),
    "profiles",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change <b>Profile,Appearance and Light and Power settings</b></font>" ),
    "qdl",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>link to documents/objects</b></font>" ),
    "qds",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">share data with other applications</font>" ),
    "security",
#ifdef QTOPIA_CELL
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change phone and sim <b>security settings</b></font>" ),
#else
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change device <b>security settings</b></font>" ),
#endif
    "voip",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">display <b>voip configuration</b> settings</font>" ),
    "volumemanager",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change <b>volume settings</b></font>" ),
    "window",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">display <b>text and buttons on screen</b></font>" ),
    "words",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\"><b>add words to dictionary</b></font>" ),

//This set of domains are not available for third party apps
//but are retained for when they may be used when dowloading signed/trusted applications
    "phonecomm",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#CC0000\">use <b>device hardware</b></font>" ),
    "docapi",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#FF9900\">access and modify <b>documents</b></font>" ),
    "drm",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">download, use and check the rights on <b>DRM</b> enabled files</font>" ),
    "helix",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">play media files using <b>helix</b></font>" ),
    "quicklauncher",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">run applications using <b>quicklauncher</b></font>" ),
    "ringprofiles",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">change <b>Profile, Appearance and Light and Power settings</b></font>" ),
    "screensaver",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">set <b>screensaver</b></font>" ),
    "sxemonitor",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">issue security alerts using <b>sxemonitor</b></font>" ),
    "theme",
    QT_TRANSLATE_NOOP( "PackageView", "can <font color=\"#66CC00\">provide a <b>new theme for your device</b></font>" ),
    "none",
    QT_TRANSLATE_NOOP( "PackageView", "can not <font color=\"#66CC00\">access any resources on your device</font>" ),
    "trusted",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#CC0000\">requests <b>unrestricted access</b> on your device</font>" ),
    "untrusted",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"#66CC00\">requests <b>minimal access priveleges</b> on your device</font>" ),
    0,
    0
};

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

QString DomainInfo::getWarningResource( const QString &url )
{
    const char **key = domainStrings;
    for ( ; *key; key++, key++ )
        if ( url == *key )
            return QCoreApplication::translate( "PackageView", *++key );
    return QString();
}
