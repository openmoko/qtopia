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

#include "packagemodel.h"
#include "packageinformationreader.h"
#include "httpfetcher.h"
#include "packagecontroller.h"

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QVariant>
#include <QDebug>
#include <QHttp>
#include <QTimer>
#include <QSettings>
#include <QFile>

#include <qstorage.h>
#include <qtopianamespace.h>

/**
  Model is:
  \list
    \o Installed
        \list
            \o Internal
                \list
                    \o Package A
                    \o Package B
                \endlist
            \o CF Card
                \list
                    \o Package C
                    \o Package D
                \endlist
            \o SD Card
                \list
                    \o Package E
                    \o Package F
                \endlist
        \endlist
    \o Available Network
        \list
            \list
                \o Package N
                \o Package P
            \endlist
            \o CF Card
                \list
                    \o Package G
                    \o Package H
                \endlist
            \o SD Card
                \list
                    \o Package J
                    \o Package K
                \endlist
            \o SD Card
                \list
                    \o Package L
                    \o Package M
                \endlist
        \endlist
    \endlist

    Where there are more than 2 packages in a category, they are
    further broken down by Section: eg Games, Utilities

    If there are no packages in a category then that category is
    not displayed.
*/

const unsigned int PackageModel::ID_SIZE = 32;
const unsigned int PackageModel::ROW_MAX = (2 << (PackageModel::ID_SIZE-10));

#define INVALID_PARENT 0x000000ffU

