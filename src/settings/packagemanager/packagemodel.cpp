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
    QFile file( Qtopia::qtopiaDir() + QLatin1String( "/etc/default/Trolltech/PackageServers.conf" ) );

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
    connect( networked, SIGNAL(packageInstalled(const InstallControl::PackageInfo &,bool)),
            installed, SLOT(addPackage(const InstallControl::PackageInfo &)) );
    connect( networked, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&,int,int)),
            this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&,int,int)));
    connect( networked, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
            this, SIGNAL(rowsRemoved(const QModelIndex&,int,int)));
    connect( networked, SIGNAL(packageInstalled(const InstallControl::PackageInfo &,bool)),
            this, SLOT(packageInstalled(const InstallControl::PackageInfo &,bool)) );
    connect( networked, SIGNAL(packageMessage(const QString &)), 
            this, SIGNAL(infoHtml(const QString &)) );
    connect( installed, SIGNAL(packageInstalled(const InstallControl::PackageInfo &,bool)),
            this, SLOT(packageInstalled(const InstallControl::PackageInfo &,bool)) );

    // can only have a max of 4 columns, if more are needed, change the
    // macros used for PackageModel::index(...) below
    // columnHeads << "Name" << "Size";
}

PackageModel::~PackageModel()
{
    if ( storage )
        delete storage;
}

void PackageModel::populateLists()
{
    QSettings serverConf( "Trolltech", "PackageServers" );
    serverConf.clear();
    QStringList servConfList = serverConf.childGroups();
    QString srvName;
    for ( int srv = 0; srv < servConfList.count(); srv++ )
    {
        srvName = servConfList[srv];

#ifndef QT_NO_SXE
        if( srvName == QLatin1String( "Sensitive" ) ||
                srvName == QLatin1String( "Configuration" ))
            continue;
#endif

        serverConf.beginGroup( srvName );
        if ( serverConf.contains( "active" ) &&
                serverConf.contains( "URL" ))
        {
            //qDebug() << "reading package server - name" << srvName << "url" << activeServers[srvName];
            if ( serverConf.value( "active" ).toBool() )
                activeServers[ srvName ] = serverConf.value( "URL" ).toString();
        }
        serverConf.endGroup();
    }
    QStorageMetaInfo *s = storageInfo();
    connect( s, SIGNAL(disksChanged()),
            this, SLOT(publishTargets()) );
    connect( this, SIGNAL(targetsUpdated(const QStringList &)),
            installed, SLOT(reloadInstalledLocations(const QStringList &)) );

    QHash<QString,QString>::const_iterator cit = activeServers.begin();
    if ( cit != activeServers.end() ) {
        NetworkPackageController* network =
             qobject_cast<NetworkPackageController*>(networked);
        if ( network != 0 )
            network->setNetworkServer( cit.value() );
    }

    if ( !QDir( Qtopia::packagePath() ).exists() )
        QDir::root().mkpath( Qtopia::packagePath() );
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

void PackageModel::setServers( const QHash<QString,QString> &updatedServers )
{
    activeServers = updatedServers;

    NetworkPackageController* network = qobject_cast<NetworkPackageController*>(networked);
    if ( network == 0 )
        return;

    QHash<QString,QString>::const_iterator cit = activeServers.begin();
    if ( cit != activeServers.end() ) {
        if ( network->networkServer() != cit.value() ) {
            network->setNetworkServer( cit.value() );
        }
    } else {
        network->setNetworkServer( QString() );
    }

    emit serversUpdated( getServers() );
}

void PackageModel::setServer( const QString& server )
{
    QString serverUrl = activeServers[ server ];
    if ( serverUrl != QString() ) {
        NetworkPackageController* network =
             qobject_cast<NetworkPackageController*>(networked);
        if ( network != 0 )
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
    c->install( item.row() );
    QString html;
    if ( c == installed )
        html = tr( "Uninstalling %1 ..." ).arg( dataItem );
    else
    {
        html = tr( "Installing %1 ..." ).arg( dataItem );
        emit infoHtml( html );
    }
}

void PackageModel::reenableItem( const QModelIndex &item )
{
    QModelIndex parent = item.parent();
    if ( !parent.isValid() )
        return;

    QString html;
    if ( !qobject_cast<InstalledPackageController *>(installed)->reenable( item.row() ) )
    {
        html = tr( "Reenable unsuccessful" );
        emit infoHtml( html );
    }
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

/*!
  \internal
  Receive notifications that an item has been selected as current in the
  view.  Respond by sending text by emitting the infoHtml() signal
*/
void PackageModel::sendUpdatedText( const QModelIndex &item )
{
    QString html = data( item, Qt::WhatsThisRole ).toString();
    QModelIndex parent = item.parent();
    if ( parent.isValid() )
    {
        AbstractPackageController *c = rootItems[parent.row()];
        if ( c->inherits( "NetworkPackageController" ))
        {
            // html is actually the domain
            emit domainUpdate( html );
            return;
        }
    }
    
    emit infoHtml( html );
}

void PackageModel::userTargetChoice( const QString &t )
{
    // qDebug() << "Setting new install destination to" << t;
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

void PackageModel::packageInstalled( const InstallControl::PackageInfo &pkg, bool success )
{
    
    InstalledPackageController *installed = qobject_cast<InstalledPackageController *>( sender() );
    QString html = tr( "%1 <b>%2</b> %3", "%1 = package name, %2 = successfully/unsuccessfully, %3 = installed/uninstalled" )
                    .arg( pkg.name )
                    .arg( success ? tr("successfully") : tr("unsuccessfully") )
                    .arg( installed ? tr("uninstalled") : tr("installed") ); 
    emit infoHtml( html );
}

void PackageModel::publishTargets()
{
    QFileSystemFilter fsf;
    fsf.applications = QFileSystemFilter::Set;
    QList<QFileSystem*> fl = storageInfo()->fileSystems( &fsf );
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

Q_GLOBAL_STATIC(QStorageMetaInfo,theStorageInfo);

QStorageMetaInfo *PackageModel::storageInfo() const
{
    return theStorageInfo();
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
        // qDebug() << "DATA - role:" << role << "parent row:" << parent.row() << "id:" << hex <<
        //      ((unsigned int )(index.internalId()))
        //      << "parent id" << ((unsigned int)(parent.internalId()));
        return c->data( index.row(), index.column(), role );
    }
    else
    {
        if ( index.row() < rootItems.count() )
        {
            // qDebug() << "DATA - ROOT - role:" << role << "row:" << index.row();
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
        // qDebug() << "INDEX - ROOT - row:" << row << "col:" << column << "index:" << hex << internalId;
        return createIndex( row, 0, internalId );
    }
    else
    {
        unsigned int parentId = getRow( parent.internalId() );
        // qDebug() << "INDEX - row:" << row << "col:" << column
        //     << "parent row:" << parentId << "index:" << hex << makeId( row, column, parentId );
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
        // qDebug() << "PARENT - NULL - row" << index.row() << "col:" << index.column() << "id:" << hex << (unsigned int)thisId;
        return QModelIndex();
    }

    unsigned int internalId = makeId( thisParent, 0, INVALID_PARENT );
    // qDebug() << "PARENT - row:" << index.row() << "col:" << index.column() << "id:" << hex << thisId << "-> parent index:" << internalId;
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
