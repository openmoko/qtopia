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


#ifndef QTOPIA_NO_SMS
#include "smsclient.h"
#include "emailhandler.h"
#include "smsdecoder.h"
#include "email.h"
#include "common.h"

#include <qtopia/pim/qphonenumber.h>
#include <qsmsreader.h>
#include <qsmsmessage.h>
#include <qsmssender.h>
#include <qtopia/mail/mailmessage.h>
#include <qregexp.h>

QRegExp* SmsClient::sSmsAddress = 0;
QRegExp* SmsClient::sValidSmsAddress = 0;

QString SmsClient::vCardPrefix()
{
    static QString prefix("//qtmail-smsvcard "); //no tr
    return prefix;
}

SmsClient::SmsClient()
{
    req = new QSMSReader( QString(), this );
    smsFetching = false;
    smsSending = false;
    req->check();
    total = 0;
    count = 0;
    success = false;
    haveSimIdentity = false;
    sawNewMessage = false;
    account = 0;
    if (!sSmsAddress)
      sSmsAddress = new QRegExp( "^.*$");
    if (!sValidSmsAddress)
      sValidSmsAddress = new QRegExp( "^[\\+\\*#\\d\\-\\(\\)\\s]*$");

    sender = new QSMSSender( QString(), this );
    connect( sender, SIGNAL(finished(const QString&,QTelephony::Result)),
             this, SLOT(finished(const QString&,QTelephony::Result)) );

    simInfo = new QSimInfo( QString(), this );

    connect( req, SIGNAL(messageCount( int )),
             this, SLOT(messageCount( int )) );
    connect( req, SIGNAL(fetched( const QString&, const QSMSMessage& )),
             this, SLOT(fetched( const QString&, const QSMSMessage& )) );
}

SmsClient::~SmsClient()
{
}

void SmsClient::setAccount(MailAccount *_account)
{
    account = _account;
}

void SmsClient::newConnection()
{
    // XXX need better way to handle this.
    QSettings c("PhoneProfile"); // no tr
    c.beginGroup("Profiles"); // no tr
    bool planeMode = c.value("PlaneMode", false).toBool();
    smsSending = true;
    if (planeMode) {
        errorHandling(0, tr("Messages cannot be sent in Airplane Mode."));
        return;
    }

    QList<RawSms>::iterator rawMsg;
    for ( rawMsg = smsList.begin(); rawMsg != smsList.end(); rawMsg++) {
        QSMSMessage msg;

    //check for vcard over sms
    QString prelude = rawMsg->body.left(vCardPrefix().length());
    if(prelude == vCardPrefix())
    {
        QString vcardData = rawMsg->body.right(rawMsg->body.length()-prelude.length());
        //restore CR's stripped by composer
        vcardData.replace("\n","\r\n");
        msg.setApplicationData(vcardData.toLatin1());
        msg.setDestinationPort(9204);
    }
    else
        msg.setText( rawMsg->body );

        msg.setRecipient( rawMsg->number );
        sender->send( msg );
        ++total;
    }
    success = true;
    smsSending = false;
    smsList.clear();
}

int SmsClient::unreceivedSmsCount()
{
    return req->unreadList().count();
}

bool SmsClient::readyToDelete()
{
    return req->ready();
}

int SmsClient::addMail(Email* mail)
{
    QString from,name,email;
    int msgSize = 0;

    QStringList recipients = mail->to() + mail->cc() + mail->bcc();
    QStringList smsRecipients = separateSmsAddresses(recipients);
    Q_ASSERT(smsRecipients.count() > 0);

    QString smsBody = formatOutgoing(mail->subject(),mail->plainTextBody());

    for(QStringList::Iterator it = smsRecipients.begin();
        it != smsRecipients.end(); it++)
    {
        if(smsAddress(*it))
        {
            if(!validSmsAddress(*it))
            {
                QString temp = "<qt>" + tr("Invalid sms recipient specified for\n "
                   "mail with subject:\n%1\n"
                   "NO mail has been sent.")
                .arg( mail->subject() ) + "</qt>";

                emit errorOccurred(0,temp);
                return -1;
            }
            {
                //address is valid, queue for sending
                // Extract the phone number from the e-mail address.
                from = *it;
                MailMessage::parseEmailAddress( from, name, email );
                if ( email.length() > 4 && email.right(4) == "@sms" ) {
                    email = email.left( email.length() - 4 );
                }
                RawSms msg;
                msg.number = QPhoneNumber::resolveLetters( email );
                msg.body = smsBody;
                msgSize += smsBody.length();
                smsList.append( msg );
            }
        }
    }
    return msgSize;
}

