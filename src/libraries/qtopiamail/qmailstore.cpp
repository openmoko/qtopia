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

// Needed to give friend access to the function defined by Q_GLOBAL_STATIC
#define QMAILSTOREINSTANCE_DEFINED_HERE
#include "qmailstore.h"
#include "qmailstore_p.h"
#include "qmailfolder.h"
#include "qmailfolderkey.h"
#include "qmailfoldersortkey.h"
#include "qmailmessage.h"
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"
#include "qmailid.h"
#include "qmailtimestamp.h"
#include <qtopiasql.h>
#include <QDebug>
#include <QContent>
#include <qtopialog.h>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>

#include <algorithm>

/*!
    \class QMailStore
    \mainclass
    \preliminary
    \brief The QMailStore class represents the main interface for storage and retrieval
    of messages and folders on the message store.
     
    \ingroup messaginglibrary

    The QMailStore class is accessed through a singleton interface and provides functions 
    for adding, updating and deleting of QMailFolders and QMailMessages on the message store. 
    
    QMailStore also provides functions for querying and counting of QMailFolders and QMailMessages
    when used in conjunction with QMailMessageKey and QMailFolderKey classes.

    \sa QMailMessage, QMailFolder, QMailMessageKey, QMailFolderKey
*/

/*!
    Constructs a new QMailStore object and opens the message store database.
*/


QMailStore::QMailStore()
{
    //create the db and sql interfaces
    d = new QMailStorePrivate();
    d->database = QtopiaSql::instance()->applicationSpecificDatabase("qtmail");
}

/*!
    Destroys this QMailStore object.
*/

QMailStore::~QMailStore()
{
}

