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

#include "smsstatusdbusexporter.h"

SMSStatusDBusExporter::SMSStatusDBusExporter(QObject* parent)
    : QObject(parent)
    , m_sender(0)
{}

// the bool is ignored, here for phonekit compat
QString SMSStatusDBusExporter::send(const QString& number, const QString& message, bool)
{
    if (!m_sender) {
        m_sender = new QSMSSender(QString(), this);
        connect(m_sender, SIGNAL(finished(const QString&,QTelephony::Result)),
                SLOT(_q_sent(const QString&,QTelephony::Result)));
    }

    QSMSMessage shortMessage;
    shortMessage.setText(message);
    shortMessage.setRecipient(number); 

    return m_sender->send(shortMessage);
}

void SMSStatusDBusExporter::_q_sent(const QString& id, QTelephony::Result result)
{
    emit smsSent(id, result);
}
