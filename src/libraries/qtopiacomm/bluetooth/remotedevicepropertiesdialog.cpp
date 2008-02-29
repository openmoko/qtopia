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

#include <qtopiacomm/private/remotedevicepropertiesdialog_p.h>

#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qtopialog.h>

#include <QtopiaApplication>
#include <QWaitWidget>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSoftMenuBar>
#include <QAction>
#include <QMenu>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QList>
#include <QScrollArea>
#include <QScrollBar>


/*!
    \internal
    \class RemoteDevicePropertiesDialog

    A dialog that shows information about a remote device. Shows a main
    screen with the main information about a device, and provides other
    information (services, etc.) that is accessible through the context menu.
 */

const QString RemoteDevicePropertiesDialog::SERVICE_ERROR_MSG =
        tr( "<P>Unable to look up services for this device" );

/*!
    Constructor.
    Assumes given remote and local devices are valid.
 */
RemoteDevicePropertiesDialog::RemoteDevicePropertiesDialog(
            const QBluetoothAddress &localAddr,
            QWidget *parent,
            Qt::WFlags flags )
    : QDialog( parent, flags ),
      m_remote( QBluetoothRemoteDevice(QBluetoothAddress()) ),
      m_local( new QBluetoothLocalDevice(localAddr, this) ),
      m_waitWidget( new QWaitWidget(this) ),
      m_servicesDialog( 0 ),
      m_servicesText( 0 )
{
    if ( !m_local->isValid() ) {
        qLog(Bluetooth) << "Invalid local device, won't show any device info";
        return;
    }

    // set alias when dialog is closed
    connect( this, SIGNAL(accepted()), SLOT(setDeviceAlias()) );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    contextMenu->addAction( QIcon( ":icon/view" ), tr( "View services" ),
            this, SLOT(showServices()) );
#endif

    // service searching
    connect( &m_sdap, SIGNAL( searchComplete( const QSDAPSearchResult& ) ),
             SLOT( foundServices( const QSDAPSearchResult & ) ) );

    // wait widget
    m_waitWidget->setText( tr( "Finding services..." ) );
    m_waitWidget->setCancelEnabled( true );
    connect( m_waitWidget, SIGNAL( cancelled() ),
             &m_sdap, SLOT( cancelSearch() ) );

    initMainInfo();

    setWindowTitle( tr( "Device information" ) );
}

RemoteDevicePropertiesDialog::~RemoteDevicePropertiesDialog()
{
}

void RemoteDevicePropertiesDialog::setRemoteDevice( const QBluetoothRemoteDevice &remote, const QPixmap &icon )
{
    m_remote = remote;

    // in case remote device details have just changed
    m_local->updateRemoteDevice( m_remote );

    // find device alias
    QBluetoothReply<QString> reply = m_local->remoteAlias( m_remote.address() );
    if ( reply.isError() )
        m_prevAlias = QString();
    else
        m_prevAlias = reply.value();

    if ( m_servicesText )
        m_servicesText->clear();

    resetMainInfo( m_prevAlias.isEmpty() ? m_remote.name() : m_prevAlias, icon );
}

void RemoteDevicePropertiesDialog::initMainInfo()
{
    m_icon = new QLabel;
    m_title = new QLineEdit;
    QFont f = m_title->font();
    f.setBold( true );
    m_title->setFont( f );
    m_title->setReadOnly( true );
    m_title->setFrame( false );

    // heading (icon + nickname)
    m_headingLayout = new QHBoxLayout;
    m_headingLayout->setMargin( 0 );
    m_headingLayout->setSpacing( 4 );
    m_headingLayout->addWidget( m_icon, 0, Qt::AlignRight );
    m_headingLayout->addWidget( m_title, 0 );

    // grid of device details
    QGridLayout *grid = new QGridLayout;
    grid->setMargin( 0 );
    grid->setSpacing( 1 );

    // add attribute labels
    QList<QLabel *> labels;
    labels.append( new QLabel( tr("Nickname:") ) );
    labels.append( new QLabel( tr("Address:") ) );
    labels.append( new QLabel( tr("Type:") ) );
    labels.append( new QLabel( tr("Sub-type:") ) );
    labels.append( new QLabel( tr("Version:") ) );
    labels.append( new QLabel( tr("Vendor:") ) );
    labels.append( new QLabel( tr("Company:") ) );

    for ( int i=0; i<labels.size(); i++ )
        grid->addWidget( labels[i], i, 0, 1, 1, Qt::AlignRight );

    // add attribute value containers
    QLineEdit *lineEdit;
    for ( int i=0; i<labels.size(); i++ ) {
        lineEdit = new QLineEdit;
        lineEdit->setReadOnly( true );

        grid->addWidget( lineEdit, i, 1 );
        m_devAttrs.append( lineEdit );
    }

    // set attributes for the nickname line edit
    m_aliasLineEdit = m_devAttrs[0];
    m_aliasLineEdit->setReadOnly( false );
    connect( m_aliasLineEdit, SIGNAL(editingFinished()),
             SLOT(checkAliasEdit()) );
    f = m_aliasLineEdit->font();
    f.setBold( true );
    m_aliasLineEdit->setFont( f );

    // main layout, contains all the information widgets
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin( 6 );
    mainLayout->setSpacing( 4 );
    mainLayout->addLayout( m_headingLayout );
    mainLayout->addLayout( grid );
    mainLayout->addStretch();

    // make info scrollable by adding everything into a child widget and
    // making the child scrollable
    QWidget *baseWidget = new QWidget;
    baseWidget->setLayout( mainLayout );
    QScrollArea *scrollArea = new QScrollArea;
    //scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollArea->setWidgetResizable( true ); // otherwise widget won't expand
    scrollArea->setWidget( baseWidget );

    // put everything into this dialog
    QVBoxLayout *baseLayout = new QVBoxLayout;
    baseLayout->addWidget( scrollArea );
    setLayout( baseLayout );
}

