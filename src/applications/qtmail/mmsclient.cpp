/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "mmsclient.h"
#include "mmscomms.h"
#ifdef MMSCOMMS_HTTP
# include "mmscomms_http.h"
#endif
#include "maillist.h"
#include "mmsmessage.h"
#include <QHttp>
#include <QUrl>
#include <QDebug>
#include <QDSAction>
#include <QDSData>
#include <QDSServices>
#include <QDSServiceInfo>

#ifdef QTOPIA_DESKTOP
#include <qcopenvelope_qd.h>
#else
#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#endif
#include <qtopialog.h>

#include <QtopiaAbstractService>

QString getPath(const QString& fn, bool isdir=false);


int MmsClient::txnId = 1;

MmsClient::MmsClient()
    : account(0), comms(0), mailList(0), quitRecv(false)
{
}

MmsClient::~MmsClient()
{
}

void MmsClient::setAccount(MailAccount *_account)
{
    if (account == _account)
        return;
    if (comms && comms->isActive()) {
        qWarning("MMS connection is already use.");
        return;
    }

    account = _account;

    delete comms;

#ifdef MMSCOMMS_HTTP
    qLog(Messaging) << "Using MMSCommsHttp reference implementation";
    comms = new MmsCommsHttp(account, this);
#else
# error "No MMS comms implementation supplied"
#endif
    connect(comms, SIGNAL(notificationInd(const MMSMessage&)),
            this, SLOT(notificationInd(const MMSMessage&)));
    connect(comms, SIGNAL(deliveryInd(const MMSMessage&)),
            this, SLOT(deliveryInd(const MMSMessage&)));
    connect(comms, SIGNAL(sendConf(const MMSMessage&)),
            this, SLOT(sendConf(const MMSMessage&)));
    connect(comms, SIGNAL(retrieveConf(const MMSMessage&,int)),
            this, SLOT(retrieveConf(const MMSMessage&,int)));
    connect(comms, SIGNAL(statusChange(const QString&)),
            this, SLOT(statusChange(const QString&)));
    connect(comms, SIGNAL(error(int,const QString&)),
            this, SLOT(commsError(int,const QString&)));
    connect(comms, SIGNAL(transferSize(int)),
            this, SLOT(transferSize(int)));
    connect(comms, SIGNAL(transfersComplete()),
            this, SLOT(transfersComplete()));
}

void MmsClient::newConnection()
{
    messagesSent = 0;
    messagesRecv = 0;
    sendNextMessage();
    quitRecv = false;
}

void MmsClient::quit()
{
    if (comms && comms->isActive()) {
        quitRecv = true;
    } else {
        emit mailTransferred(0);
    }
}

void MmsClient::addMail(const MailMessage &mail)
{
    MMSMessage mms = convertToMms(mail);
    QSettings conf(networkConfig(), QSettings::IniFormat);
    conf.beginGroup("MMS");
    QString visibility = conf.value("Visibility").toString();
    if (visibility == "show")
        mms.addField("X-Mms-Sender-Visibility", "Show");
    else if (visibility == "hidden")
        mms.addField("X-Mms-Sender-Visibility", "Hide");
    int exp = conf.value("Expiry", 0).toInt();
    if (exp)
        mms.addField("X-Mms-Expiry", exp*3600);

    outgoing.append(mms);
}

void MmsClient::sendNextMessage()
{
    if (!outgoing.count())
        return;
    qLog(Messaging) << "Sending MMS message";
    MMSMessage &msg = outgoing.first();
    comms->sendMessage(msg);
}

void MmsClient::sendConf(const MMSMessage &msg)
{
    const QWspField *field = msg.field("X-Mms-Transaction-Id");
    qLog(Messaging) << "Recieved send conf, txn id" << (field ? field->value : QLatin1String("unknown"));
    field = msg.field("X-Mms-Response-Status");
    if (field && field->value != "Ok") {
        // ### handle send error
        errorHandling(0, field->value);
    } else {
        messagesSent++;
        outgoing.removeFirst();
        if (outgoing.count()) {
            sendNextMessage();
        } else {
            emit mailSent(messagesSent);
            emit mailSent(-1);
        }
    }
}

