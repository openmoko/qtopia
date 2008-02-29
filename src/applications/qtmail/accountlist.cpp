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

#include "accountlist.h"
#include "account.h"
#include <qsettings.h>
#include <qtopialog.h>

AccountList::AccountList(QObject *parent, const char *name)
    : QObject(parent)
{
    setObjectName( name );
}

QListIterator<MailAccount*> AccountList::accountIterator()
{
    return ( QListIterator<MailAccount*>(list) );
}

int AccountList::count()
{
    return list.count();
}

MailAccount* AccountList::at(int x) 
{
    return list.at(x); 
};


void AccountList::readAccounts()
{
    QSettings accountconf("Trolltech","qtmail_account");
    MailAccount *account;

    accountconf.beginGroup( "accountglobal" );
    int count = accountconf.value("accounts", 0).toInt();
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    bool smsExists = false;
#endif
    bool systemExists = false;
#ifndef QTOPIA_NO_MMS
    bool mmsExists = false;
#endif
#endif

    for (int x = 0; x < count; x++) {
        account = new MailAccount();
        accountconf.endGroup();
        accountconf.beginGroup( "account_" + QString::number(x) );
        account->readSettings(&accountconf);
        append(account);

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
        if (account->accountType() == MailAccount::SMS)
            smsExists = true;
#endif
        if (account->accountType() == MailAccount::System)
            systemExists = true;

#ifndef QTOPIA_NO_MMS
        else if (account->accountType() == MailAccount::MMS)
            mmsExists = true;
#endif
#endif
    }
    accountconf.endGroup();

    static const char *const account_names[] = {
        QT_TRANSLATE_NOOP( "AccountList", "SMS" ),
        QT_TRANSLATE_NOOP( "AccountList", "System" ),
        QT_TRANSLATE_NOOP( "AccountList", "MMS" ),
    };
    Q_UNUSED(account_names);

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    if (!smsExists) {
        account = new MailAccount();
        account->setAccountType( MailAccount::SMS );
        account->setAccountName( "SMS" );
        account->setMailServer( "SMS" );
        append(account);
    }
#endif
    if (!systemExists) {
        account = new MailAccount();
        account->setAccountType( MailAccount::System );
        account->setAccountName( "System" );
        account->setMailServer( "System" );
        append(account);
    }
#ifndef QTOPIA_NO_MMS
    if (!mmsExists) {
        qLog(Messaging) << "Adding MMS account";
        account = new MailAccount();
        account->setAccountType( MailAccount::MMS );
        account->setAccountName( "MMS" );
        account->setMailServer("MMS");
        append(account);
    }
#endif
#endif
}

void AccountList::saveAccounts()
{
    QSettings accountconf("Trolltech","qtmail_account");
    accountconf.beginGroup( "accountglobal" );

    accountconf.setValue("accounts", (int) count() );
    int count = 0;
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        accountconf.endGroup();
        accountconf.beginGroup( "account_" + QString::number(count) );
        accountconf.remove("");
        it.next()->saveSettings(&accountconf);
        count++;
    }

    // Just in case an account has been deleted, clear it in the conf file
    // Assumes this method is called each time an account is removed
    { accountconf.endGroup(); accountconf.beginGroup( "account_" + QString::number(count) ); };
    accountconf.remove("");

    accountconf.sync();
    accountconf.endGroup();
}

/* As we can manipulate each individual account outside this
   class, this is the only way of seeing whether more than one
   account has been made default (which doesn't make sense) */
uint AccountList::defaultMailServerCount()
{
    uint count = 0;

    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        if ( it.next()->defaultMailServer() )
            count++;
    }

    return count;
}

MailAccount* AccountList::defaultMailServer()
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if ( account->defaultMailServer() )
            return account;
    }
    return NULL;
}

void AccountList::setDefaultMailServer(MailAccount *account)
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *a = it.next();
        if ( a != account ) {
            a->setDefaultMailServer( false );
        } else {
            a->setDefaultMailServer( true );
        }
    }
}

QStringList AccountList::emailAccounts()
{
    QStringList l;

    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if ( !account->emailAddress().isEmpty() ) {
            if( account == defaultMailServer() )
                l.prepend( account->emailAddress() );
            else
                l.append( account->emailAddress() );
        }
    }

    return l;
}

MailAccount* AccountList::getSmtpRef()
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if ( !account->smtpServer().isEmpty() )
            return account;
    }

    return NULL;
}

// gets a user: oystein@pop.mymail.com and returns a reference
// to the correct mailaccount object.  Note: this will return
// any account it can find if the account doesn't exist.  For this
// reason, new code should use "getAccountById" instead and
// check for NULL.
MailAccount* AccountList::getPopRefByAccount(QString user)
{
    QString thisUser;
    MailAccount* any=0;
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *a = it.next();
        if ( !a->mailServer().isEmpty() ) {
            if ( !any ) any = a;
            thisUser = a->id();
            if (thisUser == user)
                return a;
        }
    }

    return any;
}

// gets an account that matches a particular id.  NULL if no such account.
MailAccount* AccountList::getAccountById(QString id)
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *a = it.next();
        if ( a->id() == id ) {
            return a;
        }
    }
    return 0;
}

/*  Find the account matching the email address */
MailAccount* AccountList::getSmtpRefByMail(QString email)
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if ( !account->smtpServer().isEmpty() ) {
            if ( account->emailAddress() == email )
                return account;
        }
    }

    return NULL;
}

//scans through the list to retrieve the first encountered username
QString AccountList::getUserName()
{
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if ( !account->userName().isEmpty() )
            return account->userName();
    }

    return "";
}

void AccountList::intervalCheck(MailAccount *account)
{
    int idCount = 0;
    QListIterator<MailAccount*> it(list);
    while (it.hasNext()) {
        MailAccount *a = it.next();
        if ( a == account ) {
            emit checkAccount( idCount );
            break;
        }
        idCount++;
    }
}

void AccountList::append(MailAccount *a)
{
    list.append(a);
    connect( a, SIGNAL(intervalCheck(MailAccount*)),
             this, SLOT(intervalCheck(MailAccount*)) );
};

void AccountList::remove(int pos)
{
    list.removeAt(pos);
    if ( at( pos ) )
        disconnect( at( pos ), SIGNAL(intervalCheck(MailAccount*)),
                    this, SLOT(intervalCheck(MailAccount*)) );
};

void AccountList::remove(MailAccount* const a)
{
    list.removeAll(a);
    disconnect( a, SIGNAL(intervalCheck(MailAccount*)),
                this, SLOT(intervalCheck(MailAccount*)) );
};
