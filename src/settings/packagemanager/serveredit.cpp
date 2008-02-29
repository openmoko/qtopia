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

#include "serveredit.h"

#include <QTreeWidget>
#include <QTimer>
#include <QSettings>
#include <QtopiaApplication>

#ifdef QTOPIA_PHONE
#include <QKeyEvent>
#include <QMenu>
#include <QDesktopWidget>
#include <qsoftmenubar.h>
#endif

#include <qdebug.h>

class ServerItem : public QTreeWidgetItem
{
public:
    ServerItem( QTreeWidget *, const QString & );
    ~ServerItem();
    bool isOn() const;
    void setOn( bool );
    void setDuplicate( bool );
    void setName( const QString & );
    QString name() const;
    static QString id( const QString & );
    QString id() const;
private:
    bool isDuplicate;
};

////////////////////////////////////////////////////////////////////////
/////
///// ServerEdit implementation
/////
ServerEdit::ServerEdit( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
    , modified( false )
    , editedServer( 0 )
    , removeServerAction( 0 )
    , activateServerAction( 0 )
    , deactivateServerAction( 0 )
    , serversToRemove()
{
    setupUi( this );
    connect( servers, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(serverChanged(QTreeWidgetItem*,int)));

    connect( servers, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(serverSelected(QTreeWidgetItem*,QTreeWidgetItem*)));

#ifdef QTOPIA_PHONE
    newserver->hide();
    removeserver->hide();
    serverurl->show();
    servername->show();
    nameLabel->show();
    urlLabel->show();
    QMenu* contextMenu = QSoftMenuBar::menuFor( this );
    QAction *newServerAction = new QAction( QIcon( ":icon/new" ), tr( "New" ), this );
    removeServerAction = new QAction( QIcon( ":icon/trash" ), tr( "Remove" ), this );
    contextMenu->addAction( newServerAction );
    contextMenu->addAction( removeServerAction );

    connect( newServerAction, SIGNAL(triggered()),
            this, SLOT(addNewServer()) );
    connect( removeServerAction, SIGNAL(triggered()),
            this, SLOT(removeServer()) );

    if ( !Qtopia::mousePreferred() ) {
        activateServerAction = new QAction( tr( "Activate" ), this );
        connect(activateServerAction, SIGNAL(triggered()),
                this, SLOT(activateServer()));
        contextMenu->addAction( activateServerAction );

        deactivateServerAction = new QAction( tr( "Deactivate" ), this );
        connect(deactivateServerAction, SIGNAL(triggered()),
                this, SLOT(deactivateServer()));
        contextMenu->addAction( deactivateServerAction );
    }
#else
    connect( newserver, SIGNAL(clicked()),
            this, SLOT(addNewServer()) );
    connect( removeserver, SIGNAL(clicked()),
            this, SLOT(removeServer()) );
#endif
    QTimer::singleShot( 0, this, SLOT(init()) );
}

ServerEdit::~ServerEdit()
{
}

void ServerEdit::init()
{
    QSettings serverConf( "Trolltech", "PackageServers" );
    QStringList servConfList = serverConf.childGroups();
    ServerItem *servItem;
    for ( int srv = 0; srv < servConfList.count(); srv++ )
    {
        serverConf.beginGroup( servConfList[srv] );
        if ( serverConf.contains( "URL" ) &&
                serverConf.contains( "active" ))
        {
            servItem = new ServerItem( servers, servConfList[srv] );
            servItem->setText( 1, serverConf.value( "URL" ).toString() );
            servItem->setOn( serverConf.value( "active" ).toBool() );
        }
        serverConf.endGroup();
    }
    serverurl->setEnabled( true );
    servername->setEnabled( true );
    serverurl->setVisible( true );
    servername->setVisible( true );
#ifdef QTOPIA_PHONE
    showMaximized();
    /*QDesktopWidget *desktop = QApplication::desktop();
    setGeometry( y(), x(), desktop->width(), desktop->height() - ( 2 * x() ) );*/
    vboxLayout->update();
#endif
    modified = false;

    if ( servers->topLevelItemCount() > 0 )
        servers->setCurrentItem( servers->topLevelItem( 0 ) );

    updateIcons();
}

void ServerEdit::accept()
{
    if ( modified )
    {
        activeServers.clear();
        QSettings serverConf( "Trolltech", "PackageServers" );
        for ( int srv = 0; srv < servers->topLevelItemCount(); ++srv )
        {
            ServerItem *servItem = static_cast<ServerItem *>( servers->topLevelItem( srv ));
            serverConf.beginGroup( servItem->name() );
            serverConf.setValue( "URL", servItem->text( 1 ));
            serverConf.setValue( "active", servItem->isOn() );
            serverConf.endGroup();
            if ( servItem->isOn() )
                activeServers[ servItem->name() ] = servItem->text( 1 );
        }

        for ( int srv = 0; srv < serversToRemove.count(); ++srv )
        {
            serverConf.remove( serversToRemove.at( srv ) );
        }

        serversToRemove.clear();
    }
    QDialog::accept();
}

void ServerEdit::addNewServer()
{
    ServerItem* item;
    if (servername->text().isEmpty() || serverurl->text().isEmpty())
    {
        // create default name
        int nameNumber = 1;
        bool duplicate = true;
        QString potentialName;
        while (duplicate) {

            potentialName = tr("Package Feed");
            if (nameNumber > 1)
                potentialName += " " + QString::number(nameNumber);
            duplicate = false;
            for (int serverI = 0; serverI < servers->topLevelItemCount(); ++serverI) {

                ServerItem* existingItem = (ServerItem*)(servers->topLevelItem(serverI));
                if (ServerItem::id(potentialName) == existingItem->id())
                    duplicate = true;
            }
            ++nameNumber;
        }

        // create new item
        item = new ServerItem(servers, potentialName);
        item->setText(0, potentialName);
        item->setText(1, "http://");
    }
    else
    {
        // allows one-level undo
        item = new ServerItem(servers, servername->text());
        item->setText(1, serverurl->text());
    }

    if ( serversToRemove.contains( item->name() ) )
        serversToRemove.removeAll( item->name() );

    item->setOn(true);
    modified = true;

    updateIcons();
}

void ServerEdit::removeServer()
{
    if (servers->topLevelItemCount() > 0) {

        disconnect(servername, SIGNAL(textChanged(const QString&)),
                   this, SLOT(nameChanged(const QString&)));
        disconnect(serverurl, SIGNAL(textChanged(const QString&)),
                   this, SLOT(urlChanged(const QString&)));
        disconnect(servers, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this, SLOT(serverChanged(QTreeWidgetItem*,int)));
        disconnect(servers, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                   this, SLOT(serverSelected(QTreeWidgetItem*,QTreeWidgetItem*)));

        // Remove the selected items
        QList<QTreeWidgetItem*> selectedItems = servers->selectedItems();
        if ( selectedItems.count() != 0 ) {
            QList<QTreeWidgetItem*>::const_iterator cit;
            int index = -1;
            for ( cit = selectedItems.begin(); cit != selectedItems.end(); ++cit ) {
                index = servers->indexOfTopLevelItem( *cit );
                if ( index != -1 ) {
                    serversToRemove.append( static_cast<ServerItem*>(*cit)->name() );
                    delete servers->takeTopLevelItem( index );
                }
            }

            if (servers->topLevelItemCount() == 0) {
                editedServer = 0;
            } else  {
                if ( index != -1 ) {
                    editedServer = static_cast<ServerItem*>(
                                        servers->topLevelItem( index ));
                    if ( editedServer == 0 )
                        editedServer = static_cast<ServerItem*>(
                                        servers->topLevelItem( 0 ));
                } else {
                    editedServer = static_cast<ServerItem*>(
                                        servers->topLevelItem( 0 ));
                }
                servers->setItemSelected(editedServer, true);
            }

            connect(servers, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                    this, SLOT(serverChanged(QTreeWidgetItem*,int)));
            connect(servers, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                    this, SLOT(serverSelected(QTreeWidgetItem*,QTreeWidgetItem*)));

            servers->update();
        }
    }

    // disable edits if there are no more servers
    if (servers->topLevelItemCount() == 0)
    {
        editedServer = 0;
        servername->clear();
        serverurl->clear();
        servername->setEnabled(false);
        serverurl->setEnabled(false);
    }

    if ( editedServer )
        editServer( editedServer );
    modified = true;
}

void ServerEdit::updateIcons()
{
    QList<QTreeWidgetItem*> selectedItems = servers->selectedItems();
    bool serversSelected = selectedItems.count() > 0;

    removeServerAction->setEnabled( serversSelected );

    if ( !Qtopia::mousePreferred() && serversSelected ) {
        ServerItem* server = static_cast<ServerItem*>( servers->currentItem() );
        if ( server ) {
            activateServerAction->setVisible( !server->isOn() );
            deactivateServerAction->setVisible( server->isOn() );
        } else {
            activateServerAction->setVisible( false );
            deactivateServerAction->setVisible( false );
        }
    }
}

void ServerEdit::editServer( QTreeWidgetItem* server )
{
    if ( server == 0 )
        return;

    ServerItem* item = static_cast<ServerItem*>(server);
    if (servername->isEnabled()) {
        disconnect(servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
        disconnect(serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    } else {
        servername->setEnabled(true);
        serverurl->setEnabled(true);
    }

    servername->setText(item->text(0));
    serverurl->setText(item->text(1));

    editedServer = item;

    connect(servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
    connect(serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));

    updateIcons();
}

void ServerEdit::activateServer()
{
    ServerItem* server = static_cast<ServerItem*>( servers->currentItem() );
    if ( server )
        server->setOn( true );
}

void ServerEdit::deactivateServer()
{
    ServerItem* server = static_cast<ServerItem*>( servers->currentItem() );
    if ( server )
        server->setOn( false );
}

void ServerEdit::serverChanged( QTreeWidgetItem * item, int column )
{
    Q_UNUSED( column );

    if ( item == 0 )
        return;

    ServerItem* serverItem = static_cast<ServerItem*>(item);
    if ( serverItem == 0 )
        return;

    modified = true;

    updateIcons();
}

void ServerEdit::serverSelected( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
    Q_UNUSED( previous );

    editServer( current );
    updateIcons();
}

void ServerEdit::nameChanged( const QString &newText )
{
    if ( editedServer == NULL ) return;
    if ( editedServer->name() != newText )
    {
        editedServer->setName( newText );
        modified = true;
    }
}

void ServerEdit::urlChanged( const QString &newText )
{
    if ( editedServer == NULL ) return;
    if ( editedServer->text( 1 ) != newText )
    {
        editedServer->setText( 1, newText );
        modified = true;
    }
}

////////////////////////////////////////////////////////////////////////
/////
///// ServerItem implementation
/////
ServerItem::ServerItem( QTreeWidget* parent, const QString& serverText )
    : QTreeWidgetItem( parent )
    , isDuplicate( false )
{
    setText(0, serverText);
    setFlags(flags() | Qt::ItemIsUserCheckable);
}

ServerItem::~ServerItem()
{
}

inline bool ServerItem::isOn() const
{
    return data(0, Qt::CheckStateRole).toBool();
}

inline void ServerItem::setOn( bool on )
{
    setData(0, Qt::CheckStateRole, on);
}


void ServerItem::setDuplicate(bool duplicate)
{
    if (isDuplicate != duplicate)
    {
        isDuplicate = duplicate;
        treeWidget()->repaint();
    }
}

inline void ServerItem::setName( const QString& name )
{
    setText(0, name);
}

inline QString ServerItem::name() const
{
    return text(0);
}

/**
  \internal
  static
*/
QString ServerItem::id(const QString& name)
{
    QString workName = name;

    // replace spaces with underscores
    for (int characterI = 0; characterI < (int)workName.length(); ++characterI)
        if (workName[characterI] == ' ')
            workName[characterI] = '_';

    return workName;
}

QString ServerItem::id() const
{
    return id( name() );
}


