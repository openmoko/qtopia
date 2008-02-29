/****************************************************************************
**
** Copyright (C) 2000-2006 Trolltech AS. All rights reserved.
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
#ifndef __DBUSAPPLICATIONCHANNEL_H__
#define __DBUSAPPLICATIONCHANNEL_H__

#if defined(QTOPIA_DBUS_IPC)

#include <QString>
#include <qtopiaglobal.h>

class QDBusMessage;

class QTOPIABASE_EXPORT DBUSQtopiaApplicationChannel : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.qtopia")

public:
    DBUSQtopiaApplicationChannel(const QString &appName, QObject *parent = 0);
    ~DBUSQtopiaApplicationChannel();

    const QString &appName() const { return m_appName; }

public slots:
    Q_SCRIPTABLE void appMessage(const QString &msg, const QByteArray &data);
    Q_SCRIPTABLE void foobar();

signals:
    void received(const QString &msg, const QByteArray &data);

private:
    QString m_serviceName;
    QString m_appName;
};

#endif

#endif // __DBUSAPPLICATIONCHANNEL_H__