void RemoteDevicePropertiesDialog::resetMainInfo( const QString &displayName, const QPixmap &pixmap )
{
    // set title
    m_title->setText( ( m_remote.name().isEmpty() ) ?
            m_remote.address().toString() : m_remote.name() );
    m_title->home( false );

    // set icon
    if ( pixmap.isNull() ) {
        m_icon->hide();
        m_headingLayout->setAlignment( m_title, Qt::AlignHCenter );
    } else {
        m_headingLayout->setAlignment( m_title, Qt::AlignLeft );
        m_icon->setPixmap( pixmap );
        m_icon->show();
    }

    m_title->setFocus(); // give initial focus to title

    m_devAttrs[0]->setText( displayName );
    m_devAttrs[1]->setText( m_remote.address().toString() );
    m_devAttrs[2]->setText( m_remote.deviceMajorAsString() );
    m_devAttrs[3]->setText( m_remote.deviceMinorAsString() );
    m_devAttrs[4]->setText( m_remote.version() );
    m_devAttrs[5]->setText( m_remote.manufacturer() );
    m_devAttrs[6]->setText( m_remote.company() );

    // make sure cursor is at start, so the start of the text is displayed
    for ( int i=0; i<m_devAttrs.size(); i++ )
        m_devAttrs[i]->home( false );
}

void RemoteDevicePropertiesDialog::showServices()
{
    qLog( Bluetooth ) << "RemoteDevicePropertiesDialog::showServices";

    QBluetoothAddress addr = m_remote.address();
    if ( !addr.valid() )
        return;

    // search by L2CAP to avoid having to do a full slow "records" sdp search
    if ( !m_sdap.searchServices( addr, *m_local, QSDPUUID::L2cap ) ) {
        QMessageBox::warning( this, tr( "Service Error" ), SERVICE_ERROR_MSG );
    } else {
        qLog(Bluetooth) << "Starting service search...";
        m_waitWidget->show();
    }
}


void RemoteDevicePropertiesDialog::foundServices( const QSDAPSearchResult &result )
{
    qLog( Bluetooth ) << "RemoteDevicePropertiesDialog::foundServices";

    if ( result.errorOccurred() ) {
        qLog( Bluetooth ) << "Error in finding services:" << result.error();
        m_waitWidget->hide();
        QMessageBox::warning( this, tr( "Service Error" ), SERVICE_ERROR_MSG );
        return;
    }

    if ( !m_servicesDialog ) {
        m_servicesDialog = new QDialog( this );
        m_servicesDialog->setModal( true );
        m_servicesDialog->setWindowTitle( tr( "Supported services" ) );
        QSoftMenuBar::setCancelEnabled( m_servicesDialog, true );

        m_servicesText = new QTextEdit;
        m_servicesText->setReadOnly( true );
        m_servicesText->setWordWrapMode( QTextOption::WordWrap );

        // textedit size doesn't seem quite right (can't see scrollbars)
        // if not added within a layout
        QVBoxLayout *layout = new QVBoxLayout( m_servicesDialog );
        layout->addWidget( m_servicesText );

#ifdef QTOPIA_KEYPAD_NAVIGATION
        QSoftMenuBar::setLabel( m_servicesText, Qt::Key_Select, QSoftMenuBar::NoLabel );
#endif
    } else {
        // clear the list of services
        if ( m_servicesText )
            m_servicesText->clear();
    }

    const QList<QSDPService> services = result.services();
    for ( int i=0; i<services.count(); i++ ) {
        if ( !services[i].serviceName().isEmpty() ) // ignore unnamed services
            m_servicesText->append( services[i].serviceName() );
    }

    m_servicesDialog->showMaximized();

    // scroll to beginning of list (doesn't work until after show() call)
    m_servicesText->verticalScrollBar()->setValue( 0 );

    m_servicesDialog->raise();
    m_servicesDialog->activateWindow();


    m_waitWidget->hide();
}

void RemoteDevicePropertiesDialog::checkAliasEdit()
{
    if ( m_aliasLineEdit ) {
        if ( m_aliasLineEdit->text().trimmed().isEmpty() )
            m_aliasLineEdit->setText( m_remote.name() );
    }
}

void RemoteDevicePropertiesDialog::setDeviceAlias()
{
    qLog(Bluetooth) << "RemoteDevicePropertiesDialog::setDeviceAlias";

    QString aliasEntry = m_aliasLineEdit->text();
    if ( aliasEntry.trimmed().isEmpty() || aliasEntry == m_remote.name() ) {
        m_local->removeRemoteAlias( m_remote.address() );
        return;
    }
    if ( aliasEntry == m_prevAlias )
        return;

    qLog(Bluetooth) << "Changing alias";
    m_local->setRemoteAlias( m_remote.address(), m_aliasLineEdit->text() );
}
