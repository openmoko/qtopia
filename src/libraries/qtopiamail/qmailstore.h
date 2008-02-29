/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QMAILSTORE_H
#define __QMAILSTORE_H

#include <QMailId>
#include <qtopiaglobal.h>
#include "qmailfolderkey.h"
#include "qmailfoldersortkey.h"
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"

class QMailFolder;
class QMailMessage;
class QMailStorePrivate;
class QMailStore;



typedef QList<QMailFolder> QMailFolderList;
typedef QList<QMailMessage> QMailMessageList;
typedef QList<QMailId> QMailIdList;

#ifdef QMAILSTOREINSTANCE_DEFINED_HERE
static QMailStore* QMailStoreInstance();
#endif

class QTOPIAMAIL_EXPORT QMailStore : public QObject
{
    Q_OBJECT 

public:
    enum ReturnOption
    {
        ReturnAll,
        ReturnDistinct
    };

public:
    virtual ~QMailStore();

    bool addFolder(QMailFolder* f);
    bool addMessage(QMailMessage* m);

    bool removeFolder(const QMailId& id);
    bool removeMessage(const QMailId& id);

    bool updateFolder(QMailFolder* f);
    bool updateMessage(QMailMessage* m);
    bool updateMessages(const QMailMessageKey& key,
                        const QMailMessageKey::Properties& properties,
                        const QMailMessage& data);
    bool updateMessages(const QMailMessageKey& key,
                        const QMailMessage::Status status,
                        bool set);

    int countFolders(const QMailFolderKey& key = QMailFolderKey()) const;
    int countMessages(const QMailMessageKey& key = QMailMessageKey()) const;

    int sizeOfMessages(const QMailMessageKey& key = QMailMessageKey()) const;

    QMailIdList queryFolders(const QMailFolderKey& key = QMailFolderKey(),
                             const QMailFolderSortKey& sortKey = QMailFolderSortKey()) const;
    QMailIdList queryMessages(const QMailMessageKey& key = QMailMessageKey(),
                              const QMailMessageSortKey& sortKey = QMailMessageSortKey()) const;

    QMailFolder folder(const QMailId& id) const;

    QMailMessage message(const QMailId& id) const;
    QMailMessage message(const QString& uid, const QString& account) const;

    QMailMessage messageHeader(const QMailId& id) const;
    QMailMessage messageHeader(const QString& uid, const QString& account) const;
    QMailMessageList messageHeaders(const QMailMessageKey& key, 
                                    const QMailMessageKey::Properties& properties,
                                    const ReturnOption& option = ReturnAll) const;

    static QMailStore* instance();
#ifdef QMAILSTOREINSTANCE_DEFINED_HERE	
    friend QMailStore* QMailStoreInstance();    
#endif

signals:
    void messagesAdded(const QMailIdList& ids);
    void messagesRemoved(const QMailIdList& ids);
    void messagesUpdated(const QMailIdList& ids);
    void foldersAdded(const QMailIdList& ids);
    void foldersRemoved(const QMailIdList& ids);
    void foldersUpdated(const QMailIdList& ids);

private:
    QMailStore();
				
    friend class EmailFolderList;
    friend class EmailClient;

private:
    QMailStorePrivate* d;

};



#endif //QMAILSTORE_H

