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

#include "dbrestoreop.h"

#include <qtopialog.h>
#include <qtopiasql.h>

#include <QSqlDriver>
#include <QSqlQuery>

/*!
  Construct a new DbRestoreOperation object.

  The database for the restore operation is determined by the \a id
  and that database will be used as the source of the restore.

  The target of the restore is the system database - for example the file
  Qtopia::applicationFileName("Qtopia","qtopia_db.sqlite").
*/
DbRestoreOperation::DbRestoreOperation( QtopiaDatabaseId id, QObject *parent )
    : DbOperation( parent )
    , dbId( id )
{
}

/*!
  Destroy the restore operation object.
*/
DbRestoreOperation::~DbRestoreOperation()
{
}

/*!
  Restore the database for this operation by migrating it into
  the current system database.

  The database for the operation is determined when constructing this
  restore operation object.

  This method may produce errors.  If there is an error the migrate
  operation will be rolled back (on a transactional database).  To
  see the full text of errors turn on Sql logging.

  The system database is deemed to be authoritative for the schema.

  This means that if a column in the data being restored does not
  exist in the system database, that data from that column will not
  be migrated.  If a table from the restore source is not present in
  the system database, that whole table is not restored.

  Conversely if a column in the system database does not exist in the
  data being restored, it will be filled with either NULL values, or
  (if NOT NULL) default values.

  \sa DbRestoreOperation()
*/
void DbRestoreOperation::restoreToSystem()
{
    QSqlDatabase &sysDb = QtopiaSql::systemDatabase();
    QStringList tables = sysDb.tables();
    const DbSchema *src = DbOperation::factory()->schemaForDb( dbId );
    const DbSchema *sys = DbOperation::factory()->schemaForDb( 0 );
    bool transact = sysDb.driver()->hasFeature( QSqlDriver::Transactions );
    QSqlQuery xsql( sysDb );
    QString migrateQuery = "BEGIN TRANSACTION";

    if ( transact && !sysDb.transaction() )
        goto DB_ERROR;
    foreach ( QString tb, tables )
    {
        migrateQuery = src->migrate( tb, sys );
        QStringList qryList = migrateQuery.split( ";", QString::SkipEmptyParts );
        foreach( QString q, qryList )
            if ( !xsql.exec( q ) )
                goto DB_ERROR;
        qLog(Sql) << "Migrated table" << tb << "\n" << migrateQuery;
    }
    migrateQuery = "COMMIT TRANSACTION";
    if ( transact && !sysDb.commit() )
        goto DB_ERROR;
    return;
DB_ERROR:
    QSqlError e = sysDb.lastError();
    QString errString = QString( "Error migrating from %1 to system" ).arg( src->dbName() );
    if ( !e.databaseText().isEmpty() )
        errString = errString + "\nDatabase error:" + e.databaseText();
    if ( !e.driverText().isEmpty() )
        errString = errString + "\nDriver error:" + e.driverText();
    qLog(Sql) << errString;
    if ( errString.length() > 70 )
        errString = errString.left( 70 ) + "...";
    qWarning( "%s", qPrintable( errString ));
    if ( transact )
        sysDb.rollback();
}