/*!
    Adds a new QMailFolder object \a a into the message store, performing
    respective integrity checks. Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStore::addFolder(QMailFolder* a)
{    
	d->database.transaction();

    //check that the parent folder actually exists

    if(a->parentId().isValid() && !d->folderExists(a->parentId().toULongLong()))
	{
		d->database.rollback();
		qLog(Messaging) << "Parent folder does not exist";
		return false;
	}

	//insert the qmailfolder

	QSqlQuery query = d->prepare("INSERT INTO mailfolders (name,parentid) VALUES (?,?)");

	if(query.lastError().type() != QSqlError::NoError)
		return false;
	
	query.addBindValue(a->name());
	query.addBindValue(a->parentId().toULongLong());
	
	if(!d->execute(query))
		return false;

	//set the insert id

	quint64 newId = query.lastInsertId().toULongLong();
	QMailId insertId(newId);	
	a->setId(insertId);

	if(!d->database.commit())
	{
		a->setId(QMailId());
		return false;
	}
	//_folderSync->notifyAdded(a.parentId(),a.id());
	return true;
}

/*!
    Adds a new QMailMessage object \a m into the message store, performing
    respective integrity checks. Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStore::addMessage(QMailMessage* m)
{
	if(!m->parentFolderId().isValid())
		return false;

	d->database.transaction();

	QString mailfile;
	if(!d->mailBodyStore.insert(*m,&mailfile))
	{
		qLog(Messaging) << "Could not store mail body";
		return false;
	}


	QString sql = "INSERT INTO mailmessages (type, parentfolderid, sender, \
		   recipients, subject, stamp, status, fromaccount, \
		   frommailbox, mailfile, serveruid, size) \
		   VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
	
	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
	{
		//delete the temp mail body
		if(!d->mailBodyStore.remove(mailfile))
			qLog(Messaging) << "Could not remove temp mail body" << mailfile;
		return false;
	}

	query.addBindValue(static_cast<int>(m->messageType()));
	query.addBindValue(m->parentFolderId().toULongLong());

    // Ensure that any phone numbers are added in minimal form
    QMailAddress from(m->from());
    QString fromText(from.isPhoneNumber() ? from.minimalPhoneNumber() : from.toString());
    query.addBindValue(fromText);

    QStringList recipients;
    foreach (const QMailAddress& address, m->to()) {
        recipients.append(address.isPhoneNumber() ? address.minimalPhoneNumber() : address.toString());
    }
    query.addBindValue(recipients.join(","));

	query.addBindValue(m->subject());
	query.addBindValue(QMailTimeStamp(m->date()).toLocalTime());
	query.addBindValue(static_cast<int>(m->status()));
	query.addBindValue(m->fromAccount());
	query.addBindValue(m->fromMailbox());
	query.addBindValue(mailfile);
	query.addBindValue(m->serverUid());
	query.addBindValue(m->size());
	
	if(!d->execute(query))
	{
		if(!d->mailBodyStore.remove(mailfile))
			qLog(Messaging) << "Could not remove temp mail body" << mailfile;
		return false;
	}

	//set the insert id

	quint64 newId = query.lastInsertId().toULongLong();
	QMailId insertId(newId);	
	m->setId(insertId);

	if(!d->database.commit())
	{
		m->setId(QMailId());
		if(!d->mailBodyStore.remove(mailfile))
			qLog(Messaging) << "Could not remove temp mail body" << mailfile;

		return false;
	}

	//fire sync signals

	return true;
}

/*!
    Removes a QMailFolder with QMailId \a id from the message store. This action also
    removes the sub-folder and messages of the folder. Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStore::removeFolder(const QMailId& id)
{
	d->database.transaction();
	d->deleteFolder(id);
	if(!d->database.commit())
		return false;
	//_folderSync->notifyRemoved(f.parentId(),f.id());
	return true;
}

/*!
    Removes a QMailMessage with QMailId \a id from the message store. Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStore::removeMessage(const QMailId& id)
{
	//get the mailfile

    d->database.transaction();
	
	QSqlQuery query = d->prepare("SELECT mailfile FROM mailmessages WHERE id = ?");

	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(id.toULongLong());

	if(!d->execute(query))
		return false;

	query.first();

	QString mailfile = query.value(0).toString();

	query = d->prepare("DELETE FROM mailmessages WHERE id = ?");

	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(id.toULongLong());

	if(!d->execute(query))
		return false;

	if(!d->database.commit())
		return false;
	
	//delete the mail body

	if(!d->mailBodyStore.remove(mailfile))
		qLog(Messaging) << "Could not remove the mail body " << mailfile;

    if(d->headerCache.contains(id))
        d->headerCache.remove(id);

	return true;

}
/*!
    Updates the existing QMailFolder \a f on the message store.
    Returns \c true if the operation completed successfully, 
    \c false otherwise. 
*/

bool QMailStore::updateFolder(QMailFolder* f)
{

    //check for a valid folder

    if(!f->id().isValid())
    {
        qLog(Messaging) << "Invalid folder to update";
        return false;
    }

    //check for self reference

    if(f->parentId().isValid() && f->parentId() == f->id())
	{
		qLog(Messaging) << "A folder cannot be a child to itself";
		return false;
	}

	d->database.transaction();

	//check that the parentfolder exists

	if(f->parentId().isValid() && !d->folderExists(f->parentId().toULongLong()))
	{
		d->database.rollback();
		qLog(Messaging) << "Parent folder does not exist";
		return false;
	}

	QSqlQuery query = d->prepare("UPDATE mailfolders SET name=?, parentid=? WHERE id =?");

	if(query.lastError().type() != QSqlError::NoError)
		return false;
	
	query.addBindValue(f->name());
	query.addBindValue(f->parentId().toULongLong());
    query.addBindValue(f->id().toULongLong());

	if(!d->execute(query))
		return false;
    
	if(!d->database.commit())
        return false;

	//fire sync messages

    return true;
}

/*!
    Updates the existing QMailMessage \a m on the message store.
    Returns \c true if the operation completed successfully, or \c false otherwise. 
*/

