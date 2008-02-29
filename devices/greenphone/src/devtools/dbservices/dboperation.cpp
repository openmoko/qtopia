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

#include "dboperation.h"
#include "dbrestoreop.h"

#include <qtopiasql.h>

/*!
  \internal
  ctor - private
*/
DbOperation::DbOperation( QObject *parent )
    : QObject( parent )
{
}

/*!
  Destroy this instance
*/
DbOperation::~DbOperation()
{
}

/*!
  Return a singleton instance of the DbOperation class which
  can be used as a factory.

  \sa restoreOp(), backupOp()
*/
DbOperation *DbOperation::factory()
{
    static DbOperation dbOp;

    return &dbOp;
}

/*!
  Return a newly constructed instance of a DbRestoreOperation object
  which can carry out a restore from the database at \a srcDb

  The instance is parented onto \a parent, if non-zero; otherwise
  it is parented onto the factory object itself.
*/
DbRestoreOperation *DbOperation::restoreOp( const QString &srcDb, QObject *parent )
{
    connectToSystemDatabase();
    QtopiaSql::attachDB( srcDb, srcDb );
    QtopiaDatabaseId dbId = QtopiaSql::databaseIdForPath( srcDb );
    extractSchema( dbId );
    return new DbRestoreOperation( dbId, parent ? parent : this );
}

/*!
  Extract the Schema for the database \a db.

  The resulting schema is stored in the factory's internal list
  and may be retrieved with the schemaForDb() method.

  \sa schemaForDb()
*/
void DbOperation::extractSchema( QtopiaDatabaseId dbId )
{
    QSqlDatabase &db = QtopiaSql::database( dbId );
    schemaList[dbId] = new DbSchema( db );
}

/*!
  Extract the Schema for the system database.

  The resulting schema is stored in the factory's internal list
  and may be retrieved with the schemaForDb() method.

  \sa schemaForDb(), extractSchema()
*/
void DbOperation::extractSystemSchema()
{
    extractSchema( 0 );
}

/*!
  Connect to the Qtopia system database.  This is defined by the
  current home location.
*/
void DbOperation::connectToSystemDatabase()
{
    QtopiaSql::openDatabase();
    extractSystemSchema();
}
