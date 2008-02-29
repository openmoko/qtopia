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
#include <qtopia/private/qdocumentselectorsocketserver_p.h>
#include <QLineEdit>
#include <QComboBox>
#include <QStorageDeviceSelector>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileSystem>
#include <QtopiaApplication>
#include <QDocumentSelector>
#include <QContentSortCriteria>

Q_IMPLEMENT_USER_METATYPE_ENUM(QIODevice::OpenMode);

class NewDocumentDialog : public QDialog
{
public:
    NewDocumentDialog( QWidget *parent = 0 );

    void setName( const QString &name );
    QString name() const;

    void setTypes( const QStringList &types );
    QString type() const;

    QString location() const;

private:
    QLineEdit *m_nameEdit;
    QComboBox *m_typeEdit;
    QStorageDeviceSelector *m_locationEdit;
};

NewDocumentDialog::NewDocumentDialog( QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( QDocumentSelectorServer::tr( "Save As..." ) );

    QGridLayout *layout = new QGridLayout( this );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Name" ), this ), 0, 0 );
    layout->addWidget( m_nameEdit = new QLineEdit( this ), 0, 1 );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Location" ), this ), 1, 0 );
    layout->addWidget( m_locationEdit = new QStorageDeviceSelector( this ), 1, 1 );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Type" ), this ), 2, 0 );
    layout->addWidget( m_typeEdit = new QComboBox( this ), 2, 1 );
}

void NewDocumentDialog::setName( const QString &name )
{
    m_nameEdit->setText( name );
}

QString NewDocumentDialog::name() const
{
    return m_nameEdit->text();
}

void NewDocumentDialog::setTypes( const QStringList &types )
{
    m_typeEdit->clear();

    m_typeEdit->addItems( types );
}

QString NewDocumentDialog::type() const
{
    return m_typeEdit->currentText();
}

QString NewDocumentDialog::location() const
{
    return m_locationEdit->installationPath();
}

class SaveDocumentDialog : public QDialog
{
public:
    SaveDocumentDialog( QWidget *parent = 0 );

    void setContent( const QContent &content );

private:
    QLabel *m_nameLabel;
    QLabel *m_typeLabel;
    QLabel *m_locationLabel;
};

SaveDocumentDialog::SaveDocumentDialog( QWidget *parent )
    : QDialog( parent )
{
    QGridLayout *layout = new QGridLayout( this );

    setWindowTitle( QDocumentSelectorServer::tr( "Save..." ) );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Name" ), this ), 0, 0 );
    layout->addWidget( m_nameLabel = new QLabel( this ), 0, 1 );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Location" ), this ), 1, 0 );
    layout->addWidget( m_locationLabel = new QLabel( this ), 1, 1 );

    layout->addWidget( new QLabel( QDocumentSelectorServer::tr( "Type" ), this ), 2, 0 );
    layout->addWidget( m_typeLabel = new QLabel( this ), 2, 1 );
}

void SaveDocumentDialog::setContent( const QContent &content )
{
    m_nameLabel->setText( content.name() );
    m_typeLabel->setText( content.type() );
    m_locationLabel->setText( QFileSystem::fromFileName( content.fileName() ).name() );
}

QDocumentSelectorServer::QDocumentSelectorServer( QObject *parent )
    : QDocumentServerHost( "QDocumentSelectorServer", parent )
    , m_selector( 0 )
    , m_selectorDialog( 0 )
    , m_newDocumentDialog( 0 )
    , m_saveDocumentDialog( 0 )
    , m_openMode( QIODevice::ReadOnly )
{
}

QDocumentSelectorServer::~QDocumentSelectorServer()
{
    delete m_selectorDialog;
    delete m_newDocumentDialog;
    delete m_saveDocumentDialog;
}

QDocumentServerMessage QDocumentSelectorServer::invokeMethod( const QDocumentServerMessage &message )
{
    Q_UNUSED( message );

    Q_ASSERT( false );

    return QDocumentServerMessage();
}