bool QMailStore::updateMessage(QMailMessage* m)
{
    if(!m->id().isValid())
        return false;

    if(!m->uncommittedChanges() && !m->uncommittedMetadataChanges())
        return true;

    QString sql = "\
UPDATE mailmessages SET type=?, \
parentfolderid=?, \
sender=?, \
recipients=?, \
subject=?, \
stamp=?, \
status=?, \
fromaccount=?, \
frommailbox=?, \
serveruid=?, \
size=? \
WHERE id = ?";

    QSqlQuery query = d->prepare(sql);

    if(query.lastError().type() != QSqlError::NoError)
        return false;

    query.addBindValue(static_cast<unsigned int>(m->messageType()));
    query.addBindValue(m->parentFolderId().toULongLong());

    // Ensure that any phone numbers are added in minimal form
    QMailAddress from(m->from());
    QString fromText(from.isPhoneNumber() ? from.minimalPhoneNumber() : from.toString());
    query.addBindValue(fromText);

    QStringList recipients;
    foreach (const QMailAddress& address, m->to()) {
        recipients.append(address.isPhoneNumber() ? address.minimalPhoneNumber() : address.toString());
    }
    query.addBindValue(recipients.join(","));

    query.addBindValue(m->subject());
    query.addBindValue(QMailTimeStamp(m->date()).toLocalTime());
    query.addBindValue(static_cast<unsigned int>(m->status()));
    query.addBindValue(m->fromAccount());
    query.addBindValue(m->fromMailbox());
    query.addBindValue(m->serverUid());
    query.addBindValue(m->size());
    query.addBindValue(m->id().toULongLong());

    if(!d->execute(query))
        return false;

    //update the mail body
    //get the mailfile reference

    //if(option == RetrieveHeadersAndBody)
    if(m->uncommittedChanges())
    {
        query = d->prepare("SELECT mailfile FROM mailmessages WHERE id=" + QString::number(m->id().toULongLong()));
		
        if(!d->execute(query))
            return false;

        query.first();

        QString mailfile = query.value(0).toString();

        if(!d->mailBodyStore.update(mailfile,*m))
        {
            qLog(Messaging) << "Could not update mail body " << mailfile;
            return false;
        }
    }

    // The message is now up-to-date with data store
    m->changesCommitted();

    if(d->headerCache.contains(m->id()))
        d->headerCache.remove(m->id());


    //fire sync functions
    return true;
}

/*!
    Returns the count of the number of folders which pass the 
    filtering criteria defined in QMailFolderKey \a key.        
*/

int QMailStore::countFolders(const QMailFolderKey& key) const
{
	QString sql = "SELECT COUNT(*) FROM mailfolders WHERE ";
  	sql += d->buildWhereClause(key);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() == QSqlError::NoError)
	{
		d->bindWhereData(key,query);
		if(d->execute(query) && query.next())
			return query.value(0).toInt();
	}
    
	return 0;
}

/*!
    Returns the count of all the folders in the message store. 
*/

int QMailStore::countFolders() const
{
	QString sql = "SELECT COUNT(*) FROM mailfolders";

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() == QSqlError::NoError)
	{
		if(d->execute(query) && query.next())
			return query.value(0).toInt();
	}
    
	return 0;
}

/*!
    Returns the count of the number of messages which pass the 
    filtering criteria defined in QMailMessageKey \a key.        
*/

int QMailStore::countMessages(const QMailMessageKey& key) const
{
    if (key.isEmpty())
        return 0;

	QString sql = "SELECT COUNT(*) FROM mailmessages WHERE ";
  	sql += d->buildWhereClause(key);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() == QSqlError::NoError)
	{
		d->bindWhereData(key,query);
		if(d->execute(query) && query.next())
			return query.value(0).toInt();
	}
    
	return 0;

}

/*!
    Returns the count of all the messages in the message store 
*/

int QMailStore::countMessages() const
{
	QString sql = "SELECT COUNT(*) FROM mailmessages";
	
	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() == QSqlError::NoError)
	{
		if(d->execute(query) && query.next())
			return query.value(0).toInt();
	}
    
	return 0;

}



/*!
    Returns the QMailId's of all folders in the
    message store	
*/

