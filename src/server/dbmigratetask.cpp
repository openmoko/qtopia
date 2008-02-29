/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qtopiaserverapplication.h"
#include <QProcess>
#include <qtopianamespace.h>
#include "dbmigratetask.h"

/*!
  \class DBMigrateTask
  \ingroup QtopiaServer::Task
  \brief The DBMigrateTask class runs dbmigrate.

  The DBMigrateTask class run dbmigrate to ensre the database tables are
  in a format the current version of Qtopia expects.
*/

/*! \internal

  Block the server's startup while we wait for dbmigrate to run.
*/
void DBMigrateTask::rundbmigrate()
{
    QString dbmigrate = Qtopia::qtopiaDir() + "bin/dbmigrate";
    QProcess proc;
    proc.start( dbmigrate, QStringList() << "--systemupgrade" );
    while ( proc.waitForStarted() == false ) {
        if ( proc.error() != QProcess::UnknownError )
            qFatal("Cannot run %s", dbmigrate.toLocal8Bit().constData());
    }
    while ( proc.waitForFinished() == false ) {
        if ( proc.error() != QProcess::UnknownError )
            qFatal("%s aborted", dbmigrate.toLocal8Bit().constData());
    }
    QByteArray output = proc.readAllStandardOutput();
    QByteArray error = proc.readAllStandardError();
    if ( output.count() || error.count() ) {
        qDebug() << "********************************************************************************" << endl
                 << "START dbmigrate output:" << endl;
        if ( output.count() )
            qDebug() << output.constData();
        if ( error.count() )
            qDebug() << error.constData();
        qDebug() << "END dbmigrate output" << endl
                 << "********************************************************************************" << endl;
    }
}

QTOPIA_STATIC_TASK(DBMigrate, DBMigrateTask::rundbmigrate());