void QDocumentSelectorServer::invokeSlot( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "openDocument(QContentFilter,QContentSortCriteria)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        if( !m_selector )
        {
            m_selectorDialog = new QDialog;

            m_selectorDialog->setWindowTitle( tr( "Open..." ) );

            QVBoxLayout *layout = new QVBoxLayout( m_selectorDialog );

            layout->addWidget( m_selector = new QDocumentSelector( m_selectorDialog ) );

            connect( m_selector, SIGNAL(documentSelected(QContent)), this, SLOT(documentSelected(QContent)) );
            connect( m_selector, SIGNAL(documentSelected(QContent)), m_selectorDialog, SLOT(hide()) );
            connect( m_selectorDialog, SIGNAL(rejected()), this, SLOT(rejected()) );

            QtopiaApplication::setMenuLike( m_selectorDialog, true );
        }

        m_selector->setFilter( qvariant_cast< QContentFilter >( arguments.at( 0 ) ) );
        m_selector->setSortCriteria( qvariant_cast< QContentSortCriteria >( arguments.at( 1 ).toInt() ) );

        m_openMode = QIODevice::ReadOnly;

        QtopiaApplication::showDialog( m_selectorDialog );
    }
    else if( signature == "newDocument(QString,QStringList)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( !m_newDocumentDialog )
        {
            m_newDocumentDialog = new NewDocumentDialog;

            connect( m_newDocumentDialog, SIGNAL(accepted()), this, SLOT(newDocumentAccepted()) );
            connect( m_newDocumentDialog, SIGNAL(rejected()), this, SLOT(rejected()) );
        }

        m_openMode = QIODevice::WriteOnly;

        m_newDocumentDialog->setName( qvariant_cast< QString >( arguments.at( 0 ) ) );
        m_newDocumentDialog->setTypes( qvariant_cast< QStringList >( arguments.at( 1 ) ) );

        QtopiaApplication::showDialog( m_newDocumentDialog );
    }
    else if( signature == "saveDocument()" )
    {
        Q_ASSERT( arguments.count() == 0 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        if( !m_saveDocumentDialog )
        {
            m_saveDocumentDialog = new SaveDocumentDialog;

            connect( m_saveDocumentDialog, SIGNAL(accepted()), this, SLOT(saveDocumentAccepted()) );
            connect( m_saveDocumentDialog, SIGNAL(rejected()), this, SLOT(rejected()) );
        }

        m_openMode = QIODevice::WriteOnly;

        m_saveDocumentDialog->setContent( m_selectedDocument );

        QtopiaApplication::showDialog( m_saveDocumentDialog );
    }
    else if( signature == "close()" )
    {
        Q_ASSERT( arguments.count() == 0 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        m_selectedDocument == QContent();
        m_openMode = QIODevice::NotOpen;
    }
}

void QDocumentSelectorServer::rejected()
{
    emitSignalWithArgumentList( "cancelled()", QVariantList() );
}

void QDocumentSelectorServer::documentSelected( const QContent &document )
{
    m_selectedDocument = document;

    QFile file( document.fileName() );

    if( file.open( m_openMode ) )
    {
        emitSignalWithArgumentList( "documentOpened(QContent,QIODevice::OpenMode)",
                QVariantList() << QVariant::fromValue( document ) << QVariant::fromValue( m_openMode ),
                QList< QUnixSocketRights >() << QUnixSocketRights( file.handle() ) );

        file.close();
    }
    else
    {
        qWarning() << file.errorString();
    }
}

void QDocumentSelectorServer::newDocumentAccepted()
{
    QContent document;

    document.setName( m_newDocumentDialog->name() );
    document.setMedia( m_newDocumentDialog->location() );
    document.setType( m_newDocumentDialog->type() );

    QIODevice *device = document.open( QIODevice::WriteOnly );

    if( device )
    {
        device->close();

        delete device;

        document.commit();

        documentSelected( document );
    }
    else
        rejected();
}

void QDocumentSelectorServer::saveDocumentAccepted()
{
    documentSelected( m_selectedDocument );
}

QDocumentSelectorSocketServer::QDocumentSelectorSocketServer( QObject *parent )
    : QUnixSocketServer( parent )
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QDocumentSelectorServer" )).toLocal8Bit();

    listen( socketPath );
}

void QDocumentSelectorSocketServer::incomingConnection( int socketDescriptor )
{
    QDocumentSelectorServer *server = new QDocumentSelectorServer( this );

    server->setSocketDescriptor( socketDescriptor );
}