void MmsClient::notificationInd(const MMSMessage &mmsMsg)
{
    QString details;
    const QWspField *field = mmsMsg.field("X-Mms-Content-Location");
    if (!field) {
        qWarning("Invalid m-notification-ind");
        return;
    }
    QString location = field->value;
    int size = 0;
    field = mmsMsg.field("X-Mms-Message-Size");
    if (field)
        size = field->value.toInt();
    const QList<QWspField> &headers = mmsMsg.headers();
    QList<QWspField>::ConstIterator it;
    for (it = headers.begin(); it != headers.end(); ++it) {
        details += (*it).name + ": " + (*it).value + "\n";
    }
    MailMessage mail;
    mail.fromRFC822(details);
    mail.setFrom(decodeRecipient(mail.from()));        // fix phone numbers
    mail.setType(MailMessage::MMS);
    mail.setSize(size);
    mail.setDateTime(QDateTime::currentDateTime());
    mail.setServerUid(location);
    internalId = QUuid();
    mail.setUuid(internalId);
    mail.setStatus(EFlag_Downloaded, false);
    mail.setStatus(EFlag_Incoming, true);
    mail.setFromAccount(account->id());
    mail.setFromMailbox("");
    Email newMail(mail);
    emit newMessage(newMail);
}

void MmsClient::sendNotifyResp(const MailMessage &notification, const QString &status)
{
    MMSMessage resp;
    resp.setType(MMSMessage::MNotifyResp);
    resp.addField("X-Mms-MMS-Version", "1.0");
    QString field = notification.header("X-Mms-Transaction-Id");
    resp.addField("X-Mms-Transaction-Id", field.mid(field.indexOf(':')+1));
    resp.addField("X-Mms-Status", status);
    QSettings conf(networkConfig(), QSettings::IniFormat);
    conf.beginGroup("MMS");
    QString delRpt = conf.value("AllowDeliveryReport", "n").toString();
    if (delRpt != QLatin1String("y"))
        resp.addField("X-Mms-Report-Allowed", "No");

    comms->sendMessage(resp);
}

void MmsClient::resetNewMailCount()
{
    QSettings mailconf("Trolltech", "qtmail");
    mailconf.beginGroup("MMS");
    int count = mailconf.value("newMmsCount").toInt();
    if (count) {
        count = 0;
        mailconf.setValue("newMmsCount", count);
        QtopiaIpcEnvelope e("QPE/System", "newMmsCount(int)");
        e << count;
    }
}

void MmsClient::setSelectedMails(MailList *list, bool connected)
{
    Q_UNUSED(connected);
    mailList = list;
    messagesRecv = 0;
    getNextMessage();
}

void MmsClient::getNextMessage()
{
    if (!mailList)
        return;
    QString *msg;

    //messagesRecv is reset when doing a read-reply
    //so just get next message
/*     if (!messagesRecv)
        msg = mailList->first();
    else */
        msg = mailList->next();
    if (msg) {
        internalId = mailList->currentId();
        QUrl url(*msg);
        comms->retrieveMessage(url);
    } else {
        emit mailTransferred(messagesRecv);
        mailList = 0;
    }
}

void MmsClient::retrieveConf(const MMSMessage &msg, int size)
{
    messagesRecv++;
    sendAcknowledge(msg);
    Email mail = convertToEmail(msg, size);
    emit newMessage(mail);
    getNextMessage();
}

void MmsClient::sendAcknowledge(const MMSMessage &retr)
{
    MMSMessage resp;
    resp.setType(MMSMessage::MAckowledgeInd);
    resp.addField("X-Mms-MMS-Version", "1.0");
    const QWspField *field = retr.field("X-Mms-Transaction-Id");
    if (field)
        resp.addField("X-Mms-Transaction-Id", field->value);
    QSettings conf(networkConfig(), QSettings::IniFormat);
    conf.beginGroup("MMS");
    QString delRpt = conf.value("AllowDeliveryReport", "n").toString();
    if (delRpt != QLatin1String("y"))
        resp.addField("X-Mms-Report-Allowed", "No");

    comms->sendMessage(resp);
}

void MmsClient::deliveryInd(const MMSMessage &mmsMsg)
{
    QString details;
    const QList<QWspField> &headers = mmsMsg.headers();
    QList<QWspField>::ConstIterator it;
    for (it = headers.begin(); it != headers.end(); ++it) {
        details += (*it).name + ": " + (*it).value + "\n";
    }
    MailMessage mail;
    mail.fromRFC822(details);
    internalId = QUuid();
    mail.setUuid(internalId);
    mail.setStatus(EFlag_Downloaded, true);
    mail.setStatus(EFlag_Incoming, true);
    mail.setFromAccount(account->id());
    mail.setFromMailbox("");
    Email newMail(mail);
    emit newMessage(newMail);
}

void MmsClient::transferSize(int done)
{
    emit transferredSize(done);
}

void MmsClient::statusChange(const QString &status)
{
    emit updateStatus(status);
}

