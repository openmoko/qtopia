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

#include "qmailstore_p.h"
#include "qmailfoldersortkey.h"
#include "qmailfoldersortkey_p.h"
#include "qmailmessagesortkey.h"
#include "qmailmessagesortkey_p.h"
#include "qmailmessagekey.h"
#include "qmailmessagekey_p.h"
#include "qmailfolderkey.h"
#include "qmailfolderkey_p.h"
#include "qmailtimestamp.h"

#include <qtopialog.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMutex>
#include <QDSAction>
#include <QDSServiceInfo>
#include <QTextCodec>
#include <QtopiaSql>
#include <QSystemMutex>
#include <sys/types.h>
#include <sys/ipc.h>

MailMessageCache::MailMessageCache(unsigned int headerCacheSize)
:
    mCache(headerCacheSize)
{
}

MailMessageCache::~MailMessageCache()
{
}

QMailMessage MailMessageCache::lookup(const QMailId& id) const
{
   if(!id.isValid())
       return QMailMessage();
   else
   {
       QMailMessage* cachedMessage = mCache.object(id.toULongLong());
       if(!cachedMessage)
           return QMailMessage();
       else return *cachedMessage;
   }
}

void MailMessageCache::insert(const QMailMessage& message)
{
    if(!message.id().isValid())
        return;
    else
        mCache.insert(message.id().toULongLong(),new QMailMessage(message));
}

bool MailMessageCache::contains(const QMailId& id) const
{
    return mCache.contains(id.toULongLong());
}

void MailMessageCache::remove(const QMailId& id)
{
    mCache.remove(id.toULongLong());
}

QMailStorePrivate::QMailStorePrivate()
:
    QSharedData()
{
}

QMailStorePrivate::QMailStorePrivate(const QMailStorePrivate& other)
:
    QSharedData(other),
    database(other.database)
{
}

/*!
    Recursive helper function used to delete a QMailFolder \a p
    and its child folders.  Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStorePrivate::deleteFolder(const QMailId& p)
{
	//delete the child folders
	QSqlQuery query = prepare("SELECT id FROM mailfolders WHERE parentid = ?");

	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(p.toULongLong());

	if(!execute(query))
		return false;

	//get the list of id's

	QList<quint64> idList;

	while(query.next())
		idList.append(query.value(0).toULongLong());	

	foreach(quint64 id, idList)
		if(!deleteFolder(QMailId(id)))
			return false;

	//delete the mails for this folder

	if(!deleteMailsFromFolder(p))
		return false;

	//delete this folder

	query = prepare("DELETE FROM mailfolders WHERE id = ?");

	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(p.toULongLong());

	if(!execute(query))
		return false;

	//_folderSync->notifyRemoved(p.parentId(),p.id());

	return true;

}

/*!
    Helper function used to delete the mails contained in 
    QMailfolder \a f. Returns \c true if the operation
    completed successfully, \c false otherwise. 
*/

bool QMailStorePrivate::deleteMailsFromFolder(const QMailId& f)
{
	//get the list of mail files to delete

	QSqlQuery query = prepare("SELECT mailfile FROM mailmessages WHERE parentfolderid = ?");
	
	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(f.toULongLong());

	if(!execute(query))
		return false;

	QStringList mailfiles;
    QMailIdList mailIds;

	while(query.next())
    {
		mailfiles.append(query.value(0).toString());
        mailIds.append(QMailId(query.value(1).toULongLong()));
    }

	query = prepare("DELETE FROM mailmessages WHERE parentfolderid = ?");
	
	if(query.lastError().type() != QSqlError::NoError)
		return false;

	query.addBindValue(f.toULongLong());

	if(!execute(query))
		return false;

	//delete the mail files

	foreach(QString mailfile, mailfiles)
	{
		if(!mailBodyStore.remove(mailfile))
			qLog(Messaging) << "Could not remove the mail body " << mailfile;
	}

    //delete any cached id's

    foreach(QMailId id, mailIds)
        headerCache.remove(id);

	return true;
}

QSqlQuery QMailStorePrivate::prepare(const QString& sql) const
{
	QSqlQuery query(database);
	
	if(!query.prepare(sql))
	{
		qLog(Messaging) << "Failed to prepare query " << 
			query.lastQuery().toLocal8Bit().constData() << " : " << 
			query.lastError().text().toLocal8Bit().constData();     
		database.rollback();
	}
	return query;
}

