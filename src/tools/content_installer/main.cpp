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

#include <qtopiasql.h>
#include <qcontent.h>
#include <qtopianamespace.h>
#include <private/contentlnksql_p.h>
#include <qtopialog.h>

#include <qdebug.h>

#include <time.h>

bool doFinalizeResources( const QString &dbPath )
{
    if ( QFile::exists( dbPath ))
        QFile::remove( dbPath );
    QString dbType( "QSQLITE" );
    QSqlDatabase db = QSqlDatabase::addDatabase( dbType );
    db.setDatabaseName( dbPath );
    if ( !db.open() )
    {
        qWarning( "Could not open database %s", qPrintable( dbPath ));
        return false;
    }
    QString schemaPrefix( ":/QtopiaSql/" );
    schemaPrefix += dbType;
    QDir schema( schemaPrefix );
    QStringList schemaList = schema.entryList();
    bool transact = db.driver()->hasFeature( QSqlDriver::Transactions );
    foreach ( QString s, schemaList )
    {
        QtopiaSql::loadSchema( db, schemaPrefix + "/" + s, transact );
        qLog(Sql) << "loaded schema for" << s;
    }
    return true;
}

/*!
  main = install the application via its .desktop file
*/
int main( int argc, char** argv )
{
    QApplication app(argc, argv, false);
    bool doFinalize = false;
    if ( argc < 4 )
        qFatal( "Usage: %s { installroot depotpath prefix file/s | -finalize depotpath installpath }\n", argv[0] );

    if ( strncmp( argv[1], "-finalize", 9 ) == 0 )
        doFinalize = true;

    QString dbPath( argv[ doFinalize ? 3 : 1 ] );
    if ( doFinalize )
        dbPath += "/etc/default/Trolltech/qtopia_db.sqlite";
    else
        dbPath += "/qtopia_db.sqlite";
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
        Qtopia::msleep( 10 );
    }
    if ( tries == 0 )
        qFatal( "Could not lock file %s", qPrintable( dbLockPath ));

    if ( doFinalize )
    {
        doFinalizeResources( dbPath );
    }
    else
    {
        QtopiaSql::loadConfig(QString ("QSQLITE"), dbPath, QString ());
        QString depotPath(argv[2]), prefixPath(argv[3]);
        for (int i=4; i<app.arguments ().count (); i++)
        {
            QFileInfo fi( app.arguments().at( i ));
            QContent content( fi, false );
            if(content.id() != QContent::InvalidId)
            {
                QContent::uninstall(content.id());
                content=QContent(fi, false);
            }
            if(content.file().startsWith(depotPath))
            {
                QString newPath=content.file();
                newPath=newPath.replace(depotPath, prefixPath);
                content.setFile(newPath);
            }
            if(content.linkFile().startsWith(depotPath))
            {
                QString newPath=content.linkFile();
                newPath=newPath.replace(depotPath, prefixPath);
                content.setLinkFile(newPath);
            }
            content.commit();
        }
    }
    Qtopia::unlockFile( lk );
    lk.remove();
}