void MmsClient::commsError(int code, const QString &str)
{
    QString tmp(str);
    emit updateStatus(tr("Error occurred"));
    emit errorOccurred(code, tmp);
}

void MmsClient::transfersComplete()
{
    if (quitRecv) {
        emit mailTransferred(0);
        quitRecv = false;
    }
}

void MmsClient::errorHandling(int code, QString msg)
{
    if (!comms || !comms->isActive())
        return;
    emit updateStatus(tr("Error occurred"));
    emit errorOccurred(code, msg);
    if (comms)
        comms->clearRequests(); //clear after error emit
}

Email MmsClient::convertToEmail(const MMSMessage &mms, int size)
{
    Email mail;

    mail.setType(MailMessage::MMS);
    mail.setStatus(EFlag_Incoming, true );
    mail.setUuid( internalId );

    QString message;

    QString to, cc;

    // first handle the headers.  The headers that we get out of the
    // mms decoder are still encoded, so use fromRFC822 to deal with
    // the decoding.
    QList<QWspField>::ConstIterator it;
    for (it = mms.headers().begin(); it != mms.headers().end(); ++it) {
        const QWspField &field = *it;
        if (field.name == "To") {
            if (!to.isEmpty())
                to += ", ";
            to += decodeRecipient(field.value);
        } else if (field.name == "Cc") {
            if (!cc.isEmpty())
                cc += ", ";
            cc += decodeRecipient(field.value);
        } else if (field.name == "From") {
            message += "From: " + decodeRecipient(field.value) + "\n";
        } else if (field.name != "Content-Type") {
            message += field.name + ": " + field.value + "\n";
        }
    }

    if (!to.isEmpty())
        message += "To: " + to + "\n";
    if (!cc.isEmpty())
        message += "Cc: " + cc + "\n";

    QString boundary = "-----4345=next_bound=0495----";

    message += "Mime-Version: 1.0\n";
    const QWspField *f = mms.field("Content-Type");
    if (f) {
        if (f->value.contains("application/vnd.wap.multipart.related"))
            message += f->name + ": " + "multipart/related";
        else
            message += f->name + ": " + "multipart/mixed";
        QString value = f->value;
        int semi = value.indexOf(';');
        if (semi > 0)
            message += value.mid(semi);
        message += "; boundary=\"" + boundary + "\"\n\n";
    }

    message += "This is a multipart message in Mime 1.0 format\n\n";

    QStringList serviceAttributes = QStringList() << QLatin1String( "drm" ) << QLatin1String( "handle" );

    // Now add the parts
    for (int i = 0; i < mms.messagePartCount(); i++) {
        message += "--" + boundary + "\n";
        bool isText = false;
        const QWspPart &mmsPart = mms.messagePart(i);
        const QWspField *f = mmsPart.header("Content-Type");

        QByteArray data;

        if (f) {
            QString type = f->value.trimmed();

            int semicolon = type.indexOf( ';' );

            if( semicolon != -1 )
                type.truncate( semicolon );

            QDSServices services( type, QLatin1String( "*" ), serviceAttributes );

            if( !services.isEmpty() )
            {
                QDSServiceInfo service = services.first();

                QDSAction action( service );

                if( action.exec( QDSData( mmsPart.data(), QMimeType(type) ) ) == QDSAction::CompleteData )
                {
                    QDSData responseData = action.responseData();

                    data = responseData.data();

                    size += data.size() - mmsPart.data().size();

                    message += "Content-Type: " + responseData.type().id() + "\n";
                }
                else
                {
                    message += "Content-Type: text/plain\n";
                    message += "Content-Transfer-Encoding: 8bit\n\n";
                    message += "\n\n";

                    continue;
                }
            }
            else
            {
                data = mmsPart.data();
                message += "Content-Type: " + f->value + "\n";

               if (f->value.indexOf("text/") == 0)
                   isText = true;
            }
        }
        f = mmsPart.header("Content-ID");
        if (f)
            message += "Content-ID: " + f->value + "\n";
        f = mmsPart.header("Content-Location");
        if (f)
            message += "Content-Location: " + f->value + "\n";
        if (isText) {
            message += "Content-Transfer-Encoding: 8bit\n\n";
            message += QString(data) + "\n";
        } else {
            message += "Content-Transfer-Encoding: Base64\n\n";
            message += MailMessage::encodeBase64(data) + "\n";
        }
        message += "\n";
    }

    message += "--" + boundary + "--";

    message += "\n.\n";

    mail.fromRFC822( message );

    mail.setMultipartRelated(true);
    mail.setSize( size );
    mail.setStatus(EFlag_Downloaded, true);
    f = mms.field("Message-ID");
    if (f)
        mail.setServerUid(f->value);

    mail.setFromAccount( account->id() );
    mail.setFromMailbox("");

    return mail;
}

