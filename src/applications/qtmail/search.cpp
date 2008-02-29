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



#include <qdatetime.h>

#include "search.h"

Search::Search()
{
    reset();
}

void Search::reset()
{
    _status = Any;

    _mailbox = "inbox";
    _name = "";
    fromMail = "";
    recipient = "";
    subject = "";
    body = "";
    folder = "";
    fromAccount = "";

    beforeDate = QDate();
    afterDate = QDate();
}

void Search::setMailbox(QString mailbox)
{
    _mailbox = mailbox.toLower();
}

QString Search::mailbox()
{
    return _mailbox;
}

void Search::setName(QString in)
{
    _name = in;
}

QString Search::name()
{
    if ( !_name.isEmpty() ) {
        return _name;
    } else {
        return tr("(No name)");
    }
}

// organized after lest expensive search
bool Search::matches(Email *in)
{
    mail = in;

    if ( !matchesStatus() )
        return false;

    if ( !matchesAccount() )
        return false;
    if ( !matchesFolder() )
        return false;

    if ( !match(fromMail, mail->from() ) )
        return false;

    if ( !matchesTo() )
        return false;

    if ( !match(subject, mail->subject() ) )
        return false;

    if ( !matchesBeforeDate() )
        return false;
    if ( !matchesAfterDate() )
        return false;

    if ( !matchesBody() )
        return false;

    return true;
}

void Search::setMailFrom(QString from)
{
    fromMail = from;
}

void Search::setMailTo(QString to)
{
    recipient = to;
}

void Search::setMailSubject(QString subject)
{
    this->subject = subject;
}

void Search::setMailBody(QString body)
{
    this->body = body;
}

void Search::setStatus(MailStatus s)
{
    _status = s;
}

uint Search::status()
{
    return _status;
}

void Search::setBeforeDate(QDate date)
{
    beforeDate = date;
}

void Search::setAfterDate(QDate date)
{
    afterDate = date;
}

void Search::setFromFolder(QString _folder)
{
    folder = _folder;
}

void Search::setFromAccount(QString _fromAccount)
{
    fromAccount = _fromAccount;
}

/*  TODO:  We should swap the mail in to get the full to, cc and bcc
    lists.  Only to.first() are currently cached.  Reading all the mails
    from disk are currently to slow a process though
*/
bool Search::matchesTo()
{
    if ( recipient.isEmpty() )
        return true;

    QStringList list = mail->to();
    for (QStringList::Iterator it = list.begin(); it != list.end(); it++) {
        if ( match( recipient, *it ) )
            return true;
    }

    return false;
}

/*  Allows matching of subsets.  If source is of type A, B, C then
    any instances of A B or C in target will return true.  Eg. you can match
    all mail from: Peter Pan, Wendy, Captain Hook
*/
bool Search::match(const QString &source, const QString &target)
{
    QStringList list = source.split(",");
    if ( list.count() == 0 )
        return true;

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        if ( target.indexOf( (*it).trimmed(), 0, Qt::CaseInsensitive) != -1 )
            return true;
    }

    return false;
}

/*  Reading entire mailbox is very slow on a large mailbox  */
bool Search::matchesBody()
{
    if ( body.isEmpty() )
        return true;

    mail->readFromFile();

    return match(body, mail->plainTextBody());
}

bool Search::matchesStatus()
{
    switch( _status ) {
        case Any:       return true;
        case Read:      return ( mail->status(EFlag_Read) );
        case Unread:    return ( !mail->status(EFlag_Read) );
        case Replied:   return ( mail->status(EFlag_Replied) || mail->status(EFlag_RepliedAll) );
    }

    return false;
}

// match against date, if the mails date is not parsed, always return true
bool Search::matchesBeforeDate()
{
    if ( beforeDate.isNull() )
        return true;
    if ( mail->dateTime().isNull() )
        return true;

    if ( beforeDate > mail->dateTime().date() )
        return true;

    return false;
}

// match against date, if the mails date is not parsed, always return true
bool Search::matchesAfterDate()
{
    if ( afterDate.isNull() )
        return true;
    if ( mail->dateTime().isNull() )
        return true;

    if ( afterDate < mail->dateTime().date() )
        return true;

    return false;
}

bool Search::matchesFolder()
{
    if ( folder.isEmpty() )
        return true;

    return ( folder == mail->fromMailbox() );
}

bool Search::matchesAccount()
{
    if ( fromAccount.isEmpty() )
        return true;
    if (fromAccount == mail->fromAccount() )
        return true;

    return false;
}

QString Search::getFrom()
{
    return fromMail;
}

QString Search::getTo()
{
    return recipient;
}

QString Search::getSubject()
{
    return subject;
}

QString Search::getBody()
{
    return body;
}

QDate Search::getBeforeDate()
{
    return beforeDate;
}

QDate Search::getAfterDate()
{
    return afterDate;
}