QMailIdList QMailStore::queryFolders() const
{
	QString sql = "SELECT id FROM mailfolders";

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}

/*!
    Returns the QMailId's of all folders in the
    message store sorted by the QMailFolderSortKey \a sortKey.	
*/

QMailIdList QMailStore::queryFolders(const QMailFolderSortKey& sortKey) const
{
	QString sql = "SELECT id FROM mailfolders ";
	sql += d->buildOrderClause(sortKey);

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}



/*!
    Returns a list of QMailId's which pass the 
    query criteria defined in QMailFolderKey \a key.        
*/

QMailIdList QMailStore::queryFolders(const QMailFolderKey& key) const
{
	QString sql = "SELECT id FROM mailfolders WHERE ";
	sql += d->buildWhereClause(key);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	d->bindWhereData(key,query);

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}

/*!
    Returns a list of QMailId's which pass the 
    query criteria defined in QMailFolderKey \a key and the
	QMailFolderSortKey \a sortKey.        
*/

QMailIdList QMailStore::queryFolders(const QMailFolderKey& key,
		                      const QMailFolderSortKey& sortKey) const
{
	QString sql = "SELECT id FROM mailfolders WHERE ";
	sql += d->buildWhereClause(key);
	sql += d->buildOrderClause(sortKey);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	d->bindWhereData(key,query);

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}

/*!
    Returns the QMailId's of all messages in the
    message store	
*/

QMailIdList QMailStore::queryMessages() const
{
	QString sql = "SELECT id FROM mailmessages";

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}

/*!
    Returns the QMailId's of all messages in the
    message store sorted by the QMailMessageSortKey \a sortKey.	
*/

QMailIdList QMailStore::queryMessages(const QMailMessageSortKey& sortKey) const
{
	QString sql = "SELECT id FROM mailmessages ";
	sql += d->buildOrderClause(sortKey);

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailFolder query failed";

	if(!d->execute(query))
		qLog(Messaging) << "MailFolder query failed";

	QMailIdList results;

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}


/*!
    Returns a list of QMailId's which pass the 
    query criteria defined in QMailMessageKey \a key. 
 */

QMailIdList QMailStore::queryMessages(const QMailMessageKey& key) const
{
	QMailIdList results;
    if (key.isEmpty())
        return results;

	QString sql = "SELECT id FROM mailmessages WHERE ";
	sql += d->buildWhereClause(key);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailMessage id query failed";

	d->bindWhereData(key,query);

	if(!d->execute(query))
		qLog(Messaging) << "MailMessage id query failed";

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}

/*!
    Returns a list of QMailId's which pass the 
    query criteria defined in QMailFolderKey \a key and the
	QMailFolderSortKey \a sortKey.        
*/

QMailIdList QMailStore::queryMessages(const QMailMessageKey& key,
		                      const QMailMessageSortKey& sortKey) const
{
	QMailIdList results;
    if (key.isEmpty())
        return results;

	QString sql = "SELECT id FROM mailmessages WHERE ";
	sql += d->buildWhereClause(key);
	sql += d->buildOrderClause(sortKey);	

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailMessage query failed";

	d->bindWhereData(key,query);

	if(!d->execute(query))
		qLog(Messaging) << "MailMessage query  failed";

	while(query.next())
		results.append(QMailId(query.value(0).toULongLong()));

	return results;	
}



/*!
   Returns the QMailFolder defined by a QMailId \a id from 
   the message store.
*/

QMailFolder QMailStore::folder(const QMailId& id) const
{
	QString sql = "SELECT * FROM mailfolders WHERE id=?";

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailMessage id query failed";

	query.addBindValue(id.toULongLong());

	if(!d->execute(query))
		qLog(Messaging) << "MailMessage id query failed";

	if(!query.first())
		return QMailFolder();
	else
		return d->buildQMailFolder(query.record());
}

/*!
   Returns the QMailMessage defined by a QMailId \a id from 
   the message store.
*/