MMSMessage MmsClient::convertToMms(const MailMessage &mail)
{
    MMSMessage mms;
    mms.setType(MMSMessage::MSendReq);
    QString id = "00000" + QString::number(txnId);
    id = id.right(5);
    mms.addField("X-Mms-Transaction-Id", id);
    mms.addField("X-Mms-MMS-Version", "1.0");
    txnId++;

    if (mail.dateString().isNull()) {
        mms.addField("Date", QDateTime::currentDateTime().toString());
    } else {
        mms.addField("Date", mail.dateString());
    }

    mms.addField("From", "");   // i.e. insert-an-address token
    if (!mail.subject().isEmpty())
        mms.addField("Subject", mail.subject());

    QStringList list = mail.to();
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        mms.addField("To", encodeRecipient(*it));
    list = mail.cc();
    for (it = list.begin(); it != list.end(); ++it)
        mms.addField("Cc", encodeRecipient(*it));
    list = mail.bcc();
    for (it = list.begin(); it != list.end(); ++it)
        mms.addField("Bcc", encodeRecipient(*it));

    addField(mms, mail, "X-Mms-Delivery-Report");
    addField(mms, mail, "X-Mms-Read-Reply");

    if (mail.multipartRelated())
        mms.addField("Content-Type", "application/vnd.wap.multipart.related; type=application/smil; start=<presentation-part>");
    else
        mms.addField("Content-Type", "application/vnd.wap.multipart.mixed");

    for (uint i = 0; i < mail.messagePartCount(); i++) {
        const MailMessagePart &mailPart = mail.messagePartAt(i);
        QWspPart mmsPart;
        mmsPart.addHeader("Content-Type", mailPart.contentType());
        if (mailPart.contentType() == "application/smil") {
            mmsPart.addHeader("Content-ID", "<presentation-part>");
        } else if (!mailPart.contentID().isEmpty()) {
            mmsPart.addHeader("Content-ID", mailPart.contentID());
        } else {
            mmsPart.addHeader("Content-ID", QString("CID-%1").arg(i));
        }
        mmsPart.addHeader("Date", QDateTime::currentDateTime().toString());
        if (!mailPart.contentLocation().isEmpty()) {
            mmsPart.addHeader("Content-Location", mailPart.contentLocation());
            mmsPart.addHeader("X-Wap-Content-URI", "http://orig.host/" + mailPart.contentLocation());
        }
        QByteArray decoded = mailPart.decodedBody().toLatin1();
        mmsPart.setData(decoded.data(), decoded.length());
        mms.addMessagePart(mmsPart);
    }

    if (mail.messagePartCount() == 0 && !mail.plainTextBody().isEmpty()) {
        // Work-around for MailMessage converting a text/plain attachment
        // to a body.
        QWspPart mmsPart;
        mmsPart.addHeader("Content-Type", "text/plain");
        mmsPart.addHeader("Content-ID", QString("CID-0"));
        mmsPart.addHeader("Date", mail.dateString());
        QByteArray body = mail.plainTextBody().toLocal8Bit();
        mmsPart.setData(body.data(), body.length());
        mms.addMessagePart(mmsPart);
    }

    return mms;
}

void MmsClient::addField(MMSMessage &mms, const Email &mail, const QString &field)
{
    QString f = mail.header(field);
    if (!f.isNull()) {
        int colon = f.indexOf(':');
        if (colon) {
            f = f.mid(colon+1);
            if (f.length() && f[0].isSpace())
                f = f.mid(1);
            if (!f.isEmpty())
                mms.addField(field, f);
        }
    }
}

QString MmsClient::encodeRecipient(const QString &r)
{
    QString digits("0123456789+!@#*,pwxabcd");
    bool isNumber = true;
    for (int i = 0; i < r.length(); i++) {
        if (!digits.contains(r[i])) {
            isNumber = false;
            break;
        }
    }

    if (isNumber)
        return r + "/TYPE=PLMN";
    else
        return r;
}

QString MmsClient::decodeRecipient(const QString &r)
{
    int pos = r.indexOf("/TYPE=");
    if (pos > 0)
        return r.left(pos);

    return r;
}

QString MmsClient::networkConfig() const
{
//     QSettings defltConf("Trolltech", "Network");
//     defltConf.beginGroup("WAP");
    Q_ASSERT(account);
    QString accountstr = account->networkConfig();
    qLog(Messaging) << "MmsClient: Using network config:" << accountstr;
    return accountstr;
}