bool QMailStorePrivate::execute(QSqlQuery& query) const
{
	if(!query.exec())
	{
		qLog(Messaging) << "Failed to execute query " <<
			query.lastQuery().toLocal8Bit().constData() << " : " << 
			query.lastError().text().toLocal8Bit().constData();
		database.rollback();
		return false;
	}
    qLog(Messaging) << query.executedQuery().simplified();
	return true;
}

bool QMailStorePrivate::folderExists(const quint64& id)
{
	QSqlQuery query(database);
	if(!query.prepare("SELECT id FROM mailfolders WHERE id = ?"))
	{
		qLog(Messaging) << "Failed to prepare query " << 
			query.lastQuery().toLocal8Bit().constData() << " : " << 
			query.lastError().text().toLocal8Bit().constData();     
		database.rollback();
		return false;
	}

	query.addBindValue(id);

	if(!query.exec())
	{
		qLog(Messaging) << "Failed to select from table mailfolders :" <<
			query.lastError().text().toLocal8Bit().constData();
		database.rollback();
		return false;
	}

	return (query.first());
}

QMailMessage QMailStorePrivate::buildQMailMessage(const QSqlRecord& r) const
{
	//build a mailmessage from a retrieved sql record
	QMailId id(r.value("id").toULongLong());
	QMailMessage::MessageType t = QMailMessage::MessageType(r.value("type").toInt());
	QMailId parentfolderid(r.value("parentfolderid").toULongLong());
	QString sender = r.value("sender").toString();
	QStringList recipients(r.value("recipients").toString());
	QString subject = r.value("subject").toString();
	QDateTime timestamp = r.value("stamp").toDateTime();
	QMailMessage::Status flags(r.value("status").toInt());
	QString fromAccount = r.value("fromaccount").toString();
	QString fromMailbox = r.value("frommailbox").toString();
	QString serveruid = r.value("serveruid").toString();
	int size = r.value("size").toInt();

	QMailMessage newMessage;
	QString mailfile = r.value("mailfile").toString();

	if(!mailfile.isEmpty())
    {
		if(!mailBodyStore.load(mailfile,&newMessage))
			qLog(Messaging) << "Could not load message body " << mailfile;
    }
    else
    {
        static const QString smsTag("@sms");

        // Remove sms-origin tag, if present - the SMS client previously appended
        // "@sms" to the from address, which is no longer necesary
        if (sender.endsWith(smsTag))
            sender.chop(smsTag.length());

        //values in mail take precedence over valus in DB.
        newMessage.setFrom(QMailAddress(sender));
        newMessage.setSubject(subject);
        newMessage.setDate(QMailTimeStamp(timestamp));
        newMessage.setTo(QMailAddress::fromStringList(recipients));
    }

	newMessage.setId(id);
	newMessage.setParentFolderId(parentfolderid);
	newMessage.setStatus(flags);
	newMessage.setSize(size);
	newMessage.setFromAccount(fromAccount);
	newMessage.setFromMailbox(fromMailbox);
	newMessage.setServerUid(serveruid);
    newMessage.setMessageType(t);

    newMessage.changesCommitted();

	return newMessage;
}

QMailFolder QMailStorePrivate::buildQMailFolder(const QSqlRecord& r) const
{
	QMailId id = QMailId(r.value("id").toULongLong());
	QString name = r.value("name").toString();
	QMailId parentId = QMailId(r.value("parentid").toULongLong());

	QMailFolder result(name,parentId);
	result.setId(id);
	return result;
}

QString QMailStorePrivate::buildOrderClause(const QMailFolderSortKey& key) const
{
	//convert the key to an sql selection string

	QString sortClause = " ORDER BY ";

	for(int i =0; i < key.d->arguments.count(); ++i)
	{
		QMailFolderSortKeyPrivate::Argument a = key.d->arguments.at(i);
		if(i > 0)
			sortClause += ",";
		switch(a.first)
		{
			case QMailFolderSortKey::Id:
				sortClause += "id";
				break;
			case QMailFolderSortKey::Name:
				sortClause += "name";
				break;
			case QMailFolderSortKey::ParentId:
				sortClause += "parentid";
				break;
		}
		if(a.second == Qt::AscendingOrder)
			sortClause += " ASC";
		else
			sortClause += " DESC";
	}
	return sortClause;

}