QMailMessage QMailStore::message(const QMailId& id) const
{
	QString cols = "id, type, parentfolderid, sender, recipients, \
						subject, stamp, status, fromaccount, frommailbox,\
					    mailfile, serveruid, size";

	QString sql = "SELECT " + cols + " FROM mailmessages WHERE id=?";
	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailMessage query failed";

	query.addBindValue(id.toULongLong());

	if(!d->execute(query))
		qLog(Messaging) << "MailMessage query failed";

	if(!query.first())
		return QMailMessage();
	else
		return d->buildQMailMessage(query.record());
}

/*!
   Returns the QMailMessage header defined by a QMailId \a id from 
   the message store. 
*/

QMailMessage QMailStore::messageHeader(const QMailId& id) const
{

    QMailMessage cachedHeader = d->headerCache.lookup(id);

    if(cachedHeader.id().isValid())
        return cachedHeader;

	QString cols = "id, type, parentfolderid, sender, recipients, \
						subject, stamp, status, fromaccount, frommailbox,\
					    serveruid, size";

    //retrieve the batch of records before and after this record
    //and cache them. 

	QString sql = "SELECT " + cols + " FROM mailmessages WHERE id IN (";

    int max = QMailStorePrivate::lookAhead() * 2;

    for(int i =0; i < max; ++i)
    {
        sql += '?';
        if(i < max-1)
            sql += ',';
    }
    sql += ")";

	QSqlQuery query = d->prepare(sql);

	if(query.lastError().type() != QSqlError::NoError)
		qLog(Messaging) << "MailMessage query failed";

    for(int i = -QMailStorePrivate::lookAhead(); i < QMailStorePrivate::lookAhead(); ++i)
        query.addBindValue(id.toULongLong()+i);

	if(!d->execute(query))
		qLog(Messaging) << "MailMessage query failed";

    while(query.next())
    {
		QMailMessage newMessage = d->buildQMailMessage(query.record());
        if(newMessage.id().isValid())
            d->headerCache.insert(newMessage);
    }

    return d->headerCache.lookup(id);
}


// We will impose an upper limit on how many values we will include in a 
// set constraint - otherwise, we may hit some buffer limit that we didn't
// realise existed...
static const int maxSetSize = 50;

static QPair<QString, QPair<QMailIdList::const_iterator, QMailIdList::const_iterator> > describeSet(const QMailIdList& list, int count = -1, int offset = 0)
{
    int n = qMax(qMin(count, (list.count() - offset)), 0);

    QString result;
    QMailIdList::const_iterator begin = list.begin() + offset, end = begin + n;

    if (n > 0) {
        result.append(" (?");
        for (--n; n > 0; --n)
            result.append(",?");
        result.append(')');
    }

    return qMakePair(result, qMakePair(begin, end));
}

/*! \internal */
QMailIdList QMailStore::parentFolderIds(const QMailIdList& list) const
{
    static const QString sql = 
"SELECT DISTINCT parentfolderid "
"FROM mailmessages "
"WHERE id IN ";

    QMailIdList result;

    // Ensure that no single query exceeds the set size limit
    int queryCount = list.count() / maxSetSize;
    if (list.count() != queryCount)
        ++queryCount;

    // We should use evenly sized query sets
    int setSize = list.count() / queryCount;

    int offset = 0;
    while (offset < list.count()) {
        QPair<QString, QPair<QMailIdList::const_iterator, QMailIdList::const_iterator> > listProperties(describeSet(list, setSize, offset));
        offset += setSize;

        QSqlQuery query = d->prepare(sql + listProperties.first);
        if(query.lastError().type() != QSqlError::NoError)
            qLog(Messaging) << "MailMessage parentFolderIds query prepare failed";

        QMailIdList::const_iterator it = listProperties.second.first;
        for ( ; it != listProperties.second.second; ++it)
            query.addBindValue((*it).toULongLong());

        if (!d->execute(query))
            qLog(Messaging) << "MailMessage parentFolderIds query execute failed";

        while (query.next())
            result.append(QMailId(query.value(0).toULongLong()));
    }

    // Ensure that we return a unique set
    qSort(result.begin(), result.end());

    QMailIdList::iterator uniqueEnd = std::unique(result.begin(), result.end());
    result.erase(uniqueEnd, result.end());

	return result;	
}

