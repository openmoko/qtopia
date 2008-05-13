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

#include "qmailstore.h"

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

void SMSStatusDBusExporter::open()
{
    connect(QMailStore::instance(), SIGNAL(messagesAdded(const QMailIdList&)),
            SLOT(_q_messagesAdded(const QMailIdList&)));
}

void SMSStatusDBusExporter::close()
{
    QMailStore::instance()->disconnect(this);
}

QList<QVariant> SMSStatusDBusExporter::listMessages() const
{
    QList<QVariant> result;

    /*
     * Create a QMailMessageKey that matches any folder called inbox_ident
     */ 
    QMailFolderKey inboxKey(QMailFolderKey::Name, "inbox_ident");
    QMailMessageKey folderKey;
    foreach(QMailId folderId, QMailStore::instance()->queryFolders(inboxKey))
        folderKey |= QMailMessageKey(QMailMessageKey::ParentFolderId, folderId);
    

    /* We want messages that are SMS and within a inbox */
    QMailMessageKey key = QMailMessageKey(QMailMessageKey::Type, QMailMessage::Sms);
    key &= folderKey;

    foreach(QMailId id, QMailStore::instance()->queryMessages(key))
        result << QString::number(id.toULongLong());

    return result;
}

QMap<QString, QVariant> SMSStatusDBusExporter::message(const QString& id)
{
    QMailMessage message = QMailStore::instance()->message(QMailId(id.toULongLong()));

    QMap<QString, QVariant> messageResult;
    messageResult[QLatin1String("uid")] = message.id().toULongLong();
    messageResult[QLatin1String("from")] = message.from().toString();
    messageResult[QLatin1String("subject")] = message.subject();
    messageResult[QLatin1String("content")] = message.body().data();

    return messageResult;
}

void SMSStatusDBusExporter::_q_messagesAdded(const QMailIdList& list)
{
    QList<QVariant> newIds;

    foreach(QMailId id, list) {
        QMailMessage header = QMailStore::instance()->messageHeader(id);
        QMailFolder folder = QMailStore::instance()->folder(header.parentFolderId());
        if (header.messageType() == QMailMessage::Sms && folder.isRoot() && folder.name() == QLatin1String("inbox_ident"))
            newIds << QString::number(id.toULongLong());
    }

    if (newIds.isEmpty())
        return;

    emit newSms(newIds);
}