QString QMailStorePrivate::buildOrderClause(const QMailMessageSortKey& key) const
{
	//convert the key to an sql selection string

	QString sortClause = " ORDER BY ";

	for(int i =0; i < key.d->arguments.count(); ++i)
	{
		QMailMessageSortKeyPrivate::Argument a = key.d->arguments.at(i);
		if(i > 0)
			sortClause += ",";
		
		switch(a.first)
		{
			case QMailMessageSortKey::Id:
				sortClause += "id";
				break;
			case QMailMessageSortKey::Type:
				sortClause += "type";
				break;
			case QMailMessageSortKey::ParentFolderId:
				sortClause += "parentfolderid";
				break;
			case QMailMessageSortKey::Sender:
				sortClause += "sender";
				break;
			case QMailMessageSortKey::Recipients:
				sortClause += "recipients";
				break;
			case QMailMessageSortKey::Subject:
				sortClause += "subject";
				break;
			case QMailMessageSortKey::TimeStamp:
				sortClause += "stamp";
				break;
			case QMailMessageSortKey::Flags:
				sortClause += "flags";
				break;
			case QMailMessageSortKey::FromAccount:
				sortClause += "fromaccount";
				break;
			case QMailMessageSortKey::FromMailbox:
				sortClause += "frommailbox";
				break;
			case QMailMessageSortKey::ServerUid:
				sortClause += "serveruid";
				break;
			case QMailMessageSortKey::Size:
				sortClause += "size";
				break;


		}
		if(a.second == Qt::AscendingOrder)
			sortClause += " ASC";
		else
			sortClause += " DESC";
	}
	return sortClause;

}

QString QMailStorePrivate::buildWhereClause(const QMailMessageKey& key) const
{
	//convert the key to an sql selection string
	QString logicalOpString = key.d->logicalOp == QMailMessageKeyPrivate::Or ? " OR " : " AND ";
	QString queryString;
	QTextStream q(&queryString);

	QString compareOpString;
	QString op = " ";

	foreach(QMailMessageKeyPrivate::Argument a,key.d->arguments)
	{
        bool addressRelated(a.property == QMailMessageKey::Sender || a.property == QMailMessageKey::Recipients);

		switch(a.op)
		{
			case QMailMessageKey::LessThan:
				compareOpString = " < ";
				break;
			case QMailMessageKey::GreaterThan:
				compareOpString = " > ";
				break;
			case QMailMessageKey::Equal:
                // When matching addreses, we will force equality to actually use a content match
				compareOpString = (addressRelated ? " LIKE " : " = ");
				break;
			case QMailMessageKey::LessThanEqual:
				compareOpString = " <= ";
				break;
			case QMailMessageKey::GreaterThanEqual:
				compareOpString = " >= ";
				break;
			case QMailMessageKey::NotEqual:
				compareOpString = " <> ";
				break;
			case QMailMessageKey::Contains:
				compareOpString = " LIKE ";
				break;
		}

		//TODO provide a  more elegant solution to this.

		QVariant var = a.value;

		switch(a.property)
		{
			case QMailMessageKey::Id:
				{
					QMailId id = var.value<QMailId>();
					q << op << "id " << compareOpString << '?'; 
				}
				break;
			case QMailMessageKey::Type:
                if(a.op == QMailMessageKey::Contains)
                    q << op << "type & " << '?'; 
                else
                    q << op << "type " << compareOpString << '?'; 
				break;
			case QMailMessageKey::ParentFolderId:
				{
					QMailId id = var.value<QMailId>();
					q << op << "parentfolderid " << compareOpString << '?'; 
				}
				break;
			case QMailMessageKey::Sender:
				q << op << "sender " << compareOpString << '?'; 
				break;
			case QMailMessageKey::Recipients:
				q << op << "recipients " << compareOpString << '?'; 
				break;    
			case QMailMessageKey::Subject:
				q << op << "subject " << compareOpString << '?'; 
				break;    
			case QMailMessageKey::TimeStamp:
				q << op << "timestamp " << compareOpString << '?';
				break;    
			case QMailMessageKey::Flags:
                if(a.op == QMailMessageKey::Contains)
                    q << op << "status & " << '?';
                else
				    q << op << "status " << compareOpString << '?'; 
				break;    
			case QMailMessageKey::FromAccount:
				q << op << "fromaccount " << compareOpString << '?'; 
				break;    
			case QMailMessageKey::FromMailbox:
				q << op << "frommailbox " << compareOpString << '?'; 
				break;    
			case QMailMessageKey::ServerUid:
				q << op << "serveruid " << compareOpString << '?'; 
				break;   
			case QMailMessageKey::Size:
				q << op << "size " << compareOpString << '?'; 
				break;     
		}
		op = logicalOpString;
	}

	//subkeys

	if(queryString.isEmpty())
        op = " ";

	foreach(QMailMessageKey subkey,key.d->subKeys)
	{
		QString subquery = buildWhereClause(subkey);
		q << op << " ( " << subquery << " ) ";
		op = logicalOpString;
	}       

	if(key.d->negated)
		return "NOT " + queryString;
	else
		return queryString;
}