/*! \internal */
bool QMailStore::updateParentFolderIds(const QMailIdList& list, const QMailId& id)
{
    static const QString sql =
"UPDATE mailmessages "
"SET parentfolderid = ? "
"WHERE id IN ";

	d->database.transaction();

    // Ensure that no single query exceeds the set size limit
    int queryCount = list.count() / maxSetSize;
    if (list.count() != queryCount)
        ++queryCount;

    // We should use evenly sized query sets
    int setSize = list.count() / queryCount;

    int offset = 0;
    while (offset < list.count()) {
        QPair<QString, QPair<QMailIdList::const_iterator, QMailIdList::const_iterator> > listProperties(describeSet(list, setSize, offset));
        offset += setSize;

        QSqlQuery query = d->prepare(sql + listProperties.first);
        if(query.lastError().type() != QSqlError::NoError) {
            qLog(Messaging) << "MailMessage updateParentFolderIds query prepare failed";
            return false;
        }
        
        query.addBindValue(id.toULongLong());

        QMailIdList::const_iterator it = listProperties.second.first;
        for ( ; it != listProperties.second.second; ++it)
            query.addBindValue((*it).toULongLong());

        if (!d->execute(query)) {
            qLog(Messaging) << "MailMessage updateParentFolderIds query execute failed";
            return false;
        }
    }
    
	if (!d->database.commit()) {
		qLog(Messaging) << "MailMessage updateParentFolderIds query commit failed";
		return false;
    } else {
        foreach (const QMailId& messageId, list) {
            // Update any copies in the cache
            if (d->headerCache.contains(messageId)) {
                QMailMessage cachedHeader(d->headerCache.lookup(messageId));

                // Change to the new folder
                cachedHeader.setParentFolderId(id);
                d->headerCache.insert(cachedHeader);
            }
        }
    }

    return true;
}

/*! \internal */
QList<QMailStore::DeletionProperties> QMailStore::deletionProperties(const QMailIdList& list) const
{
    static const QString sql = 
"SELECT id, serveruid, fromaccount, frommailbox "
"FROM mailmessages "
"WHERE id IN ";

    QList<DeletionProperties> result;

    // Ensure that no single query exceeds the set size limit
    int queryCount = list.count() / maxSetSize;
    if (list.count() != queryCount)
        ++queryCount;

    // We should use evenly sized query sets
    int setSize = list.count() / queryCount;

    int offset = 0;
    while (offset < list.count()) {
        QPair<QString, QPair<QMailIdList::const_iterator, QMailIdList::const_iterator> > listProperties(describeSet(list, setSize, offset));
        offset += setSize;

        QSqlQuery query = d->prepare(sql + listProperties.first);
        if(query.lastError().type() != QSqlError::NoError)
            qLog(Messaging) << "MailMessage deletionProperties query prepare failed";

        QMailIdList::const_iterator it = listProperties.second.first;
        for ( ; it != listProperties.second.second; ++it)
            query.addBindValue((*it).toULongLong());

        if (!d->execute(query))
            qLog(Messaging) << "MailMessage deletionProperties query execute failed";

        while (query.next()) {
            DeletionProperties properties;
            properties.id = QMailId(query.value(0).toULongLong());
            properties.serverUid = query.value(1).toString();
            properties.fromAccount = query.value(2).toString();
            properties.fromMailbox = query.value(3).toString();

            result.append(properties);
        }
    }

	return result;	
}


Q_GLOBAL_STATIC(QMailStore,QMailStoreInstance);

/*!
    Returns an instance of the QMailStore object.
*/

QMailStore* QMailStore::instance()
{
    static bool init = false;
    if(!init) init = QMailStorePrivate::initStore();
    return QMailStoreInstance();
}

