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

#ifndef __QMAILSTORE_H
#define __QMAILSTORE_H

#include <QMailId>
#include <QSharedData>
#include <qtopiaglobal.h>

class QMailFolder;
class QMailFolderKey;
class QMailFolderSortKey;
class QMailMessage;
class QMailMessageKey;
class QMailMessageSortKey;
class QMailStorePrivate;
class QMailStore;

typedef QList<QMailFolder> QMailFolderList;
typedef QList<QMailMessage> QMailMessageList;
typedef QList<QMailId> QMailIdList;

#ifdef QMAILSTOREINSTANCE_DEFINED_HERE
static QMailStore* QMailStoreInstance();
#endif

class QTOPIAMAIL_EXPORT QMailStore
{
public:
    virtual ~QMailStore();

    bool addFolder(QMailFolder* f);
    bool addMessage(QMailMessage* m);

    bool removeFolder(const QMailId& id);
    bool removeMessage(const QMailId& id);

    bool updateFolder(QMailFolder* f);
    bool updateMessage(QMailMessage* m); 

    int countFolders(const QMailFolderKey& k) const;
	int countFolders() const;
    int countMessages(const QMailMessageKey& k) const;
	int countMessages() const;

	QMailIdList queryFolders() const;
	QMailIdList queryFolders(const QMailFolderSortKey& sortKey) const;
    QMailIdList queryFolders(const QMailFolderKey& k) const;
	QMailIdList queryFolders(const QMailFolderKey& key, 
			          const QMailFolderSortKey& sortKey) const;
	QMailIdList queryMessages() const;
	QMailIdList queryMessages(const QMailMessageSortKey& sortKey) const;
    QMailIdList queryMessages(const QMailMessageKey& k) const;
	QMailIdList queryMessages(const QMailMessageKey& key, 
			          const QMailMessageSortKey& sortKey) const;

	QMailFolder folder(const QMailId& id) const;
	QMailMessage message(const QMailId& id) const;
	QMailMessage messageHeader(const QMailId& id) const;

    static QMailStore* instance();
#ifdef QMAILSTOREINSTANCE_DEFINED_HERE	
    friend QMailStore* QMailStoreInstance();    
#endif

private:
    QMailStore();
				
    // Here follow some nasty functions, required to speed up specific qtmail tasks:

    friend class EmailFolderList;
    friend class EmailClient;

    QMailIdList parentFolderIds(const QMailIdList& list) const;
    bool updateParentFolderIds(const QMailIdList& list, const QMailId& id);

    struct DeletionProperties {
        QMailId id;
        QString serverUid;
        QString fromAccount;
        QString fromMailbox;
    };

    QList<DeletionProperties> deletionProperties(const QMailIdList& list) const;

private:
    QSharedDataPointer<QMailStorePrivate> d;

};



#endif //QMAILSTORE_H

