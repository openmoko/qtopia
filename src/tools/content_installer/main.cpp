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

#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QTextCodec>

#include <qtopiasql.h>
#include <qcontent.h>
#include <qtopianamespace.h>
#include <private/contentlnksql_p.h>
#include <qtopialog.h>
#include "../dbmigrate/migrateengine.h"

#include <qdebug.h>

#include <time.h>
#include <stdio.h>
int main( int argc, char** argv )
{
    setenv( "LANG", "en_US.utf8", 1 ); //Force LANG to UTF-8 so correct bytes are written to the database.

    QApplication app(argc, argv, false);
    if ( argc < 6  ) {
        printf("Usage: %s database prefix destpath categories file/s\n", app.arguments().at(0).toLocal8Bit().constData());
        return 1;
    }

    QString dbPath( app.arguments().at(1) );
    QString dbLockPath = dbPath + ".lock";
    QFileInfo lkInfo( dbLockPath );
    QDir lkDir( lkInfo.absolutePath() );
    if ( !lkDir.exists() && !QDir::root().mkpath( lkInfo.absolutePath() ))
        qFatal( "Cannot create directory %s", qPrintable( lkInfo.absolutePath() ));
    QFile lk( dbLockPath );
    if ( !lk.open( QIODevice::WriteOnly ))
        qFatal( "Could not open %s", qPrintable( dbLockPath ));
    int tries = 1000;
    while ( !Qtopia::lockFile( lk, Qtopia::LockWrite ))
    {
        if ( tries == 1000 )
            printf( "\ttrying to lock %s", qPrintable( dbLockPath ));
        if ( tries % 50 == 0 )
            printf( "." );
        if ( --tries == 0 )
            break;
        Qtopia::msleep( 100 );
    }
    if ( tries == 0 )
        qFatal( "Could not lock file %s", qPrintable( dbLockPath ));

    // Add one or more .desktop files to the database
    QtopiaSql::loadConfig(QString ("QSQLITE"), dbPath, QString ());
    if(!QDBMigrationEngine::instance()->doMigrate(QStringList() << dbPath))
    {
        Qtopia::unlockFile( lk );
        lk.remove();
        qFatal( "Database creation/upgrade failed on database %s", qPrintable( dbPath ));
    }
    QString prefixPath = app.arguments().at(2);
    QString destPath = app.arguments().at(3);
    QStringList categories = app.arguments().at(4).split(" ", QString::SkipEmptyParts);
    for (int i=5; i<app.arguments().count (); i++)
    {
        QString sourceFile = app.arguments().at(i);
        QString destFile = prefixPath + destPath + QLatin1Char('/') + QFileInfo(sourceFile).fileName();
        // Remove the existing entry (we don't want duplication)
        QContent content(destFile, false);
        if(content.id() != QContent::InvalidId)
            QContent::uninstall(content.id());
        // Create a new entry
        content=QContent(sourceFile, false);
        if ( QFileInfo(sourceFile).suffix() == "desktop" )
            content.setLinkFile(destFile);
        if( content.role() != QContent::Application && content.fileKnown() )
            if(QFileInfo(content.file()).absolutePath() == QFileInfo(sourceFile).absolutePath())
                content.setFile(QDir::cleanPath(QFileInfo(destFile).absolutePath()+ QLatin1Char('/')+QFileInfo(content.file()).fileName()));
        if ( QFileInfo(sourceFile).suffix() != "desktop" ) {
            content.setName( QFileInfo(sourceFile).baseName() );
            content.setRole( QContent::Data );
        }
        // categories can come in via the .pro file (required for hint=content)
        if ( categories.count() != 0 )
            content.setCategories( categories );
        content.commit();
    }
    Qtopia::unlockFile( lk );
    lk.remove();
}
