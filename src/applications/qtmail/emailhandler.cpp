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


#include "emailhandler.h"
#ifndef QTOPIA_NO_MMS
#include "mmsclient.h"
#endif

#include "common.h"

#include <qmimetype.h>

#include <QFileInfo>
#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <longstream.h>
#include <qtopialog.h>

EmailHandler::EmailHandler()
{
    LongStream::cleanupTempFiles();
    mailAccount = 0;
    smtpAccount = 0;
    smsAccount = 0;
    mmsAccount = 0;
    smtpClient = new SmtpClient();
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    smsClient = new SmsClient();
#endif
#ifndef QTOPIA_NO_MMS
    mmsClient = new MmsClient();
#endif
#endif
    popClient = new PopClient();
    imapClient = new ImapClient();

    connectClient(smtpClient, Sending, SIGNAL(smtpError(int,QString&)));

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    connectClient(smsClient, Sending|Receiving, SIGNAL(smsError(int,QString&)));
#endif
#ifndef QTOPIA_NO_MMS
    connectClient(mmsClient, Sending|Receiving, SIGNAL(mmsError(int,QString&)));
#endif
#endif
    connectClient(popClient, Receiving, SIGNAL(popError(int,QString&)));
    connectClient(imapClient, Receiving, SIGNAL(popError(int,QString&)));
    receiving = false;
}

int EmailHandler::unreceivedSmsCount()
{
#ifndef QTOPIA_NO_SMS
    return smsClient->unreceivedSmsCount();
#else
    return 0;
#endif
}

bool EmailHandler::smsReadyToDelete() const
{
#ifndef QTOPIA_NO_SMS
    return smsClient->readyToDelete();
#else
    return false;
#endif
}

EmailHandler::~EmailHandler()
{
    delete smtpClient;
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    delete smsClient;
#endif
#ifndef QTOPIA_NO_MMS
    delete mmsClient;
#endif
#endif
    delete popClient;
    delete imapClient;
}

void EmailHandler::connectClient(Client *client, int type, QString sigName)
{
    connect(client, SIGNAL(errorOccurred(int,QString&)),sigName.toAscii() );
    connect(client, SIGNAL(mailSent(int)), this, SIGNAL(mailSent(int)) );

    if (type & Receiving)
    {
        connect(client, SIGNAL(updateStatus(const QString&)), this,
                SIGNAL(updatePopStatus(const QString&)) );

        connect(client, SIGNAL(newMessage(const Email&)),
                this, SIGNAL(mailArrived(const Email&)) );
    }
    if (type & Sending)
        connect(client, SIGNAL(updateStatus(const QString&)), this,
                SIGNAL(updateSendingStatus(const QString&)) );

    connect(client, SIGNAL(mailTransferred(int)), this,
            SIGNAL(mailTransferred(int)) );
    connect(client, SIGNAL( unresolvedUidlList(QStringList&) ),
            this, SLOT( unresolvedUidl(QStringList&) ) );
    connect(client, SIGNAL(serverFolders()), this,
            SIGNAL(serverFolders()) );

    //relaying size information
    connect(client, SIGNAL(downloadedSize(int)),
        this, SIGNAL(downloadedSize(int)) );
    connect(client, SIGNAL(mailboxSize(int)),
        this, SIGNAL(mailboxSize(int)) );
    connect(client, SIGNAL(transferredSize(int)),
        this, SIGNAL(transferredSize(int)) );
    connect(client, SIGNAL( failedList(QStringList&) ),
        this, SIGNAL( failedList(QStringList&) ) );
}

void EmailHandler::sendMail(QList<Email*>* mailList)
{
    Email *currentMail;
    QString temp;
    QStringList combinedList;
    bool allOk = (mailList->count() > 0);
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    bool smsOk = false;
#endif
#ifndef QTOPIA_NO_MMS
    bool mmsOk = false;
#endif
#endif
    bool smtpOk = false;
    int allMailSize = 0;

    for (int i = 0; i < mailList->count(); i++) {
    currentMail = mailList->at(i);

    currentMail->readFromFile();

        if ( currentMail->encodeMail() ) {
            if ( !currentMail->hasRecipients() ) {
                temp = tr("No recipients specified for\n mail with subject:\n"
                          "%1\nNO mail has been sent.")
                    .arg( currentMail->subject() );
                QMessageBox::warning(qApp->activeWindow(),
                                     tr("Mail encoding error"),
                                     temp, tr("OK"));
                allOk = false;
            } else {
#ifdef QTOPIA_PHONE
        //mms message
#ifndef QTOPIA_NO_MMS
        if (currentMail->type() & MailMessage::MMS) {
            qLog(Messaging) << "Detected MMS message";
            if (allOk)
                mmsOk = true;
            mmsClient->addMail(*currentMail);
            continue;
        }
#endif

#ifndef QTOPIA_NO_SMS
        // sms message
        QStringList phoneRecipients = currentMail->phoneRecipients();

        if(phoneRecipients.count() > 0)
        {
            qLog(Messaging) << "Detected SMS Message";
            int mailSendSize = smsClient->addMail(currentMail);
            allOk = (mailSendSize >= 0);
            if(allOk)
                allMailSize += mailSendSize;
            smsOk = allOk;
        }
#endif

        if (!allOk)
            continue;
#endif

        //email message
        if(currentMail->mailRecipients().isEmpty())
            continue;

        qLog(Messaging) << "Detected Email Message";

        int mailSendSize = smtpClient->addMail(currentMail);
        allOk = (mailSendSize >= 0);
        if(allOk)
            allMailSize += mailSendSize;
        smtpOk = allOk;
    }
        } else {                                                        //error
            temp = tr("Could not locate all files\nin mail with subject:\n"
                      "%1\nNO mail has been sent")
                .arg( currentMail->subject() );

            QMessageBox::warning(qApp->activeWindow(), tr( "Mail encoding error" ), temp, tr( "OK" ));
            allOk = false;
    }
    }

    if (allOk && (smtpOk
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
                || smsOk
#endif
#ifndef QTOPIA_NO_MMS
                || mmsOk
#endif
#endif
                )) {
        if (smtpOk) {
            smtpClient->setAccount(smtpAccount);
            smtpClient->newConnection();
        }
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
        if (smsOk)
            smsClient->newConnection();
        else
            smsClient->clearList();
#endif
#ifndef QTOPIA_NO_MMS
        if (mmsOk)
            mmsClient->newConnection();
#endif
#endif
        emit mailSendSize(allMailSize);
    } else {
        emit mailSent(-1);
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
        smsClient->clearList();
#endif
#endif
    }
}

