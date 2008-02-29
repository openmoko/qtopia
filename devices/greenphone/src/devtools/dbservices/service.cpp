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

#include "service.h"
#include "dbrestoreop.h"

#include <QSqlQuery>

#include <qtopiasql.h>
#include <qdebug.h>

/*!
  Construct a new DbServices object, publishing all its slots
*/
DbServices::DbServices( QObject *parent )
    : QtopiaAbstractService( "DbServices", parent )
{
    publishAll();
    qDebug() << "constructed DbServices";
}

/*!
  Provide the databaseRestore(QString) service

  This service will restore the database identified by the \a dbSrc string
  into the system database
*/
void DbServices::databaseRestore( const QString &dbSrc )
{
    qDebug() << "database restore service";
    DbRestoreOperation *op = DbOperation::factory()->restoreOp( dbSrc, this );
    op->restoreToSystem();
}

/*!
  Provide the databaseBackup(QString) service

  This service will backup the system database to the location identified by
  the \a dbDst string.

  TODO - implement this service
*/
void DbServices::databaseBackup( const QString &dbDst )
{
    Q_UNUSED( dbDst );
    qDebug() << "database backup service";
}
