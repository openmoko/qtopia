/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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



#ifndef EMAILFOLDERLIST_H
#define EMAILFOLDERLIST_H

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qfile.h>
#include <qtextstream.h>

#include <qtopiaglobal.h>
#include <QMailMessage>
#include <QMailFolder>
#include <QMailMessageKey>
#include <QMailMessageSortKey>

class EmailFolderList;
class AccountList;

enum IconType
{
    NoIcon = 0,
    AllMessages = 1,
    UnreadMessages = 2,
    UnsentMessages = 3
};

class QTOPIAMAIL_EXPORT MailboxList : public QObject
{
    Q_OBJECT

public:
    MailboxList(QObject *parent = 0);
    ~MailboxList();

    void openMailboxes();

    QStringList mailboxes() const;
    EmailFolderList* mailbox(const QString &name) const;
    EmailFolderList* mailbox(const QMailId& mailFolderId) const;

    // Identifier strings not visible to user
    static const char* InboxString;
    static const char* OutboxString;
    static const char* DraftsString;
    static const char* SentString;
    static const char* TrashString;
    static const char* LastSearchString;

    // Get the translated name for a mailbox
    static QString mailboxTrName( const QString &s );

    // Get the translated header text for a mailbox
    static QString mailboxTrHeader( const QString &s );

    // Get an icon for mailbox.
    static QIcon mailboxIcon( const QString &s );

signals:
    void externalEdit(const QString &);
    void mailAdded(const QMailId& id, const QString &);
    void mailUpdated(const QMailId& id, const QString &);
    void mailRemoved(const QMailId &, const QString &);
    void mailMoved(const QMailId& id, const QString&, const QString&);
    void mailMoved(const QMailIdList& list, const QString&, const QString&);
    void stringStatus(QString &);

private:
    QList<EmailFolderList*> _mailboxes;
};

class QTOPIAMAIL_EXPORT EmailFolderList : public QObject
{
    Q_OBJECT

public:
    enum MailType { All, New, Unsent, Unfinished };
    enum SortOrder { Submission, AscendingDate, DescendingDate };

    EmailFolderList(QString mailbox, QObject *parent=0);
    ~EmailFolderList();

    void openMailbox();
    bool addMail(QMailMessage &mail);
    bool removeMail(QMailId id);
    bool moveMail(const QMailId& id, EmailFolderList& dest);
    bool copyMail(const QMailId& id, EmailFolderList& dest);
    bool empty(int type = QMailMessage::AnyType);

    bool moveMailList(const QMailIdList& list, EmailFolderList& dest);

    QString mailbox() const;
    QMailFolder mailFolder() const;

    QMailIdList messages(unsigned int type = QMailMessage::AnyType, 
                         const SortOrder& order = Submission ) const;

    QMailIdList messages(const QMailMessage::Status& status, 
                         bool contains,
                         unsigned int type = QMailMessage::AnyType,
                         const SortOrder& order = Submission ) const;

    QMailIdList messagesFromMailbox(const QString& mailbox, 
                                    unsigned int type = QMailMessage::AnyType,
                                    const SortOrder& order = Submission ) const;

    QMailIdList messagesFromAccount(const QString& account, 
                                    unsigned int type = QMailMessage::AnyType,
                                    const SortOrder& order = Submission ) const;

    bool contains(const QMailId& id) const;

    // If accountList is set and type is New then as a side effect 
    // the unread count for all accounts is updated by this call.
    uint mailCount(MailType status, 
		   int type = QMailMessage::AnyType,
		   AccountList *accountList = 0);

signals:
    void externalEdit(const QString &);
    void mailAdded(const QMailId& id, const QString &);
    void mailUpdated(const QMailId& id, const QString &);
    void mailRemoved(const QMailId& id, const QString &);
    void mailMoved(const QMailId& id, const QString&, const QString&);
    void mailMoved(const QMailIdList& list, const QString&, const QString&);
    void stringStatus(QString &);

protected slots:
    void externalChange();

private:
    QString mMailbox;
    QMailFolder mFolder;
    QMailMessageKey mParentFolderKey;
};

#endif
