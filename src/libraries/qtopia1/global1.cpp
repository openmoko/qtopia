/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/private/contact.h>

#include <qdir.h>
#include <qfile.h>

#if defined(Q_WS_WIN32)
#include <objbase.h>
#else
extern "C" {
#include <uuid/uuid.h>
}
#endif

QString Global::homeDirPath()
{
    QString r = QDir::homeDirPath(); 
#ifdef QTOPIA_DESKTOP
    r += "/.palmtopcenter/";
#endif
    return r;
}

QString Global::journalFileName(const QString &filename)
{
    QDir d;
    QString r = QDir::homeDirPath();
#ifndef Q_WS_QWS
    r += "/.palmtopcenter/temp";
#endif
    if ( !QFile::exists( r ) )
	if ( d.mkdir(r) == false )
	    return QString::null;
    r += "/"; r += filename;
    //qDebug("Global::journalFileName = %s", r.latin1() );
    return r;
}

QUuid Global::generateUuid()
{
    uuid_t uuid;
#ifndef Q_OS_WIN32
    ::uuid_generate( uuid );

    QUuid id;
    memcpy(&id, &uuid, sizeof(QUuid) );
    return id;
#else
    if (UuidCreate(&uuid) == RPC_S_OK)
	return QUuid(uuid);
    else
	return QUuid();
#endif
}

#ifdef Q_WS_QWS

/*!
  Returns a unique ID for this device. The value can change, if
  for example, the device is reset.
*/
QString Global::deviceId()
{
    Config cfg("Security");
    cfg.setGroup("Sync");
    QString r=cfg.readEntry("serverid");
    if ( r.isEmpty() ) {
	QUuid uuid = generateUuid();
	cfg.writeEntry("serverid",(r = uuid.toString()));
    }
    return r;
}

QString Global::ownerName()
{
    Config cfg("Security");
    cfg.setGroup("Sync");
    QString r=cfg.readEntry("ownername");
    return r;
}

#endif
