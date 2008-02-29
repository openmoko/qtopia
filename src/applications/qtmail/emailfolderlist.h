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



#ifndef EMAILFOLDERLIST_H
#define EMAILFOLDERLIST_H

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qfile.h>
#include <qtextstream.h>

#include <qtopia/mail/emailfolderaccess.h>
#include <qtopia/mail/mailmessage.h>
#include "email.h"

class EmailFolderList;
class AccountList;

class MailboxList : public QObject
{
    Q_OBJECT

public:
    MailboxList(QObject *parent = 0);
    ~MailboxList();

    void openMailboxes();
    void writeDirtyHeaders();
    void compact();

    QStringList mailboxes() const;
    EmailFolderList* mailbox(const QString &name) const;

signals:
    void externalEdit(const QString &);
    void mailAdded(Email *, const QString &);
    void mailUpdated(Email *, const QString &);
    void mailRemoved(const QUuid &, const QString &);
    void mailMoved(Email*, const QString&, const QString&);
    void stringStatus(QString &);

private:
    QList<EmailFolderList*> _mailboxes;
};

class EmailFolderList : public QObject
{
    Q_OBJECT

public:
    EmailFolderList(QString mailbox, QObject *parent=0);
    ~EmailFolderList();

    void openMailbox();
    bool addMail(const Email &mail);
    bool removeMail(QUuid id, bool expunge);
    bool moveMail(const QUuid& id, EmailFolderList& dest);
    bool copyMail(const QUuid& id, EmailFolderList& dest);
    bool empty();
    void writeDirtyHeaders();
    void compact();

    QString mailbox() const { return e->mailbox(); };

    QListIterator<Email*> entryIterator();
    Email* email(QUuid id);

    // If accountList is set then as a side effect the unread count
    // for all accounts is updated by this call.
    uint mailCount(const QString &type, AccountList *accountList = 0);

    QUuid generateUuid();

    bool swapMailIn(Email *);

signals:
    void externalEdit(const QString &);
    void mailAdded(Email *, const QString &);
    void mailUpdated(Email *, const QString &);
    void mailRemoved(const QUuid &, const QString &);
    void mailMoved(Email*, const QString&, const QString&);
    void stringStatus(QString &);

protected slots:
    void externalChange();

    bool swapMailOut();

private:
   Email *emailRef(QUuid id);

private:
    QList<Email*> entryList;
    uint MaxMem;
    EmailFolderAccess *e;
    int _addOrder;
};

#endif