PackageModel::PackageModel( QObject* parent )
    : QAbstractItemModel( parent )
    , storage( 0 )
{
    networked = AbstractPackageController::factory( AbstractPackageController::network, this );
    installed = AbstractPackageController::factory( AbstractPackageController::installed, this );

#ifndef QT_NO_SXE
    QFile file( Qtopia::qtopiaDir() + QLatin1String( "/etc/default/Trolltech/PackageManager.conf" ) );

    if( file.open( QIODevice::ReadOnly ) )
    {
        QString header = QLatin1String( "[Sensitive]" );
        QChar bracket = QLatin1Char( '[' );

        QString line;

        while( !file.atEnd() && !(line = file.readLine()).startsWith( header ) );

        while( !file.atEnd() && !(line = file.readLine()).startsWith( bracket ) )
            sensitiveDomains.append( line.trimmed() );

        file.close();
    }
#endif

    // can only have a max of 15 top level items
    rootItems << installed << networked;

    for ( int i = 0; i < rootItems.count(); i++ )
        connect( rootItems[i], SIGNAL(updated()),
                this, SLOT(controllerUpdate()) );
    connect( networked, SIGNAL(packageInstalled(InstallControl::PackageInfo)),
            installed, SLOT(addPackage(InstallControl::PackageInfo)) );
    connect( networked, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect( networked, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( networked, SIGNAL(packageInstalled(InstallControl::PackageInfo)),
            this, SLOT(packageInstalled(InstallControl::PackageInfo)) );
    connect( networked, SIGNAL(serverStatus(QString)),
            this, SLOT(serverStatusUpdated(QString)) );

    // can only have a max of 4 columns, if more are needed, change the
    // macros used for PackageModel::index(...) below
    // columnHeads << "Name" << "Size";
    
    QStorageMetaInfo *s = QStorageMetaInfo::instance();
    connect( s, SIGNAL(disksChanged()),
            this, SLOT(publishTargets()) );
    connect( this, SIGNAL(targetsUpdated(QStringList)),
            installed, SLOT(reloadInstalledLocations(QStringList)) );

    if ( !QDir( Qtopia::packagePath() ).exists() )
        QDir::root().mkpath( Qtopia::packagePath() );
}

PackageModel::~PackageModel()
{
    if ( storage )
        delete storage;
}

/*!
  Populates the servers member variable
  by reading the PackageServers configuration file. 
*/
void PackageModel::populateServers()
{
    QString serversFile = Qtopia::applicationFileName( "packagemanager", "ServersList.conf" );
    QSettings *serversConf;
    if ( !QFile::exists( serversFile ) )
        serversConf = new QSettings( "Trolltech", "PackageManager" );
    else
        serversConf = new QSettings( serversFile, QSettings::NativeFormat );

    QStringList servConfList = serversConf->childGroups();
    QString srvName;
    for ( int srv = 0; srv < servConfList.count(); srv++ )
    {
        srvName = servConfList[srv];
        serversConf->beginGroup( srvName );
        if ( serversConf->contains( "URL" ) ) 
            servers[ srvName ] = serversConf->value( "URL" ).toString();
        
        serversConf->endGroup();
    }
}

/*!
  Return an instance of InstallControl for this model.

  Initializes the instance on first invocation.
*/
InstallControl *PackageModel::installControl()
{
    static InstallControl ic;
    return &ic;
}

void PackageModel::setServers( const QHash<QString,QString> &serverList )
{
    NetworkPackageController* network = qobject_cast<NetworkPackageController*>(networked);
    if ( network == 0 )
        return;
    
    if ( servers == serverList )
        return;
    servers = serverList;
    emit serversUpdated( getServers() );
}

void PackageModel::setServer( const QString& server )
{
    NetworkPackageController* network =
        qobject_cast<NetworkPackageController*>(networked);
    if ( network == 0 )
        return;

    activeServer = server;
    QString serverUrl = servers[ server ];
    if ( serverUrl != QString() ) {
        if ( network->networkServer() != serverUrl )
            network->setNetworkServer( serverUrl );
    }
}

void PackageModel::setInstallTarget( const QString &tgt )
{
    currentInstallTarget = tgt;
}

/*!
  \internal
  Receive notifications from a view that an item has been activated by
  that view, eg by double-click or pressing return.
*/
void PackageModel::activateItem( const QModelIndex &item )
{
    QModelIndex parent = item.parent();
    if ( !parent.isValid() )
        return;

    QString dataItem = data( item, Qt::DisplayRole ).toString();

    AbstractPackageController *c = rootItems[parent.row()];
    QString html;
    c->install( item.row() );
}

void PackageModel::reenableItem( const QModelIndex &item )
{
    QModelIndex parent = item.parent();
    if ( !parent.isValid() )
        return;

    qobject_cast<InstalledPackageController *>(installed)->reenable( item.row() ); 
}


QString PackageModel::getOperation( const QModelIndex &item )
{
    QModelIndex parent = item.parent();
    if ( parent.isValid() )
    {
        AbstractPackageController *c = rootItems[parent.row()];
        return c->operationName();
    }
    return QString();
}

void PackageModel::userTargetChoice( const QString &t )
{
    QString mediaPath = mediaNames[t];
    // See if this target contains the packagePath(), ie user has
    // changed back to using the "Internal" storage
    if ( Qtopia::packagePath().startsWith( mediaPath ))
    {
        installControl()->setInstallMedia( QString() );
    }
    else
    {
        installControl()->setInstallMedia( mediaPath );
    }
}

void PackageModel::controllerUpdate()
{
    emit layoutAboutToBeChanged();
    if ( installed )
        networked->setPackageFilter( installed->packageList() );
    emit layoutChanged();
}

void PackageModel::packageInstalled( const InstallControl::PackageInfo &pkg )
{
    // the row of member variable installed is 0, hence index(0, ...); start searching from first child  
    QModelIndexList matchList = match( this->index(0, 0).child(0,0),AbstractPackageController::Md5Sum ,QVariant(pkg.md5Sum), 1);
    if ( matchList.size() > 0  ) 
        emit newlyInstalled( matchList[0] );
}

void PackageModel::publishTargets()
{
    QFileSystemFilter fsf;
    fsf.applications = QFileSystemFilter::Set;
    QList<QFileSystem*> fl = QStorageMetaInfo::instance()->fileSystems( &fsf );
    QStringList targets;
    bool installMediaValid = installControl()->installMedia().isEmpty() ? true : false;
    for ( int i = 0; i < fl.count(); i++ )
    {
        mediaNames[ fl[i]->name() ] = fl[i]->path();
        targets << fl[i]->path();
        if ( !installMediaValid && fl[i]->path() == installControl()->installMedia() )
            installMediaValid = true;
    }
    emit targetsUpdated( targets );
    if ( !installMediaValid )
    {
        // install target went away (SD card removed etc)
        installControl()->setInstallMedia( QString() );
    }
}

/**
  Reimplemented from QAbstractDataModel::data
*/
QVariant PackageModel::data(const QModelIndex &index, int role) const
{
    QModelIndex parent = index.parent();
    if ( parent.isValid() )
    {
        AbstractPackageController *c = rootItems[parent.row()];
        return c->data( index.row(), index.column(), role );
    }
    else
    {
        if ( index.row() < rootItems.count() )
        {
            return rootItems[ index.row() ]->data( role );
        }
        else
        {
            return QVariant();
        }
    }
}

Qt::ItemFlags PackageModel::flags( const QModelIndex &index ) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

/*
QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return columnHeads[ section ];

    return QVariant();
}
*/

QModelIndex PackageModel::index( int row, int column, const QModelIndex &parent ) const
{
    if (!parent.isValid())  // root
    {
        if ( row > rootItems.count() )
            return QModelIndex();
        unsigned int internalId = makeId( row, 0, INVALID_PARENT );
        return createIndex( row, 0, internalId );
    }
    else
    {
        unsigned int parentId = getRow( parent.internalId() );
        return createIndex( row, column, makeId( row, column, parentId ));
    }
}

/*!
  \internal
  \reimp
  Calculate and return an index for the parent of \a index.
*/
QModelIndex PackageModel::parent( const QModelIndex &index ) const
{
    if (!index.isValid())
        return QModelIndex();

    int thisId = index.internalId();
    unsigned int thisParent = getParent( thisId );

    if ( thisParent == INVALID_PARENT )
    {
        return QModelIndex();
    }

    unsigned int internalId = makeId( thisParent, 0, INVALID_PARENT );
    return createIndex( thisParent, 0, internalId );
}

/*!
  \internal
  \reimp
  Return the number of rows which hang off the node \a parent
*/
int PackageModel::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
        return rootItems.count();

    // only root item nodes (those with invalid parents) have rows (for now)
    if ( parent.parent().isValid() )
        return 0;

    return ( parent.row() < rootItems.count() ) ? rootItems[ parent.row() ]->numberPackages() : 0;
}

int PackageModel::columnCount( const QModelIndex &parent ) const
{
    // parent nodes only have one column, their title
    if ( !parent.isValid() )
        return 1;
    return  2; // columnHeads.count();
}

#ifndef QT_NO_SXE
bool PackageModel::hasSensitiveDomains( const QString &domain )
{
    QStringList packageDomains = domain.split( QLatin1Char( ',' ), QString::SkipEmptyParts );

    foreach( QString d, packageDomains )
        if( sensitiveDomains.contains( d ) )
            return true;

    return false;
}
#endif

void PackageModel::serverStatusUpdated(const QString &status)
{
    emit serverStatus( activeServer + ": " + status ); 
}
