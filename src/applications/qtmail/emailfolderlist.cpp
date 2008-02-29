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

#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtopiaapplication.h>
#include <sys/vfs.h>

#include "emailfolderlist.h"
#include "accountlist.h"
#include <QMailFolderKey>
#include <QMailStore>

const char* MailboxList::InboxString = "inbox_ident";     // No tr
const char* MailboxList::OutboxString = "outbox_ident"; // No tr
const char* MailboxList::DraftsString = "drafts_ident"; // No tr
const char* MailboxList::SentString = "sent_ident"; // No tr
const char* MailboxList::TrashString = "trash_ident"; // No tr
const char* MailboxList::LastSearchString = "last_search_ident"; // No tr

// Stores the translated names of mailbox properties
struct MailboxProperties
{
    MailboxProperties( const char* nameSource, const char* headerSource, const char *iconSource ) :
        name( qApp->translate( "QtMail", nameSource ) ),
        header( QObject::tr( headerSource ) ),
        icon( iconSource )
    {
    }

    QString name;
    QString header;
    const char *icon;
};

typedef QHash<QString, MailboxProperties> MailboxGroup;

// Returns a map containing translated properties for well-known mailboxes
static MailboxGroup initMailboxTr()
{
    MailboxGroup map;

    // Translated properties are stored in a map (perhaps a hash would be 
    // preferable?)
    // Heap storage is required, but lookup will be quick, and translation 
    // occurs only once
    map.insert( MailboxList::InboxString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Inbox"), 
                                   QT_TRANSLATE_NOOP( "QtMail", "From"),
                                   ":icon/inbox") );
    map.insert( MailboxList::OutboxString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Outbox"), 
                                   QT_TRANSLATE_NOOP( "QtMail", "To"),
                                   ":icon/outbox") );
    map.insert( MailboxList::TrashString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Trash"), 
                                   QT_TRANSLATE_NOOP( "QtMail", "From/To"),
                                   ":icon/trash") );
    map.insert( MailboxList::SentString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Sent"), 
                                   QT_TRANSLATE_NOOP( "QtMail", "To"),
                                   ":icon/sent") );
    map.insert( MailboxList::DraftsString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Drafts"), 
                                   QT_TRANSLATE_NOOP( "QtMail", "To"),
                                   ":icon/drafts") );
    map.insert( MailboxList::LastSearchString, 
                MailboxProperties( QT_TRANSLATE_NOOP( "QtMail", "Last search"),
                                   "",
                                   ":icon/find") );

    return map;
}

static const MailboxProperties* mailboxProperties( const QString &s )
{
    static MailboxGroup map( initMailboxTr() );

    // Search for the properties of the named mailbox
    MailboxGroup::const_iterator i = map.find(s);
    if ( i != map.end() )
        return &(i.value());

    return 0;
}