void SmsClient::clearList()
{
    smsList.clear();
}

bool SmsClient::smsAddress(const QString& str )
{
    QString from, name, email;
    from = str;
    MailMessage::parseEmailAddress( from, name, email );
    return sSmsAddress->indexIn( email ) != -1;
}

bool SmsClient::validSmsAddress(const QString& str )
{
    QString from, name, email;
    from = str;
    MailMessage::parseEmailAddress( from, name, email );
    return sValidSmsAddress->indexIn( email ) != -1;
}

QStringList SmsClient::separateSmsAddresses( QStringList &addresses )
{
    QStringList validSms;
    QStringList::Iterator it = addresses.begin();
    while( it != addresses.end() )
    {
        if( validSmsAddress( *it ) )
        {
            validSms.append( *it );
            it = addresses.erase( it );
        }
        else
        {
            ++it;
        }
    }
    return validSms;
}

void SmsClient::errorHandling(int id, QString msg)
{
    if (smsSending)
        emit errorOccurred(id, msg);
}

void SmsClient::mailRead(Email *mail)
{
    Q_UNUSED(mail)
    // Tell phone library mail has been read
    // req->markMessageReadMethod( mail->serverUid() );
}

void SmsClient::finished( const QString &, QTelephony::Result result )
{
    if ( result != QTelephony::OK )
        success = false;
    ++count;
    if ( count >= total ) {
        if ( success )
            emit mailSent( count );
        else
            emit mailSent( -1 );
    }
}

void SmsClient::messageCount( int count )
{
    if ( count > 0 && !smsFetching) {
        smsFetching = true;

        // Get the SIM identity, for use in creating unique identifiers.
        // After we get the identity, we will start fetching the messages.
        haveSimIdentity = false;
        simIdentityChanged();

    } else if ( count > 0 && smsFetching ) {

        // A new message arrived in the queue while we were fetching
        // the existing messages.  By setting "sawNewMessage" to true,
        // we can force another check to happen at the end of the fetch.
        sawNewMessage = true;

    }
}

// Extract the subject line and body from an SMS message.
// According to GSM 03.40, subjects can be encoded in one
// of two ways:
//
//          (subject)message
//          ##subject#message
//
// If the message doesn't have one of the formats above,
// then we extract the first few words from the message
// body and use that as the subject.
static void extractSubjectAndBody( const QString& str, QString& subject,
                                   QString& body )
{
    int index;

    // See if we have one of the standard GSM 03.40 forms.
    if ( str.startsWith( "(" ) ) {
        index = str.indexOf(')');
        if ( index != -1 ) {
            subject = str.mid( 1, index - 1 );
            body = str.mid( index + 1 );
            return;
        }
    } else if ( str.startsWith( "##" ) ) {
        index = str.indexOf( '#', 2 );
        if ( index != -1 ) {
            subject = str.mid( 2, index - 2 );
            body = str.mid( index + 1 );
            return;
        }
    }

    // Extract the first few words from the body to use as the subject.
    int posn = 0;
    int lastSpace = 0;
    while ( posn < str.length() && posn < 30 ) {
        if ( str[posn] == ' ' || str[posn] == '\t' ) {
            lastSpace = posn;
        } else if ( str[posn] == '\r' || str[posn] == '\n' ) {
            // Only return the first line if there is an EOL present.
            subject = str.left( posn );
            body = str;
            return;
        }
        ++posn;
    }
    if ( posn < 30 )
        lastSpace = posn;
    subject = str.left( lastSpace );
    body = str;
}

