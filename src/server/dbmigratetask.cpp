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

#include "qtopiaserverapplication.h"
#include <QProcess>
#include <qtopianamespace.h>
#include <QtopiaSql>
#include "dbmigratetask.h"

/*!
  \class DBMigrateTask
  \ingroup QtopiaServer::Task
  \brief The DBMigrateTask class runs dbmigrate.

  The DBMigrateTask class run dbmigrate to ensre the database tables are
  in a format the current version of Qtopia expects.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
*/

/*! \internal

  Block the server's startup while we wait for dbmigrate to run.
*/
void DBMigrateTask::rundbmigrate()
{
    QtopiaSql::instance()->openDatabase();
    QString dbmigrate = Qtopia::qtopiaDir() + "bin/dbmigrate";
    QProcess proc;
    proc.start( dbmigrate, QStringList() << "--systemupgrade" );
    while ( proc.waitForStarted() == false ) {
        if ( proc.error() != QProcess::UnknownError )
            qFatal("Cannot run %s: %s", qPrintable(dbmigrate), qPrintable(proc.errorString()));
    }
    while ( proc.waitForFinished() == false ) {
        if ( proc.error() != QProcess::UnknownError )
            qFatal("%s aborted: %s", qPrintable(dbmigrate), qPrintable(proc.errorString()));
    }
    QByteArray output = proc.readAllStandardOutput();
    QByteArray error = proc.readAllStandardError();
    if ( output.count() || error.count() ) {
        qWarning() << "********************************************************************************" << endl
                   << "START dbmigrate output:" << endl;
        if ( output.count() )
            qWarning() << output.constData();
        if ( error.count() )
            qWarning() << error.constData();
        qWarning() << "END dbmigrate output" << endl
                   << "********************************************************************************" << endl;
    }

    // Query the tables in each database to ensure any cached table info is updated with changes made by dbmigrate.
    foreach( QSqlDatabase database, QtopiaSql::instance()->databases() )
        database.tables();
}

QTOPIA_STATIC_TASK(DBMigrate, DBMigrateTask::rundbmigrate());