void QMailStorePrivate::bindWhereData(const QMailMessageKey& key, QSqlQuery& query) const
{
	foreach(QMailMessageKeyPrivate::Argument a,key.d->arguments)
	{
		QVariant var = a.value;
        QString stringData = var.toString();

        if ((a.property == QMailMessageKey::Sender) || (a.property == QMailMessageKey::Recipients)) {
            // If the query argument is a phone number, ensure it is in minimal form
            QMailAddress address(stringData);
            if (address.isPhoneNumber()) {
                stringData = address.minimalPhoneNumber();

                // Rather than compare exact numbers, we will only use the trailing
                // digits to compare phone numbers - otherwise, slightly different 
                // forms of the same number will not be matched
                static const int significantDigits = 8;

                int extraneous = stringData.length() - significantDigits;
                if (extraneous > 0)
                    stringData.remove(0, extraneous);
            }
        }

        //delimit data for sql "LIKE" operator
        bool addressRelated(a.property == QMailMessageKey::Sender || a.property == QMailMessageKey::Recipients);
        if((a.op == QMailMessageKey::Contains) || 
           ((a.op == QMailMessageKey::Equal) && addressRelated)) {
            if (stringData.isEmpty()) {
                // Is there any point in matching everything, because this comparator 
                // is empty?  It looks wrong to me...
            } else {
                stringData = "\%" + stringData + "\%";
            }
        }

		switch(a.property)
		{
			case QMailMessageKey::Id:
				{
					QMailId id = var.value<QMailId>();
					query.addBindValue(id.toULongLong());
				}
				break;
			case QMailMessageKey::Type:
				query.addBindValue(var.toInt());
				break;
			case QMailMessageKey::ParentFolderId:
				{
					QMailId id = var.value<QMailId>();
					query.addBindValue(id.toULongLong());
				}
				break;
			case QMailMessageKey::Sender:
				query.addBindValue(stringData);
				break;
			case QMailMessageKey::Recipients:
				query.addBindValue(stringData);
				break;    
			case QMailMessageKey::Subject:
				query.addBindValue(stringData);
				break;    
			case QMailMessageKey::TimeStamp:
				query.addBindValue(stringData);
				break;    
			case QMailMessageKey::Flags:
				query.addBindValue(var.toUInt());
				break;    
			case QMailMessageKey::FromAccount:
				query.addBindValue(stringData);
				break;    
			case QMailMessageKey::FromMailbox:
				query.addBindValue(stringData);
				break;    
			case QMailMessageKey::ServerUid:
				query.addBindValue(stringData);
				break;   
			case QMailMessageKey::Size:
				query.addBindValue(var.toInt());
				break;     
		}
	}

	//subkeys

	foreach(QMailMessageKey subkey,key.d->subKeys)
		bindWhereData(subkey,query);

}

