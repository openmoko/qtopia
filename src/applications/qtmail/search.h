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



#ifndef SEARCH_H
#define SEARCH_H

#include <qstring.h>
#include <qobject.h>
#include "email.h"

class Search : public QObject
{
    Q_OBJECT

public:
    Search();

    enum SearchFlag {
        Any = 0,
        Read,
        Unread,
        Replied
    };
    typedef uint MailStatus;

    void reset();
    bool matches(Email *in);
    void setMailbox(QString mailbox);
    QString mailbox();
    QString name();
    void setName(QString in);
    void setMailFrom(QString from);
    void setMailTo(QString to);
    void setMailSubject(QString subject);
    void setMailBody(QString body);
    void setStatus(MailStatus s);
    void setBeforeDate(QDate date);
    void setAfterDate(QDate date);
    void setFromFolder(QString _folder);
    void setFromAccount(QString _fromAccount);

    uint status();
    QString getFrom();
    QString getTo();
    QString getSubject();
    QString getBody();
    QDate getBeforeDate();
    QDate getAfterDate();

private:
    bool matchesTo();
    bool matchesBody();
    bool matchesStatus();

    bool matchesBeforeDate();
    bool matchesAfterDate();
    bool matchesFolder();
    bool matchesAccount();

    bool match(const QString &source, const QString &target);

private:
    Email *mail;

    uint _status;
    QString _name, fromMail, recipient, subject, body, folder;
    QString fromAccount;
    QString _mailbox;
    QDate beforeDate, afterDate;
};

#endif
