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

QTOPIA_EXPORT QDataStream& operator<<( QDataStream&, const ServiceRequest & );
QTOPIA_EXPORT QDataStream& operator>>( QDataStream&, ServiceRequest & );

#endif