/*  Mailbox List    */
MailboxList::MailboxList(QObject *parent)
    : QObject(parent )
{
    EmailFolderList *mailbox = new EmailFolderList(InboxString, this);
    connect(mailbox, SIGNAL(stringStatus(QString&)), this,
            SIGNAL(stringStatus(QString&)) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(OutboxString, this);
    connect(mailbox, SIGNAL(stringStatus(QString&)), this,
            SIGNAL(stringStatus(QString&)) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(TrashString, this);
    connect(mailbox, SIGNAL(stringStatus(QString&)), this,
            SIGNAL(stringStatus(QString&)) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(DraftsString, this);
    connect(mailbox, SIGNAL(stringStatus(QString&)), this,
            SIGNAL(stringStatus(QString&)) );
    _mailboxes.append( mailbox );

    mailbox = new EmailFolderList(SentString, this);
    connect(mailbox, SIGNAL(stringStatus(QString&)), this,
            SIGNAL(stringStatus(QString&)) );
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
        connect(current, SIGNAL(mailAdded(QMailId,QString)), this,
                SIGNAL(mailAdded(QMailId,QString)) );
        connect(current, SIGNAL(mailUpdated(QMailId,QString)), this,
                SIGNAL(mailUpdated(QMailId,QString)) );
        connect(current, SIGNAL(mailRemoved(QMailId,QString)), this,
                SIGNAL(mailRemoved(QMailId,QString)) );
        connect(current, SIGNAL(externalEdit(QString)), this,
                SIGNAL(externalEdit(QString)) );
        connect(current, SIGNAL(mailMoved(QMailId,QString,QString)),this,
                SIGNAL(mailMoved(QMailId,QString,QString)));
        connect(current, SIGNAL(mailMoved(QMailIdList,QString,QString)),this,
                SIGNAL(mailMoved(QMailIdList,QString,QString)));
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

EmailFolderList* MailboxList::mailbox(const QMailId& mailFolderId) const
{
    QListIterator<EmailFolderList*> it(_mailboxes);

    foreach(EmailFolderList* f, _mailboxes)
    {
        if(f->mailFolder().id() == mailFolderId)
            return f;
    }
    
    return NULL;

}

QString MailboxList::mailboxTrName( const QString &s )
{
    // Find the properties of the named mailbox
    const MailboxProperties* properties( mailboxProperties(s) );
    if (properties)
        return properties->name;

    // Return the untranslated name
    return s;
}

QString MailboxList::mailboxTrHeader( const QString &s )
{
    // Find the properties of the named mailbox
    const MailboxProperties* properties( mailboxProperties(s) );
    if (properties)
        return properties->header;

    // No header is defined, indicate with an empty string
    return QString("");
}

QIcon MailboxList::mailboxIcon( const QString &s )
{
    QMap<QString,QIcon> cache;
    QMap<QString,QIcon>::const_iterator it = cache.find(s);
    if (it != cache.end())
        return (*it);

    // Find the properties of the named mailbox
    const MailboxProperties* properties( mailboxProperties(s) );
    if (properties) {
        QIcon icon(properties->icon);
        cache[s] = icon;
        return icon;
    }

    return QIcon(":icon/folder");
}

//===========================================================================

/*  Email Folder List */
EmailFolderList::EmailFolderList(QString mailbox, QObject *parent)
    : QObject(parent),
    mMailbox(mailbox),
    mFolder(mailbox),
    mParentFolderKey(QMailMessageKey::ParentFolderId,mFolder.id())
{
}

EmailFolderList::~EmailFolderList()
{
}

void EmailFolderList::openMailbox()
{
    QString mbox = MailboxList::mailboxTrName(mMailbox);

    QMailFolderKey key(QMailFolderKey::Name,mMailbox);
    key &= QMailFolderKey(QMailFolderKey::ParentId,QMailId());
    
    QMailIdList folderIdList = QMailStore::instance()->queryFolders(key);
    
    if(folderIdList.isEmpty()) //create folder
    {
        QMailFolder newFolder(mMailbox);
        if(!QMailStore::instance()->addFolder(&newFolder))
			qWarning() << "Failed to add folder " << mMailbox;
        mFolder = newFolder;
    }
    else //load folder
    {
		QMailId folderId = folderIdList.first();
		mFolder = QMailFolder(folderId);
    }

    //set the folder key   
    mParentFolderKey = QMailMessageKey(QMailMessageKey::ParentFolderId,mFolder.id());
}

bool EmailFolderList::addMail(QMailMessage &m)
{
    if(m.id().isValid()){
        m.setParentFolderId(mFolder.id());
        if(!QMailStore::instance()->updateMessage(&m))
            return false;
        emit mailUpdated( m.id(), mailbox() );
    }
    else 
    {
        m.setParentFolderId(mFolder.id());
        if(!QMailStore::instance()->addMessage(&m))
            return false;
        emit mailAdded(m.id(),mailbox());
    }

    return true;
}

bool EmailFolderList::removeMail(QMailId id)
{
    if(!QMailStore::instance()->removeMessage(id))
        return false;

    emit mailRemoved(id, mailbox() );

    return true;
}

bool EmailFolderList::empty(int type)
{
    QMailMessageKey typeKey(QMailMessageKey::Type,type,QMailMessageKey::Contains);
    
    QMailIdList deleteList = QMailStore::instance()->queryMessages(mParentFolderKey & typeKey);

    foreach(QMailId id, deleteList)
        QMailStore::instance()->removeMessage(id);

    return true;
}

QMailIdList EmailFolderList::messages(unsigned int messageType, const SortOrder& order ) const
{
    QMailMessageKey queryKey = mParentFolderKey;

    if(messageType != QMailMessage::AnyType)
        queryKey &= QMailMessageKey(QMailMessageKey::Type,messageType,QMailMessageKey::Contains);
        
    if(order != Submission )
    {
        Qt::SortOrder srtOrder;

        if(order == DescendingDate)
          srtOrder = Qt::DescendingOrder;
        else
          srtOrder = Qt::AscendingOrder;

        QMailMessageSortKey sortKey(QMailMessageSortKey::TimeStamp,srtOrder);

        return QMailStore::instance()->queryMessages(queryKey, sortKey);
    }
    else
        return QMailStore::instance()->queryMessages(queryKey);

}

QMailIdList EmailFolderList::messages(const QMailMessage::Status& status, 
                                      bool contains,
                                      unsigned int messageType, 
                                      const SortOrder& order) const
{

    QMailMessageKey queryKey = mParentFolderKey;

    int flags = status;
    QMailMessageKey statusKey(QMailMessageKey::Flags,flags,QMailMessageKey::Contains);

    if(contains)
        queryKey &=  statusKey;
    else
        queryKey &= ~statusKey;

    if(messageType != QMailMessage::AnyType)
        queryKey &= QMailMessageKey(QMailMessageKey::Type,messageType,QMailMessageKey::Contains);
        
    if(order != Submission )
    {
        Qt::SortOrder srtOrder;

        if(order == DescendingDate)
          srtOrder = Qt::DescendingOrder;
        else
          srtOrder = Qt::AscendingOrder;

        QMailMessageSortKey sortKey(QMailMessageSortKey::TimeStamp,srtOrder);

        return QMailStore::instance()->queryMessages(queryKey, sortKey);
    }
    else
        return QMailStore::instance()->queryMessages(queryKey);


}

QMailIdList EmailFolderList::messagesFromMailbox(const QString& mailbox, 
                                                 unsigned int messageType, 
                                                 const SortOrder& order ) const
{
    QMailMessageKey queryKey = mParentFolderKey & QMailMessageKey(QMailMessageKey::FromMailbox,mailbox);

    if(messageType != QMailMessage::AnyType)
        queryKey &= QMailMessageKey(QMailMessageKey::Type, messageType, QMailMessageKey::Contains);

    if(order != Submission )
    {
        Qt::SortOrder srtOrder;

        if(order == DescendingDate)
          srtOrder = Qt::DescendingOrder;
        else
          srtOrder = Qt::AscendingOrder;

        QMailMessageSortKey sortKey(QMailMessageSortKey::TimeStamp,srtOrder);

        return QMailStore::instance()->queryMessages(queryKey, sortKey);
    }
    else
        return QMailStore::instance()->queryMessages(queryKey);
}

QMailIdList EmailFolderList::messagesFromAccount(const QString& account,
                                                 unsigned int messageType,
                                                 const SortOrder& order) const
{
    QMailMessageKey queryKey = mParentFolderKey & QMailMessageKey(QMailMessageKey::FromAccount,account);

    if(messageType != QMailMessage::AnyType)
        queryKey &= QMailMessageKey(QMailMessageKey::Type, messageType, QMailMessageKey::Contains);

    if(order != Submission )
    {
        Qt::SortOrder srtOrder;

        if(order == DescendingDate)
          srtOrder = Qt::DescendingOrder;
        else
          srtOrder = Qt::AscendingOrder;

        QMailMessageSortKey sortKey(QMailMessageSortKey::TimeStamp,srtOrder);

        return QMailStore::instance()->queryMessages(queryKey, sortKey);
    }
    else
        return QMailStore::instance()->queryMessages(queryKey);
}

bool EmailFolderList::contains(const QMailId& id) const
{
    QMailMessageKey idKey(QMailMessageKey::Id,id);
    int count = QMailStore::instance()->countMessages(idKey & mParentFolderKey );
    return count != 0;
}

uint EmailFolderList::mailCount( MailType status, 
				 int type, 
				 AccountList *accountList )
{
    //bool isAnyType = (QMailMessage::AnyType == type);
    QMap<MailAccount*,int> accountMap;
    if ( accountList ) {
        QListIterator<MailAccount*> it2 = accountList->accountIterator();
        while (it2.hasNext()) {
            accountMap.insert( it2.next(), 0 );
        }
    }

    uint count = 0;
    switch ( status )
    {
        case All:
            {
                QMailMessageKey typeKey(QMailMessageKey::Type,type,QMailMessageKey::Contains);

                QMailMessageKey allKey = mParentFolderKey & typeKey;

                count = QMailStore::instance()->countMessages(allKey);

                if(accountList)
                {
                    QListIterator<MailAccount*> it2 = accountList->accountIterator();
                    while (it2.hasNext()) {
                        MailAccount *account = it2.next();
                        QMailMessageKey accountKey(QMailMessageKey::FromAccount,account->id());
                        accountMap[account] = QMailStore::instance()->countMessages( allKey & accountKey); 
                    }
                }
                break;
            }

        case New:
            {
                QMailMessageKey typeKey(QMailMessageKey::Type,type,QMailMessageKey::Contains);
                QMailMessageKey readKey(QMailMessageKey::Flags,QMailMessage::Read,QMailMessageKey::Contains);

                QMailMessageKey newKey = mParentFolderKey & typeKey & ~readKey;

                count = QMailStore::instance()->countMessages(newKey);

                if ( accountList ) {
                    QListIterator<MailAccount*> it2 = accountList->accountIterator();
                    while (it2.hasNext()) {
                        MailAccount *account = it2.next();
                        QMailMessageKey accountKey(QMailMessageKey::FromAccount,account->id());
                        accountMap[account] = QMailStore::instance()->countMessages(newKey & accountKey);
                    }
                }
                break;
            }

        case Unsent:
            {
                QMailMessageKey typeKey(QMailMessageKey::Type,type,QMailMessageKey::Contains);
                QMailMessageKey sentKey(QMailMessageKey::Flags,QMailMessage::Sent,QMailMessageKey::Contains);

                QMailMessageKey unsentKey = mParentFolderKey & typeKey & ~sentKey;

                //dunno about this hasRecipients

                //if ( !(mail->status() & QMailMessage::Sent) 
                //     && mail->hasRecipients() )
                //   count++;

                count = QMailStore::instance()->countMessages(unsentKey);

                break;
            }

        case Unfinished:
            {
                // Currently unhandled!
                break;
            }
    }

    if ( accountList && status == All ) {
        QListIterator<MailAccount*> it2 = accountList->accountIterator();
        while (it2.hasNext()) {
            MailAccount *account = it2.next();
            account->setCount( accountMap[account] );
        }
    } else if ( accountList && status == New ) {
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
    emit externalEdit( mailbox() );
}

bool EmailFolderList::moveMail(const QMailId& id, EmailFolderList& dest)
{

    //move the context information across

    QMailFolder otherFolder = dest.mFolder;
    QMailMessage m(id,QMailMessage::Header);

    if(m.parentFolderId() != mFolder.id())
    {
        qWarning() << "Cannot move mail that does not exist in folder " << mMailbox;
        return false;
    }

    m.setParentFolderId(otherFolder.id());
        
    if(!QMailStore::instance()->updateMessage(&m))
        return false;

    emit mailMoved(id,mailbox(),dest.mailbox());

    return true;
}

bool EmailFolderList::copyMail(const QMailId& id, EmailFolderList& dest)
{
//move the context information across

    
    QMailFolder otherFolder = dest.mFolder;
    QMailMessage m(id,QMailMessage::HeaderAndBody);

    if(m.parentFolderId() != mFolder.id())
    {
        qWarning() << "Cannot move mail that does not exist in folder " << mMailbox;
        return false;
    }

    m.setId(QMailId()); //reset id
    m.setParentFolderId(otherFolder.id());

    if(!QMailStore::instance()->addMessage(&m))

    emit mailAdded(m.id(),dest.mailbox());

    return true;
}

QString EmailFolderList::mailbox() const
{
    return mMailbox;
}

QMailFolder EmailFolderList::mailFolder() const
{
    return mFolder;
}

bool EmailFolderList::moveMailList(const QMailIdList& list, EmailFolderList& dest)
{
    // Check that these messages belong to our folder
    QMailIdList folderIds(QMailStore::instance()->parentFolderIds(list));

    if ((folderIds.count() != 1) || (folderIds.first() != mFolder.id())) {
        qWarning() << "Cannot move mail that does not exist in folder " << mMailbox;
        return false;
    }

    if (!QMailStore::instance()->updateParentFolderIds(list, dest.mFolder.id()))
        return false;

    emit mailMoved(list, mailbox(), dest.mailbox());

    return true;
}

