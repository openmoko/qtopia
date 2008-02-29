/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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



#ifndef SmsClient_H
#define SmsClient_H
#ifndef QTOPIA_NO_SMS

#include <qlist.h>
#include <qobject.h>
#include <qstring.h>

#include "client.h"

#include <quuid.h>
#include <qsiminfo.h>
#include <qtelephonynamespace.h>
#include <qstringlist.h>
#include <qdatetime.h>

struct RawSms
{
    QString number;
    QString body;
};


class QSMSReader;
class QSMSSender;
class QSMSMessage;
class QRegExp;
class Email;

class SmsClient: public Client
{
        Q_OBJECT

public:
        static QString vCardPrefix();
public:
        SmsClient();
        ~SmsClient();
        void setAccount(MailAccount *_account);
        void newConnection();
//      void addMail(const QString& number, const QString& body);
        int addMail(Email* mail);
        void clearList();
        // Determines if a string is in the form *
        bool smsAddress(const QString &);
        // Determines if a string is in the form "^\\+?\\d[\\d-]*$"
        bool validSmsAddress(const QString &);
        // Separates the sms (phone numbers) addresses from the passed address list
        // Returns the sms addressses and modifies the passed list
        QStringList separateSmsAddresses(QStringList &);
        // Format an outgoing message
        QString formatOutgoing( const QString& subject, const QString &body );
        bool hasDeleteImmediately() const;
        void deleteImmediately(const QString& serverUid);
        void resetNewMailCount();
        int unreceivedSmsCount();
        bool readyToDelete();

signals:
        void errorOccurred(int, QString &);
        void updateStatus(const QString &);
        void transferredSize(int);
        void mailSent(int);
        void newMessage(const Email&);

public slots:
        void errorHandling(int, QString msg);
        void mailRead(Email *mail);

private slots:
        void finished(const QString &, QTelephony::Result);
        void messageCount( int );
        void fetched( const QString&, const QSMSMessage& );
        void simIdentityChanged();
        void smsReadyChanged();
private:
        void createMail(Email& mail, QString& message, QString& id, QUuid& uuid, uint size);
private:
        QList<RawSms> smsList;
        QSMSReader *req;
        QSMSSender *sender;
        bool smsFetching, smsSending;
        int total;
        int count;
        bool success;
        QString simIdentity;
        bool haveSimIdentity;
        bool sawNewMessage;
        QStringList activeIds;
        QList<QDateTime> timeStamps;
        MailAccount *account;
        QSimInfo *simInfo;
        static QRegExp *sSmsAddress, *sValidSmsAddress;
};

#endif // QTOPIA_NO_SMS
#endif
