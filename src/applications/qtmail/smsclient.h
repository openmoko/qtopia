/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include <qtopiaglobal.h>

#include "qmailid.h"
#include "qmailaddress.h"

struct RawSms
{
    QString number;
    QString body;
    QString mimetype;
    QMailId msgId;
};


class QSMSReader;
class QSMSSender;
class QSMSMessage;
class QRegExp;
class QMailMessage;

class SmsClient: public Client
{
        Q_OBJECT

public:
        SmsClient();
        ~SmsClient();
        void setAccount(QMailAccount *_account);
        void newConnection();
        bool addMail(const QMailMessage& mail);
        void clearList();
        // Determines if a string is in the form *
        bool smsAddress(const QMailAddress &);
        // Determines if a string is in the form "^\\+?\\d[\\d-]*$"
        bool validSmsAddress(const QMailAddress &);
        // Separates the sms (phone numbers) addresses from the passed address list
        // Returns the sms addressses and modifies the passed list
        QList<QMailAddress> separateSmsAddresses(const QList<QMailAddress> &);
        // Format an outgoing message
        QString formatOutgoing( const QString& subject, const QString &body );
        bool hasDeleteImmediately() const;
        void deleteImmediately(const QString& serverUid);
        void resetNewMailCount();
        int unreceivedSmsCount();
        int unreadSmsCount();
        bool readyToDelete();
        void checkForNewMessages();
        int newMessageCount();

signals:
        void errorOccurred(int, QString &);
        void updateStatus(const QString &);
        void mailSent(int);
        void transmissionCompleted();
        void newMessage(const QMailMessage&);
        void allMessagesReceived();
        void sendProgress(const QMailId&, uint);    // Not implemented
        void messageProcessed(const QMailId&);

public slots:
        void errorHandling(int, QString msg);
        void mailRead(const QMailMessage& mail);

private slots:
        void finished(const QString&, QTelephony::Result);
        void messageCount( int );
        void fetched( const QString&, const QSMSMessage& );
        void simIdentityChanged();
        void smsReadyChanged();
private:
        QList<RawSms> smsList;
        QMap<QString, RawSms> sentMessages;
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
        QMailAccount *account;
        QSimInfo *simInfo;
        static QRegExp *sSmsAddress, *sValidSmsAddress;
};

#endif // QTOPIA_NO_SMS
#endif
