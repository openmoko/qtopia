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

#include "serveredit.h"

#include <QListWidget>
#include <QtopiaApplication>
#include <QTimer>
#include <QSettings>
#include <QMenu>
#include <qsoftmenubar.h>
#include <QLabel>
#include <QMessageBox>

class ServerItem : public QListWidgetItem
{
public:
    ServerItem( const QIcon &, QListWidget *, const QString &name="",
                const QString &url="http://" );
    ~ServerItem();
    void setName( const QString & );
    QString name() const;
    void setUrl( const QString & );
    QString url() const;
private:
    QString m_url;
};


////////////////////////////////////////////////////////////////////////
/////
///// ServerEdit implementation
/////
ServerEdit::ServerEdit( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
    , m_modified( false )
    , removeServerAction( 0 )
    , editServerAction( 0 )
    , serversToRemove()
{
    setupUi( this );

    QMenu* contextMenu = QSoftMenuBar::menuFor( this );

    QAction *newServerAction = new QAction( QIcon( ":icon/new" ), tr( "New...", "Adding a new server entry" ), this );
    connect( newServerAction, SIGNAL(triggered()),
            this, SLOT(addNewServer()) );
    contextMenu->addAction( newServerAction );

    editServerAction = new QAction( tr( "View/Edit..." ), this );
    connect( editServerAction, SIGNAL(triggered()),
            this, SLOT(editServer()) );
    contextMenu->addAction( editServerAction );
    
    removeServerAction = new QAction( QIcon( ":icon/trash" ), tr( "Remove" ), this );
    contextMenu->addAction( removeServerAction );
    connect( removeServerAction, SIGNAL(triggered()),
            this, SLOT(removeServer()) );

    connect( contextMenu, SIGNAL(aboutToShow()), 
             this, SLOT(contextMenuShow()) );
    connect( servers, SIGNAL(itemActivated(QListWidgetItem*)), 
             this, SLOT(editServer()) );
    showMaximized();
    
    QTimer::singleShot( 0, this, SLOT(init()) );
}

ServerEdit::~ServerEdit()
{
}

void ServerEdit::init()
{
    QString serversFile = Qtopia::applicationFileName( "packagemanager", "ServersList.conf" );
    QSettings *serversConf;
    if ( !QFile::exists( serversFile ) )
        serversConf = new QSettings( "Trolltech", "PackageManager" );
    else
        serversConf = new QSettings( serversFile, QSettings::NativeFormat );

    QStringList servConfList = serversConf->childGroups();
    ServerItem *servItem;
    for ( int srv = 0; srv < servConfList.count(); srv++ )
    {
        serversConf->beginGroup( servConfList[srv] );
        if ( serversConf->contains( "URL" ) )
        {
            servItem = new ServerItem(QIcon( ":icon/irreceive" ), servers, servConfList[srv] );
            servItem->setUrl( serversConf->value( "URL" ).toString() );
        }
        serversConf->endGroup();
    }
    delete serversConf;
    
    if ( servers->count() > 0 )
        servers->setCurrentRow(0);

}

void ServerEdit::accept()
{
    if ( m_modified )
    {
        QString filename = Qtopia::applicationFileName( "packagemanager", "ServersList.conf" );
        QSettings serverConf( filename, QSettings::NativeFormat );
        for ( int srv = 0; srv < servers->count(); ++srv )
        {
            ServerItem *servItem = static_cast<ServerItem *>( servers->item( srv ));
            serverConf.beginGroup( servItem->name() );
            serverConf.setValue( "URL", servItem->url() );
            serverConf.endGroup();
        }
        
        for ( int srv = 0; srv < serversToRemove.count(); ++srv )
            serverConf.remove( serversToRemove.at( srv ) );
    }
   QDialog::accept();
}

void ServerEdit::addNewServer()
{
    ServerEditor serverEditor( ServerEditor::New, this, "", "http://" );
    serverEditor.showMaximized();
    if ( serverEditor.exec() == QDialog::Accepted )
    {
        if ( serverEditor.wasModified() )
        {
            ServerItem* server = new ServerItem(QIcon( ":icon/irreceive" ), servers, serverEditor.name(), serverEditor.url() );
            if ( serversToRemove.contains( server->name() ) )
                serversToRemove.removeAll( server->name() );
            m_modified = true;
        }
    }
}

void ServerEdit::removeServer()
{
    ServerItem *server = static_cast<ServerItem*>( servers->currentItem() );
    if ( server == 0 )
        return;
    
    serversToRemove.append( server->name() );
    delete server;
    m_modified = true;
}

void ServerEdit::editServer()
{
    ServerItem *server = static_cast<ServerItem*>( servers->currentItem() );
    if ( server == 0 )
        return;
    
    ServerEditor *serverEditor;

    if ( !permanentServers.contains(server->name()) )
        serverEditor = new ServerEditor( ServerEditor::ViewEdit, this, server->name(), server->url() );
    else
        serverEditor = new ServerEditor( ServerEditor::ViewOnly, this,  server->name(), server->url() );
     
    serverEditor->showMaximized();
    if ( serverEditor->exec() == QDialog::Accepted && serverEditor->wasModified() )
    {
        if ( serverEditor->name() != server->name() )
            serversToRemove.append( server->name() );
        server->setName( serverEditor->name() );
        server->setUrl( serverEditor->url() );
        m_modified = true;
    }
    delete serverEditor;
}

void ServerEdit::contextMenuShow()
{
    ServerItem *server = static_cast<ServerItem *>( servers->currentItem() );
    if ( server  )
    {
        if ( permanentServers.contains(server->name()) )
        {
            editServerAction->setText( tr("View") );
            removeServerAction->setVisible( false );
        }
        else
        {
            editServerAction->setText( tr("View/Edit...") );
            removeServerAction->setVisible( true );
        }
    }
    else
    {
        removeServerAction->setVisible( false );
    }
}       

QHash<QString,QString> ServerEdit::serverList() const
{
    QHash<QString, QString> serverList;
    ServerItem *servItem;
    for ( int srv = 0; srv < servers->count(); ++srv )
    {
        servItem = static_cast<ServerItem *>(servers->item( srv ));
        serverList[ servItem->name() ] = servItem->url();
    }
    return serverList;
}


////////////////////////////////////////////////////////////////////////
/////
///// ServerItem implementation
/////
ServerItem::ServerItem( const QIcon &icon, QListWidget* parent, const QString& name, 
                        const QString& url )
    : QListWidgetItem( icon, name, parent )
{
    setName( name );
    setUrl( url );    
}

ServerItem::~ServerItem()
{
}

inline void ServerItem::setName( const QString& name )
{
    setText( name );    
}

inline QString ServerItem::name() const
{
    return text();
}

inline void ServerItem::setUrl( const QString &url )
{
    m_url = url ; 
}

inline QString ServerItem::url() const
{   
   return m_url;     
}

////////////////////////////////////////////////////////////////////////
/////
///// ServerEditor implementation
/////

ServerEditor::ServerEditor( Mode mode, ServerEdit *parent, 
                            const QString &name, const QString &url )
    :QDialog( parent ), m_mode( mode ),
     m_parent( parent ), m_modified( false ) 
{
    m_nameLabel = new QLabel( "Name:", this );
    m_nameLineEdit = new QLineEdit( name, this );
    
    m_urlLabel = new QLabel( "URL:", this );
    m_urlTextEdit = new QTextEdit( url, this );
    
    QGridLayout *layout = new QGridLayout( this );
    layout->addWidget( m_nameLabel, 0, 0 );
    layout->addWidget( m_nameLineEdit, 0, 1 );   
    layout->addWidget( m_urlLabel, 1, 0, Qt::AlignTop );
    layout->addWidget( m_urlTextEdit, 1, 1 );
    
    setLayout( layout );
    m_initialName = name;
    m_initialUrl = url; 

    if ( m_mode == ServerEditor::ViewOnly ) 
    {
        m_urlTextEdit->setReadOnly( true );
        m_nameLineEdit->setReadOnly( true );
    }
}

void ServerEditor::accept() 
{
    if ( m_nameLineEdit->text().isEmpty()  ) 
    {
        if ( m_mode == ServerEditor::New && m_urlTextEdit->toPlainText() == m_initialUrl )
        {
            QDialog::accept();     
            return;
        }
        
        QMessageBox::warning( this, tr( "Warning" ), tr( "Name field is empty" ) );
        const char * dummyStr = QT_TRANSLATE_NOOP("ServerEditor", "Cancel editing?");
        Q_UNUSED( dummyStr );
    }
    else if ( m_urlTextEdit->toPlainText().isEmpty() )
    {
        QMessageBox::warning( this, tr( "Warning" ), tr( "URL field is empty" ) );
    }
    else if ( m_nameLineEdit->text() != m_initialName 
              && m_parent->serverList().contains(m_nameLineEdit->text()) )
    {
        QMessageBox::warning( this, tr( "Warning" ), tr( "Server already exists" ) );
    }
    else
    {
        if ( m_initialName.isEmpty() || m_urlTextEdit->toPlainText() != m_initialUrl 
             || m_nameLineEdit->text() != m_initialName )
        {
            m_modified = true;
        } 
        QDialog::accept();
    }
}
