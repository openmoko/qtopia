/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef SMSSTATUSDBUSEXPORTER_H
#define SMSSTATUSDBUSEXPORTER_H

#ifdef QT_ILLUME_LAUNCHER

#include <QDBusConnection>
#include <QSMSSender>

#include <qmailmessage.h>

class SMSStatusDBusExporter : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openmoko.qtopia.Sms")

public:
    SMSStatusDBusExporter(QObject* parent);

Q_SIGNALS:
    Q_SCRIPTABLE void smsSent(const QString&, int result);
    Q_SCRIPTABLE void newSms(const QList<QVariant>& ids);

public Q_SLOTS:
    Q_SCRIPTABLE QString send(const QString& number, const QString& message, bool);
    Q_SCRIPTABLE void open();
    Q_SCRIPTABLE void close();
    Q_SCRIPTABLE QList<QVariant> listMessages() const;
    Q_SCRIPTABLE QMap<QString,QVariant> message(const QString&);

private Q_SLOTS:
    void _q_sent(const QString &id, QTelephony::Result result);
    void _q_messagesAdded(const QMailIdList&);

private:
    QSMSSender *m_sender;
};

#endif

#endif