QString QMailStorePrivate::buildWhereClause(const QMailFolderKey& key) const
{
	//convert the key to an sql selection string
	QString logicalOpString = key.d->logicalOp == QMailFolderKeyPrivate::Or ? " OR " : " AND ";
	QString queryString;
	QTextStream q(&queryString);

	QString compareOpString;
	QString op = " ";

	foreach(QMailFolderKeyPrivate::Argument a,key.d->arguments)
	{
		switch(a.op)
		{
			case QMailFolderKey::LessThan:
				compareOpString = " < ";
				break;
			case QMailFolderKey::GreaterThan:
				compareOpString = " > ";
				break;
			case QMailFolderKey::Equal:
				compareOpString = " = ";
				break;
			case QMailFolderKey::LessThanEqual:
				compareOpString = " <= ";
				break;
			case QMailFolderKey::GreaterThanEqual:
				compareOpString = " >= ";
				break;
			case QMailFolderKey::NotEqual:
				compareOpString = " <> ";
				break;
			case QMailFolderKey::Contains:
				compareOpString = " LIKE ";
				break;
		}
		QVariant var = a.value;
		switch(a.property)
		{
			case QMailFolderKey::Id:
				{
					QMailId id = var.value<QMailId>();
					q << op << "id " << compareOpString << '?';            
				}
				break;
			case QMailFolderKey::Name:
				q << op << "name " << compareOpString << '?'; 
				break;
			case QMailFolderKey::ParentId:
				{
					QMailId id = var.value<QMailId>();
					q << op << "parentid " << compareOpString << '?';
				}
				break;
		}
		op = logicalOpString;
	}

	//subkeys
    if(queryString.isEmpty())
        op = " ";

	foreach(QMailFolderKey subkey,key.d->subKeys)
	{
		QString subquery = buildWhereClause(subkey);
		q << op << " ( " << subquery << " ) ";
		op = logicalOpString;

	}       

	if(key.d->negated)
		return "NOT " + queryString;
	else
		return queryString;

}

void QMailStorePrivate::bindWhereData(const QMailFolderKey& key, QSqlQuery& query) const
{
	foreach(QMailFolderKeyPrivate::Argument a,key.d->arguments)
	{
		QVariant var = a.value;
        switch(a.property)
		{

			case QMailFolderKey::Id:
				{
					QMailId id = var.value<QMailId>();
					query.addBindValue(id.toULongLong());
				}
				break;
			case QMailFolderKey::Name:
                {
                    //delimit data for sql "LIKE" operator 

                    QString stringData = var.toString();
                    if(a.op == QMailFolderKey::Contains)
                        stringData = "\%" + stringData + "\%";
                    query.addBindValue(stringData);
                }
				break;
                
			case QMailFolderKey::ParentId:
				{
					QMailId id = var.value<QMailId>();
					query.addBindValue(id.toULongLong());
				}
				break;
		}
	}

	foreach(QMailFolderKey subkey,key.d->subKeys)
		bindWhereData(subkey,query);
}

bool QMailStorePrivate::initStore()
{
    bool result = false;
    QSqlDatabase db = QtopiaSql::instance()->applicationSpecificDatabase("qtmail");

    int id = (int)::ftok("qmailstoredb", 0);
    QSystemMutex sMutex(id,false);
    sMutex.lock(100);

	if(!db.isOpenError())
        result = setupTables(QStringList() << "mailfolders" << "mailmessages",db);

    sMutex.unlock();

    return result;
}

bool QMailStorePrivate::setupTables(QStringList tableList, QSqlDatabase& db)
{
    QStringList tables = db.tables();

    bool result = true;

    foreach(QString table, tableList) {
        
        if (tables.contains(table, Qt::CaseInsensitive))
            continue;
        
        // load schema.
        QFile data(QLatin1String(":/QtopiaSql/") + db.driverName() + QLatin1String("/") + table);
        if(!data.open(QIODevice::ReadOnly))
        {
            qLog(Messaging) << "Failed to load table resource " << table;
            result &= false;
        }
        QTextStream ts(&data);
        // read assuming utf8 encoding.
        ts.setCodec(QTextCodec::codecForName("utf8"));
        ts.setAutoDetectUnicode(true);
        
        QString sql = parseSql(ts);
        while(!sql.isEmpty())
        {
            QSqlQuery query(db);
            if(!query.exec(sql))
            {
                qLog(Messaging) << "Failed to exec table creation SQL query " << sql << " " \
                    << query.lastError().text();
                result &= false;
            }
            sql = parseSql(ts);
        }
    }
    return result;
}

QString QMailStorePrivate::parseSql(QTextStream& ts)
{
    QString qry = "";
    while(!ts.atEnd())
    {
        QString line = ts.readLine();
        // comment, remove.
        if (line.contains (QLatin1String("--")))
            line.truncate (line.indexOf (QLatin1String("--")));
        if (line.trimmed ().length () == 0)
            continue;
        qry += line;
        
        if ( line.contains( ';' ) == false) 
            qry += QLatin1String(" ");
        else
            return qry;
    }
    return qry;
}