// Format an outgoing message using GSM 03.40 rules.
QString SmsClient::formatOutgoing( const QString& /*subject*/, const QString &body )
{
#if 0
    // If the subject is empty, then return the body as-is.
    if ( subject.isEmpty() || subject == tr("(no subject)") ) {
        return body;
    }

    // Concatenate the subject and body using GSM 03.40 encoding rules.
    return "##" + subject + "#" + body;
#endif

    // XXX - the subject formatting confuses some users, so it
    // has been disabled for the time being.
    return body;
}

bool SmsClient::hasDeleteImmediately() const
{
    return true;
}

void SmsClient::deleteImmediately(const QString& serverUid)
{
    // Bail out if the SIM does not actually contain this message.
    // This is probably because another SIM was inserted, or because
    // the message was deleted on another phone before the SIM was
    // re-inserted into this one.
    if ( !activeIds.contains( serverUid ) ) {
        return;
    }

    // Extract the qtopiaphone identity for the message.
    int posn = serverUid.lastIndexOf( QChar('>') );
    if ( posn < 0 )
        return;     // Shouldn't happen, but be careful anyway.
    QString id = serverUid.mid( posn + 1 );

    // Send the deletion request to the SIM.
    req->deleteMessage( id );

    // Remove the identifier from the serverUidlList in the account.
    if ( account ) {
        QStringList list = account->getUidlList();
        list.removeAll( serverUid );
        account->setUidlList( list );
    }

    // Remove the identifier from the active identifiers.
    // have to iterate
    while (activeIds.indexOf( serverUid ) != -1) {
        int idx = activeIds.indexOf( serverUid );
        activeIds.removeAt( idx );
        timeStamps.removeAt( idx );
    }
        
    // activeIds.removeAll( serverUid );
    
}

void SmsClient::resetNewMailCount()
{
    req->setUnreadCount( 0 );
}

