/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef SERVICES_H
#define SERVICES_H

#include <qtopia/applnk.h>
#include <qdatastream.h>
#include <qbuffer.h>

class QTOPIA_EXPORT Service {
public:
    static QStringList list();
    static QString binding(const QString& service);
    static QStringList apps(const QString& service, bool automatics=TRUE);
    static QString app(const QString& service, const QString& appname=QString::null,
		bool automatics=TRUE);
    static QString appConfig(const QString& service, const QString& appname=QString::null,
		bool automatics=TRUE);
    static QString config(const QString& service);
    static QStrList channels(const QString& service, bool automatics=TRUE);
    static QCString channel(const QString& service, const QString& appname=QString::null,
		bool automatics=TRUE);
    static AppLnk appLnk( const QString& service, const QString& appname=QString::null,
		bool automatics=TRUE);
};

class QTOPIA_EXPORT ServiceRequest : public QDataStream {
public:
    ServiceRequest();
    ServiceRequest(const QString& service, const QCString& message);
    ServiceRequest(const ServiceRequest& orig);
    ~ServiceRequest();

    bool send() const;
    bool isNull() const;

    void setService(const QString& service);
    QString service() const { return m_Service; }
    QStrList channels(bool automatics=TRUE) const;
    QCString channel(const QString& appname=QString::null,
                bool automatics=TRUE) const;
    QStringList apps(bool automatics=TRUE) const;
    QString app(const QString& appname=QString::null,
                bool automatics=TRUE) const;
    void setMessage(const QCString& message);
    QCString message() const { return m_Message; }
    const QByteArray data() const;

    ServiceRequest& operator=(const ServiceRequest& orig);

private:
    QString m_Service;
    QCString m_Message;
};

inline bool operator==( const ServiceRequest &m1, const ServiceRequest &m2 )
{
    return m1.service() == m2.service() && m1.message() == m2.message()
	    && m1.data() == m2.data();
}

#endif