void EmailHandler::setSmtpAccount(MailAccount *account)
{
    smtpAccount = account;
}

void EmailHandler::setMailAccount(MailAccount *account)
{
    mailAccount = account;
}

void EmailHandler::setSmsAccount(MailAccount *account)
{
    smsAccount = account;
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    smsClient->setAccount( account );
#endif
#endif
}


void EmailHandler::setMmsAccount(MailAccount *account)
{
    mmsAccount = account;
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    mmsClient->setAccount(account);
#endif
#endif
}

void EmailHandler::getMailHeaders()
{
    Client *client = clientFromAccount(mailAccount);
    if ( mailAccount->accountType() == MailAccount::POP )
        receiving = true; // maybe this should be unconditional

    if (client) {
        client->setAccount(mailAccount);
        client->headersOnly(true, 2000);        //less than 2000, download all
        client->newConnection();
    }
}

void EmailHandler::getMailByList(MailList *mailList, bool newConnection)
{
    if (mailList->count() == 0) {       //should not occur though
        emit mailTransferred(0);
        return;
    }
    receiving = true;

    Client *client = clientFromAccount(mailAccount);

    if (client) {
        client->headersOnly(false, 0);

        if (newConnection) {
            client->setAccount(mailAccount);
            client->newConnection();
        }

        client->setSelectedMails(mailList, !newConnection);
    }
}

void EmailHandler::popQuit()
{
    Client *client = clientFromAccount(mailAccount);
    if (client)
        client->quit();

    receiving = false;
}

void EmailHandler::acceptMail(const Email &mail)
{
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    if (mmsClient && mail.type() == MailMessage::MMS) {
        mmsClient->sendNotifyResp(mail, "Deferred");
    }
#elif !defined(QTOPIA_DESKTOP)
    //Q_CONST_UNUSED(mail);
    (void)mail;
#endif
}

void EmailHandler::rejectMail(const Email &mail)
{
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    if (mmsClient && mail.type() == MailMessage::MMS) {
        mmsClient->sendNotifyResp(mail, "Rejected");
    }
#elif !defined(QTOPIA_DESKTOP)
    //Q_CONST_UNUSED(mail);
    (void)mail;
#endif
}

void EmailHandler::unresolvedUidl(QStringList &list)
{
    QString user = mailAccount->id();

    emit unresolvedUidlList(user, list);
}

void EmailHandler::mailRead(Email *mail)
{
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_SMS)
    smsClient->mailRead( mail );
#else
    Q_UNUSED(mail);
#endif
}

void EmailHandler::cancel()
{
    QString msg = tr( "Cancelled by user" );

    receiving = false;

    /* all clients handles this call regardless of whether
        they are actually in use or not (disregarded) */
    popClient->errorHandling(ErrCancel, msg);
    imapClient->errorHandling(ErrCancel, msg);
    smtpClient->errorHandling(ErrCancel, msg);
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    smsClient->errorHandling(ErrCancel, msg);
#endif
#ifndef QTOPIA_NO_MMS
    mmsClient->errorHandling(ErrCancel, msg);
#endif
#endif
}

Client* EmailHandler::clientFromAccount(MailAccount *account)
{
    if ( account->accountType() == MailAccount::POP )
        return popClient;
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    else if ( account->accountType() == MailAccount::MMS)
        return mmsClient;
#endif
#ifndef QTOPIA_NO_SMS
    else if ( account->accountType() == MailAccount::SMS)
        return smsClient;
#endif
#endif
    else if ( account->accountType() == MailAccount::IMAP)
        return imapClient;
    return 0;
}
