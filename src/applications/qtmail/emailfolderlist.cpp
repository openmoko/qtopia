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



#include <qmessagebox.h>
#include <qstringlist.h>

#include <qtopiaapplication.h>
#if defined(QTOPIA_DESKTOP) && defined(Q_OS_WIN32)
#include <qtopia/pim/qtopiapim.h>
#define QTOPIAPIM_TEMPLATEDLL
#include <qtopia/pim/qtopiapimwinexport.h>
#endif

#ifndef Q_OS_WIN32
#ifndef Q_OS_MACX
#include <sys/vfs.h>
#endif
#endif

#include "emailfolderlist.h"
#include "common.h"
#include "accountlist.h"

using namespace QtMail;

/*  Mailbox List    */
MailboxList::MailboxList(QObject *parent)
    : QObject(parent )
{
    EmailFolderList *mailbox = new EmailFolderList(InboxString, this);
    connect(mailbox, SIGNAL( stringStatus(QString&) ), this,
            SIGNAL( stringStatus(QString&) ) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(OutboxString, this);
    connect(mailbox, SIGNAL( stringStatus(QString&) ), this,
            SIGNAL( stringStatus(QString&) ) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(TrashString, this);
    connect(mailbox, SIGNAL( stringStatus(QString&) ), this,
            SIGNAL( stringStatus(QString&) ) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(DraftsString, this);
    connect(mailbox, SIGNAL( stringStatus(QString&) ), this,
            SIGNAL( stringStatus(QString&) ) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(SentString, this);
    connect(mailbox, SIGNAL( stringStatus(QString&) ), this,
            SIGNAL( stringStatus(QString&) ) );
    _mailboxes.append( mailbox );
}

MailboxList::~MailboxList()
{
}

void MailboxList::openMailboxes()
{
    QListIterator<EmailFolderList*> it(_mailboxes);
    while (it.hasNext()) {
        EmailFolderList* current = it.next();
        current->openMailbox();

        //connect after mail has been read to speed up reading */
        connect(current, SIGNAL( mailAdded(Email*,const QString&) ), this,
                SIGNAL( mailAdded(Email*,const QString&) ) );
        connect(current, SIGNAL( mailUpdated(Email*,const QString&) ), this,
                SIGNAL( mailUpdated(Email*,const QString&) ) );
        connect(current, SIGNAL( mailRemoved(const QUuid&,const QString&) ), this,
                SIGNAL( mailRemoved(const QUuid&,const QString&) ) );
        connect(current, SIGNAL( externalEdit(const QString&) ), this,
                SIGNAL( externalEdit(const QString&) ) );
        connect(current, SIGNAL(mailMoved(Email*,const QString&,const QString&)),this,
                SIGNAL( mailMoved(Email*, const QString&, const QString&)));
    }
}

void MailboxList::writeDirtyHeaders()
{
    QListIterator<EmailFolderList*> it(_mailboxes);
    while (it.hasNext()) {
        it.next()->writeDirtyHeaders();
    }
}

void MailboxList::compact()
{
    QListIterator<EmailFolderList*> it(_mailboxes);
    while (it.hasNext()) {
        it.next()->compact();
    }
}

QStringList MailboxList::mailboxes() const
{
    QStringList list;

    QListIterator<EmailFolderList*> it(_mailboxes);
    while (it.hasNext()) {
        list.append( it.next()->mailbox() );
    }

    return list;
}

EmailFolderList* MailboxList::mailbox(const QString &name) const
{
    QListIterator<EmailFolderList*> it(_mailboxes);
    while (it.hasNext()) {
        EmailFolderList* current = it.next();
        if ( current->mailbox() == name )
            return current;
    }

    return NULL;
}

/*  Email Folder List */
EmailFolderList::EmailFolderList(QString mailbox, QObject *parent)
    : QObject(parent),_addOrder(0)
{
    //MaxMem can be exceeded, eg mailsize larger than MaxMem, and
    //maxmem refers to the ascii-size of the mail (eg. multiply by 2)
    MaxMem = 100 * 1024;   //swap out if exceeding maxmem, 100k

    e = new EmailFolderAccess( mailbox, this );
    connect(e, SIGNAL( mailboxChanged() ), this, SLOT( externalChange() ) );
}

EmailFolderList::~EmailFolderList()
{
    QListIterator<Email*> it(entryList);
    while (it.hasNext()) {
        delete it.next();
    }
    delete e;
}

void EmailFolderList::openMailbox()
{
    QString m = tr( "Reading mailbox, please wait" );
    emit stringStatus( m );

    QString mbox = mailboxTrName( e->mailbox() );
    EmailFolderAccess::MailboxStatus status = e->status();

    if ( status == EmailFolderAccess::OldVersion || status == EmailFolderAccess::IndexFileDeleted ) {
        QString msg;
        switch( status ) {
            case EmailFolderAccess::IndexFileDeleted:
            {
                msg = tr("<qt>Index files have been deleted. "
                         "They must be recreated to access the "
                         "%1 folder.<br>Recreate now?</qt>").arg(mbox);
                break;
            }
            case EmailFolderAccess::OldVersion:
            {
                msg = tr("<qt>Mailbox is in older format. "
                         "It must be upgraded to access the "
                         "%1 folder.<br>Upgrade now?").arg(mbox);
                break;
            }
            default:
            {
                msg = tr("<qt>Index files have been deleted. "
                         "They must be recreated to access the "
                         "%1 folder.<br>Recreate now?").arg(mbox);
            }
        }

        switch( QMessageBox::warning( qApp->activeWindow(), tr("Email - Invalid index files"), msg,
            tr("Yes"), tr("No"), 0, 0, 1 ) ) {

            case 0:
            {
                e->recreateIndexFiles();
                break;
            }
            case 1: break;
        }
    } else if ( status != EmailFolderAccess::Open ) {
        QString msg = tr("<qt>Could not open folder %1</qt>").arg(mbox);
        QMessageBox::warning( qApp->activeWindow(), tr("Email - Could not open mailbox"), msg);
    }

    QList<QUuid> list = e->mailUuidList();
    QListIterator<QUuid> it(list);

    Email *mail;
    uint arrival = 0;
    while (it.hasNext()) {
        mail = new Email( e->mailContextInfo( it.next() ), this );
        mail->setArrival( arrival );    //maintain a notion of arrival time, by their sequential position in the file

        entryList.append( mail);
        arrival++;
    }
    _addOrder = entryList.count();
}

bool EmailFolderList::addMail(const Email &m)
{
    Email *mail = new Email(m, this);
    mail->setArrival( _addOrder );

    if ( e->hasMail( mail->uuid() ) ) {

        if ( !e->updateFullMail( *((MailMessage *) mail) ) )
            return false;

        QUuid id = mail->uuid();
        delete mail;
        Email *mailContext = new Email(e->mailContextInfo( id ), this);


        Email *old = emailRef( id );
        if ( old ) {
            entryList.removeAll( old );
            mailContext->setArrival( old->arrival() );      //maintain old arrival time (since it's just updated)
        qWarning("Setting old arrival order %d",old->arrival());
            entryList.append( mailContext );
        _addOrder++;
            emit mailUpdated( mailContext, mailbox() );

            delete old;
        }
    } else {
        if ( !e->addMail( *((MailMessage *) mail) ) )
            return false;

        QUuid id = mail->uuid();
        delete mail;
    Email *mailContext = new Email(e->mailContextInfo( id ), this);
    mailContext->setArrival(_addOrder);
        entryList.append( mailContext );
    ++_addOrder;

        emit mailAdded(mailContext, mailbox() );
    }

    return true;
}

Email* EmailFolderList::emailRef(QUuid id)
{
    Email *entry;
    QListIterator<Email*> it(entryList);
    while (it.hasNext()) {
        entry = it.next();
        if ( entry->uuid() == id )
            return entry;
    }

    return NULL;
}

bool EmailFolderList::removeMail(QUuid id, bool /* expunge */)
{
    if ( !e->removeMail(id) )
        return false;

    Email *mail = emailRef( id );
    if ( mail ) {
        entryList.removeAll( mail );

        emit mailRemoved(id, mailbox() );

        // delete after mailRemoved
        delete mail;

        return true;
    }

    return false;
}

bool EmailFolderList::empty()
{
    QListIterator<Email*> it(entryList);
    while ( it.hasNext() ) {
        Email *mail = it.next();
        if ( e->removeMail( mail->uuid() ) )
            if ( entryList.removeAll( mail ) )
                delete mail;
    }

    return true;
}

void EmailFolderList::compact()
{
    e->autoCompact();
}

void EmailFolderList::writeDirtyHeaders()
{
    QListIterator<Email*> it(entryList);
    while ( it.hasNext() ) {
        Email *mail = it.next();
        if ( mail->dirty() ) {
            if ( e->updateMailContextInfo( *mail ) ) {
                mail->setDirty( false );
            }
        }
    }
}

QListIterator<Email*> EmailFolderList::entryIterator()
{
    return ( QListIterator<Email*>(entryList) );
}

Email* EmailFolderList::email(QUuid id)
{
    QListIterator<Email*> it(entryList);
    while ( it.hasNext() ) {
        Email *mail = it.next();
        if ( mail->uuid() == id )
            return mail;
    }

    return NULL;
}

QUuid EmailFolderList::generateUuid()
{
    return e->generateUuid();
}

uint EmailFolderList::mailCount(const QString &type, AccountList *accountList)
{
    QMap<MailAccount*,int> accountMap;
    if ( accountList ) {
        QListIterator<MailAccount*> it2 = accountList->accountIterator();
        while (it2.hasNext()) {
          accountMap.insert( it2.next(), 0 );
        }
    }

    uint count = 0;
    if ( type == "all") { // No tr
        count = entryList.count();
    } else if ( type == "new" ) { // No tr
        QListIterator<Email*> it(entryList);
        while( it.hasNext() ) {
            Email *mail = it.next();
            if ( mail->status(EFlag_Read) )
                continue;
            count++;

            if ( accountList ) {
                QListIterator<MailAccount*> it2 = accountList->accountIterator();
                while (it2.hasNext()) {
                    MailAccount *account = it2.next();
                    if ( account->matchesEmail( mail ) ) {
                        int oldCount = accountMap[account];
                        accountMap.insert( account, oldCount + 1 );
                    }
                }
            }
        }
    } else if ( type == "unsent") { // No tr
        QListIterator<Email*> it(entryList);
        while (it.hasNext()) {
            Email *mail = it.next();
            if ( !mail->status(EFlag_Sent) && !mail->unfinished() )
               count++;
        }
    }

    if ( accountList ) {
        QListIterator<MailAccount*> it2 = accountList->accountIterator();
        while (it2.hasNext()) {
            MailAccount *account = it2.next();
            account->setUnreadCount( accountMap[account] );
        }
    }
    return count;
}

void EmailFolderList::externalChange()
{
    qWarning(QString("mailbox external change: " + e->mailbox()).toLatin1() );


    QListIterator<Email*> it(entryList);
    while (it.hasNext()) {
        delete it.next();
    }
    entryList.clear();

    openMailbox();

    emit externalEdit( e->mailbox() );
}

bool EmailFolderList::swapMailIn(Email *mail)
{
    swapMailOut();

    uint arrival = mail->arrival();

    *( (MailMessage *) mail) = e->fullMail( mail->uuid() );
    mail->reparent(this);

    mail->setInMemory( true );
    //restore the arrival order
    mail->setArrival(arrival);

    return true;
}

/*  Ignore this one for now */
bool EmailFolderList::swapMailOut()
{
    QListIterator<Email*> it(entryList);

    MailMessage m;
    while (it.hasNext()) {
        Email *mail = it.next();
        if ( mail->inMemory() ) {
        uint oldArrival = mail->arrival();

            // write out header if it has changed (eg. read)
            if ( mail->dirty() ) {
                e->updateMailContextInfo( *mail );
                mail->setDirty( false );
            }

            *mail = e->mailContextInfo( mail->uuid() );
            mail->reparent(this);
            mail->setInMemory( false );
        //restore the arrival order
        mail->setArrival(oldArrival);
        }
    }

    return true;
}

bool EmailFolderList::moveMail(const QUuid& id, EmailFolderList& dest)
{
    Email* mail = emailRef(id);

    if(!mail)
    {
        qWarning("Move failed. Mail does not exist in this mailbox %s",qPrintable(mailbox()));
        return false;
    }

    if(!e->moveMail(id,*dest.e))
        return false;

    //copy the mail pointers across

    entryList.removeAll(mail);
    mail->reparent(&dest);
    dest.entryList.append(mail);
    emit mailMoved(mail,mailbox(),dest.mailbox());
    return true;
}

bool EmailFolderList::copyMail(const QUuid& id, EmailFolderList& dest)
{
    Email* mail = emailRef(id);

    if(!mail)
    {
        qWarning("Copy failed. Mail does not exist in this mailbox %s",qPrintable(mailbox()));
        return false;
    }
    QUuid newId;
    if(!e->copyMail(id,*dest.e,newId))
        return false;

    //TODO fix fugly

    Email* copiedMail = new Email(dest.e->mailContextInfo( newId ),&dest);
    dest.entryList.append(copiedMail);
    emit mailAdded(copiedMail,dest.mailbox());
    return true;
}

