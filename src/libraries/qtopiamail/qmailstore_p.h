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

#ifndef __QMAILSTOREPRIVATE_H
#define __QMAILSTOREPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QSharedData>
#include <QSqlDatabase>
#include <QString>
#include <QCache>
#include "mailbodystore_p.h"
#include "qmailfolder.h"
#include "qmailmessage.h"

class QMailId;
class QMailFolderKey;
class QMailFolderSortKey;
class QMailMessageKey;
class QMailMessageSortKey;
class QSqlRecord;
class QSqlQuery;

class MailMessageCache 
{
public:
    MailMessageCache(unsigned int headerCacheSize = 100);
    ~MailMessageCache();

    QMailMessage lookup(const QMailId& id) const;
    void insert(const QMailMessage& message);
    bool contains(const QMailId& id) const;
    void remove(const QMailId& id);
    
private:
    QCache<quint64,QMailMessage> mCache;     
    
};


class QMailStorePrivate : public QSharedData
{
public:
    QMailStorePrivate();
    QMailStorePrivate(const QMailStorePrivate& other);

    bool deleteFolder(const QMailId& f);
    bool deleteMailsFromFolder(const QMailId& f);
	QSqlQuery prepare(const QString& sql) const;
	bool execute(QSqlQuery& q) const;
	bool folderExists(const quint64& id);
	QMailMessage buildQMailMessage(const QSqlRecord& r) const;
	QMailFolder buildQMailFolder(const QSqlRecord& r) const;
	QString buildOrderClause(const QMailFolderSortKey& key) const;
	QString buildOrderClause(const QMailMessageSortKey& key) const;
	QString buildWhereClause(const QMailMessageKey& k) const;
	void bindWhereData(const QMailMessageKey& key, QSqlQuery& query) const;
	QString buildWhereClause(const QMailFolderKey& k) const;
	void bindWhereData(const QMailFolderKey& key, QSqlQuery& query) const;
    static bool initStore();
    static bool setupTables(QStringList tableList, QSqlDatabase& db);
    static QString parseSql(QTextStream& ts);
    static int lookAhead() { return 5; };

public:
    mutable MailMessageCache headerCache;
    mutable QSqlDatabase database;
	MailBodyStore mailBodyStore;
};

#endif