void SmsClient::fetched( const QString& id, const QSMSMessage& message )
{
    if (!id.isEmpty()) {
        MailMessage mail;
        QString mailStr;
        QString subject;
        QString body;
        int part;

        // Construct a full identity for the message.  This should be
        // unique enough to identify messages on different SIM's.
        QDateTime dt = message.timestamp();
        QString identity = QString("sms:%1:%2%3%4%5%6%7:>%8")
                                .arg( simIdentity )
                                .arg( dt.date().year(), 4 )
                                .arg( dt.date().month(), 2 )
                                .arg( dt.date().day(), 2 )
                                .arg( dt.time().hour(), 2 )
                                .arg( dt.time().minute(), 2 )
                                .arg( dt.time().second(), 2 )
                                .arg( id );

        // Add it to the active list, so that we know what's on the
        // inserted SIM right now, as opposed to the cached copy in
        // the mailbox folder.
        activeIds += identity;
        timeStamps += dt;

        // If we already have this message in the mailbox, then ignore it.
        if ( account ) {
            QStringList list = account->getUidlList();
            if ( list.contains( identity ) ) {
                if ( account->deleteMail() )
                    deleteImmediately( id );
                req->nextMessage();
                return;
            }
            list += identity;
            account->setUidlList( list );
        }
        mail.setServerUid( identity );
        

        // If the sender is not set, but the recipient is, then this
        // is probably an outgoing message that was reflected back
        // by the phone simulator.
        if( !message.sender().isEmpty() )
            mail.setFrom( message.sender() + "@sms" );
        else if( !message.recipient().isEmpty() )
            mail.setFrom( message.recipient() + "@sms" );

        // Extract the subject and body.
        extractSubjectAndBody( message.text(), subject, body );

        // Set the subject from the first few words of the text.
        mail.setSubject( subject );

        // Determine if the entire body is text, or if it contains attachments.
        bool hasAttachments = false;
        QList<QSMSMessagePart> parts = message.parts();
        for ( part = 0; part < parts.count(); ++part ) {
            if ( !(parts[part].isText()) ) {
                hasAttachments = true;
                break;
            }
        }
        if( !hasAttachments ) {
            // Set the plaintext body (MailMessage will encode as UTF-8).
            mail.setPlainTextBody( body );
        } else {
            SMSDecoder::formatMessage( mail, message );
        }

        // Set the reception date.
        QDateTime date = message.timestamp();
        if (date.isValid())
            mail.setDateTime( date );
        else
            mail.setDateTime( QDateTime::currentDateTime() );

        // Synthesize some other headers that MailMessage::encodeMail can't do.
        mailStr = "X-Sms-Type: ";
        switch(message.messageType()) {
            case QSMSMessage::Normal:
                mailStr += "normal\n"; break;
            case QSMSMessage::CellBroadCast:
                mailStr += "broadcast\n"; break;
            case QSMSMessage::StatusReport:
                mailStr += "status-report\n"; break;
            default:
                mailStr += "unknown\n"; break;
        }
        mailStr += "Mime-Version: 1.0\n";
        if ( !hasAttachments )
            mailStr += "Content-Type: text/plain; charset=utf-8\n";

        // Create the final message.
        mail.encodeMail();
        mailStr += mail.toRFC822();

        Email newMail(mail);
        QUuid id;
        createMail(newMail,mailStr,identity,id,mailStr.length());
        emit newMessage(newMail);

        // If the "deleteMail" flag is set, then delete the message
        // from the SIM immediately, rather than waiting for later.
        if ( account && account->deleteMail() )
            deleteImmediately( id );

        req->nextMessage();

        sawNewMessage = true;
    } else {
        smsFetching = false;
        if ( sawNewMessage ) {
            // Check again, just in case another new message arrived
            // while we were performing the fetch.
            req->check();
            return;
        }

        // Make sure there are always 5 free slots on the SIM card
        // so there is enough space for the reception of new messages.
        if ( account && !account->deleteMail()
             && (req->totalMessages() - req->usedMessages()) < 5
             && req->totalMessages() >= 5
             && !timeStamps.isEmpty() ) {
            int toBeDeleted = 5 - (req->totalMessages() - req->usedMessages());
            while ( toBeDeleted-- > 0 && activeIds.size() > 0 ) {
                QDateTime dt = timeStamps[0];
                int index = 0;
                for (int i = 1; i < timeStamps.size(); ++i) {
                    if (timeStamps[i] < dt) {
                        dt = timeStamps[i];
                        index = i;
                    }
                }
                deleteImmediately( activeIds[index] );
                activeIds.removeAt( index );
                timeStamps.removeAt( index );
            }
        }
    }
}

void SmsClient::simIdentityChanged()
{
    if ( smsFetching && !haveSimIdentity ) {
        haveSimIdentity = true;
        simIdentity = simInfo->identity();
        sawNewMessage = false;
        activeIds.clear();
        timeStamps.clear();
        req->firstMessage();
    }
}

void SmsClient::smsReadyChanged()
{
    // Force a message check if the sim has just become ready.
    if ( req->ready() ) {
        req->check();
    }
}

void SmsClient::createMail(Email& mail, QString& message, QString& id, QUuid& internalId, uint size)
{
    mail.setStatus(EFlag_Incoming, true );
    mail.setUuid( internalId );

    QString str = message;
    QtMail::replace(str, QString::fromLatin1( "\r\n" ), QString::fromLatin1( "\n" ) );
    mail.fromRFC822( str );

    mail.setSize( size );
    mail.setStatus(EFlag_Downloaded, true);
    mail.setServerUid( id.mid( id.indexOf(" ") + 1, id.length() ) );

    mail.setServerUid(id);
    mail.setFromAccount( account->id() );
    mail.setType(MailMessage::SMS);
    mail.setFromMailbox("");
}
#endif //QTOPIA_NO_SMS
