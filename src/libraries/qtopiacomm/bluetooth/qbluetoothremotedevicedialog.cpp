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

#include <qtopiacomm/private/qbluetoothremotedevicedialog_p.h>
#include <qtopiacomm/private/remotedevicepropertiesdialog_p.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>

#include <qbluetoothsdpquery.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusinterface.h>
#include <qtdbus/qdbusreply.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <QtopiaApplication>

#include <QAction>
#include <QActionEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPixmap>
#include <QIcon>
#include <QWaitWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QShowEvent>
#include <QSoftMenuBar>
#include <QMenu>
#include <QFont>
#include <QSettings>
#include <QTimer>


/*!
    \internal
    \class DevicesTableController
    Controls a QTableWidget that displays devices and their pairing status.
 */

const Qt::ItemFlags DevicesTableController::ITEM_FLAGS =
        Qt::ItemIsSelectable | Qt::ItemIsEnabled;

DevicesTableController::DevicesTableController(
        QBluetoothRemoteDeviceDialogPrivate *parent )
    : QObject( parent ),
      m_parent( parent ),
      m_devicesTable( new QTableWidget( 0, 2, parent ) ),
      m_lastSelectedRow( -1 ),
      m_discoveryButton( 0 )
{
    m_devicesTable->setFocusPolicy( Qt::StrongFocus );
    m_devicesTable->setRowCount( 0 );
    m_devicesTable->setColumnCount( 2 );
    m_devicesTable->verticalHeader()->hide();
    m_devicesTable->setSelectionMode( QAbstractItemView::SingleSelection );

    // selecting cells
    connect( m_devicesTable, SIGNAL(cellActivated(int,int)),
             SLOT(cellActivated(int,int)) );
    connect( m_devicesTable, SIGNAL(currentCellChanged(int,int,int,int)),
             SLOT(currentCellChanged(int,int,int,int)) );
}

DevicesTableController::~DevicesTableController()
{
}

void DevicesTableController::addDevice( const QBluetoothRemoteDevice &remote, const QBluetoothLocalDevice &local )
{
    if ( m_displayedDevices.contains( remote ) ) {
        qLog(Bluetooth) << "addDevice:" << remote.address().toString()
                << "already in devices table, not adding again.";
        return;
    }

    qLog(Bluetooth) << "Displaying device" << remote.name() << "with major"
            << remote.deviceMajor() << "and classes" << remote.serviceClasses();

    m_displayedDevices.append( remote );

    // grow table
    int row = m_devicesTable->rowCount();
    m_devicesTable->setRowCount( row + 1 );

    // first column, device type icon + device name
    QTableWidgetItem *newItem = new QTableWidgetItem();
    newItem->setFlags( ITEM_FLAGS );
    m_devicesTable->setItem( row, 0, newItem );

    // second column, pairing status
    newItem = new QTableWidgetItem("");
    newItem->setFlags( ITEM_FLAGS );
    m_devicesTable->setItem( row, 1, newItem );

    // actually fill in the name, pairing status, etc.
    refreshDevice( row, local );

    if ( row == 0 )
        m_devicesTable->setCurrentCell( row, 0 );
}

void DevicesTableController::refreshDevice( int row, const QBluetoothLocalDevice &local, const QString &updatedName )
{
    if ( row < 0 || row >= deviceCount() ) {
        qLog(Bluetooth) << "DevicesTableController::refreshDevice() bad index ("
                << row << "), current device count is" << deviceCount();
        return;
    }

    QBluetoothRemoteDevice &remote = m_displayedDevices[ row ];
    QTableWidgetItem *item;

    // show icon
    item = m_devicesTable->item( row, 0 );
    item->setIcon( find_device_icon( remote ).pixmap( 20 ) );

    // show alias or device name or address
    QString displayName;
    QBluetoothReply<QString> reply = local.remoteAlias( remote.address() );
    if ( !reply.isError() )
        displayName = reply.value();
    if ( displayName.isEmpty() ) {
        // use remote name (or updated name if provided), or address
        displayName = ( updatedName.isEmpty() ) ?
                remote.name() : updatedName;
    }

    // must check whether name is displayable first (i.e. can it be
    // displayed by any of the installed fonts)
    QVariant fontData = ( displayName.isEmpty() ? QVariant() :
            Qtopia::findDisplayFont( displayName ) );
    if ( fontData.isValid() ) {
        item->setText( displayName );
        item->setFont( fontData.value<QFont>() );
    } else {
        // name can't be displayed - use the address string instead
        item->setText( remote.address().toString() );
    }

    // show pairing status
    item = m_devicesTable->item( row, 1 );
    item->setCheckState( local.isPaired( remote.address() ) ?
            Qt::Checked : Qt::Unchecked );

    qLog(Bluetooth) << "Refreshed display for" << remote.address().toString()
            << "row" << row;
}

void DevicesTableController::clear()
{
    // remove column span for discovery item
    if ( m_discoveryButton ) {
        m_devicesTable->setSpan( m_discoveryButton->row(),
                                 m_discoveryButton->column(), 1, 1 );

        // m_discoveryButton will be deleted by table in clear() below
        m_discoveryButton = 0;
    }

    m_displayedDevices.clear();
    m_devicesTable->clear();
    m_devicesTable->setRowCount( 0 );
    m_lastSelectedRow = -1;
}

int DevicesTableController::deviceCount() const
{
    return m_displayedDevices.size();
}

QBluetoothAddress DevicesTableController::deviceAt( int row ) const
{
    if ( row < 0 || row >= deviceCount() ) {
        qLog(Bluetooth) << "DevicesTableController::deviceAt() bad index ("
                << row << "), current device count is" << deviceCount();
        return QBluetoothAddress();
    }
    return m_displayedDevices[row].address();
}

QBluetoothRemoteDevice DevicesTableController::deviceObjectAt( int row ) const
{
    if ( row < 0 || row >= deviceCount() ) {
        qLog(Bluetooth) << "DevicesTableController::deviceObjectAt() bad index ("
                << row << "), current device count is" << deviceCount();
        return QBluetoothRemoteDevice(QBluetoothAddress());
    }
    return m_displayedDevices[row];
}

QString DevicesTableController::deviceLabelAt( int row ) const
{
    if ( row < 0 || row >= deviceCount() ) {
        qLog(Bluetooth) << "DevicesTableController::deviceLabelAt() bad index ("
                << row << "), current device count is" << deviceCount();
        return QString();
    }
    return m_devicesTable->item( row, 0 )->text();
}

int DevicesTableController::row( const QBluetoothAddress &addr ) const
{
    for ( int i=0; i<m_displayedDevices.count(); i++ ) {
        if ( m_displayedDevices[i].address() == addr )
            return i;
    }
    return -1;
}

QBluetoothAddress DevicesTableController::selectedDevice() const
{
    if ( deviceCount() == 0 )
        return QBluetoothAddress();

    int currentRow = m_devicesTable->currentRow();

    // is the "find more devices" button selected?
    if ( currentRow < 0 ||
         ( m_discoveryButton && currentRow == m_discoveryButton->row() ) )
        return QBluetoothAddress();

    return deviceAt( currentRow );
}

QTableWidget *DevicesTableController::devicesTable() const
{
    return m_devicesTable;
}

void DevicesTableController::redrawTable()
{
    // table horizontal header
    QHeaderView *header = m_devicesTable->horizontalHeader();
    header->setResizeMode( 0, QHeaderView::Stretch );
    header->setResizeMode( 1, QHeaderView::Fixed );
    QPixmap pairedIcon( ":image/bluetooth/paired" );
    header->resizeSection( 1, pairedIcon.size().width() * 2 );
    header->setClickable( false );
    header->setMovable( false );
    header->setFocusPolicy( Qt::NoFocus );

    QTableWidgetItem *headerItem = new QTableWidgetItem();
    headerItem->setText( tr( "Device name" ) );
    m_devicesTable->setHorizontalHeaderItem( 0, headerItem );

    headerItem = new QTableWidgetItem();
    headerItem->setIcon( pairedIcon );
    m_devicesTable->setHorizontalHeaderItem( 1, headerItem );
}

void DevicesTableController::showDiscoveryButton()
{
    if ( m_discoveryButton )
        return;

    int row = m_devicesTable->rowCount();
    m_devicesTable->setRowCount( row + 1 );

    m_discoveryButton = new QTableWidgetItem( tr("Find more devices...") );
    m_discoveryButton->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    m_discoveryButton->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    m_devicesTable->setItem( row, 0, m_discoveryButton );
    m_devicesTable->setSpan( row, 0, 1, 2 );

    // select button if nothing else in table
    if ( row == 0 )
        m_devicesTable->setCurrentCell( row, 0 );
}

void DevicesTableController::setPaired( int row, bool paired )
{
    if ( row < 0 || row >= deviceCount() ) {
        qLog(Bluetooth) << "DevicesTableController::setPaired() bad index ("
                << row << "), current device count is" << deviceCount();
        return;
    }

    QTableWidgetItem *item = m_devicesTable->item( row, 1 );
    item->setCheckState( paired ? Qt::Checked : Qt::Unchecked );
}

void DevicesTableController::cellActivated( int row, int col )
{
    if ( m_discoveryButton &&
         m_devicesTable->item( row, col ) == m_discoveryButton ) {
        m_parent->discoveryActionTriggered();
        return;
         }

         if ( col == 0 ) {
             m_parent->activatedDevice( row );

         } else if ( col == 1 ) {
             if ( m_devicesTable->item( row, col )->checkState() == Qt::Checked )
                 m_parent->unpairingRequested( row );
             else
                 m_parent->pairingRequested( row );
         }
}

void DevicesTableController::currentCellChanged( int row, int col, int, int)
{
    if ( deviceCount() != 0 && col == 0 ) {
        // check not just shifting selection across a row
        if ( row != m_lastSelectedRow ) {
            m_parent->changedDeviceSelection();
            m_lastSelectedRow = row;
        }
    }
}



/*!
   \internal
   \class BluetoothIconBlinker
   Blinks a little Bluetooth icon.
 */
BluetoothIconBlinker::BluetoothIconBlinker( int blinkInterval, QObject *parent )
    : QObject( parent ),
      m_icon( new QLabel ),
      m_timer( new QTimer )
{
    m_timer->setInterval( blinkInterval );
    connect( m_timer, SIGNAL(timeout()), SLOT(toggleIconImage()) );

    int size = 22;
    m_pixmapOffline = QIcon( ":icon/bluetooth/bluetooth-offline" ).pixmap( size );
    m_pixmapOnline = QIcon( ":icon/bluetooth/bluetooth-online" ).pixmap( size );

    m_icon->setPixmap( m_pixmapOffline );
    m_icon->setAlignment( Qt::AlignRight );
}

BluetoothIconBlinker::~BluetoothIconBlinker()
{
    delete m_icon;
    delete m_timer;
}

void BluetoothIconBlinker::setBlinking( bool blink )
{
    if ( blink ) {
        m_timer->start();
        m_icon->setPixmap( m_pixmapOnline );
    } else {
        m_timer->stop();
        m_icon->setPixmap( m_pixmapOffline );
    }
}

QLabel *BluetoothIconBlinker::iconLabel() const
{
    return m_icon;
}

void BluetoothIconBlinker::toggleIconImage()
{
    if ( m_icon->pixmap()->serialNumber() == m_pixmapOnline.serialNumber() )
        m_icon->setPixmap( m_pixmapOffline );
    else
        m_icon->setPixmap( m_pixmapOnline );
}



/*!
    \class QBluetoothRemoteDeviceDialogFilter
    \brief The QBluetoothRemoteDeviceDialogFilter class provides a filter on the devices that are displayed by a QBluetoothRemoteDeviceDialog.

    The QBluetoothRemoteDeviceDialogFilter class provides a filter on the devices
    that are displayed by a QBluetoothRemoteDeviceDialog. This allows the
    programmer to control whether particular devices should be displayed in
    the dialog.

    For example, to create a remote device dialog that only displays computers
    and phones:
    \code
    QBluetoothRemoteDeviceDialogFilter filter;
    QSet<QBluetooth::DeviceMajor> majors;
    majors.insert(QBluetooth::Computer);
    majors.insert(QBluetooth::Phone);
    filter.setAcceptedDeviceMajors(majors);

    QBluetoothRemoteDeviceDialog dialog;
    dialog.setFilter(filter);
    \endcode

    By default the QBluetoothRemoteDeviceDialogFilter will accept all devices,
    including those with an invalid class of device information.

    \bold {Note:} The programmer should be careful when using device majors,
    device minors and service classes as a means of filtering devices, as some
    devices may not have the correct class of device information.

    \ingroup qtopiabluetooth
    \sa QBluetoothRemoteDeviceDialog
 */

/*!
    Constructs a QBluetoothRemoteDeviceDialogFilter that will accept all devices.
 */
QBluetoothRemoteDeviceDialogFilter::QBluetoothRemoteDeviceDialogFilter()
    : m_serviceClasses( QBluetooth::AllServiceClasses )
{
}

/*!
    Destructs a QBluetoothRemoteDeviceDialogFilter.
 */
QBluetoothRemoteDeviceDialogFilter::~QBluetoothRemoteDeviceDialogFilter()
{
}

/*!
    Constructs a new QBluetoothRemoteDeviceDialogFilter from \a other.
 */
QBluetoothRemoteDeviceDialogFilter::QBluetoothRemoteDeviceDialogFilter( const QBluetoothRemoteDeviceDialogFilter &other )
{
    operator=( other );
}

/*!
    Assigns the contents of \a other to the current object.
 */
QBluetoothRemoteDeviceDialogFilter &QBluetoothRemoteDeviceDialogFilter::operator=(const QBluetoothRemoteDeviceDialogFilter &other)
{
    if ( this == &other )
        return *this;

    m_deviceMajors = other.m_deviceMajors;
    m_serviceClasses = other.m_serviceClasses;

    return *this;
}

/*!
    Returns whether this filter is equal to \a other.
 */
bool QBluetoothRemoteDeviceDialogFilter::operator==(const QBluetoothRemoteDeviceDialogFilter &other)
{
    if ( this == &other )
        return true;

    return ( m_deviceMajors == other.m_deviceMajors &&
            m_serviceClasses == other.m_serviceClasses );
}

/*!
    Sets the filter to accept devices that match at least one of the
    device majors contained in \a deviceMajors. If a device does not match at
    least one of the specified device majors, it will not be displayed in
    the associated device selector.

    For example, to create a filter that only accepts computers and phones:
    \code
        QBluetoothRemoteDeviceDialogFilter filter;
        QSet<QBluetooth::DeviceMajor> majors;
        majors.insert(QBluetooth::Computer);
        majors.insert(QBluetooth::Phone);
        filter.setAcceptedDeviceMajors(majors);
    \endcode

    If the filter should accept devices regardless of their device majors,
    pass an empty set to this method. (This is the default value.)

    \sa acceptedDeviceMajors()
 */
void QBluetoothRemoteDeviceDialogFilter::setAcceptedDeviceMajors( const QSet<QBluetooth::DeviceMajor> &deviceMajors )
{
    m_deviceMajors = deviceMajors;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value is an empty set (i.e. the filter will accept devices with
    any device major).

    \sa setAcceptedDeviceMajors()
 */
QSet<QBluetooth::DeviceMajor> QBluetoothRemoteDeviceDialogFilter::acceptedDeviceMajors() const
{
    return m_deviceMajors;
}

/*!
    Sets the filter to accept devices that match the given \a serviceClasses.

    For example, to create a filter that only accepts devices with the
    ObjectTransfer and Telephony service classes:
    \code
        QBluetoothRemoteDeviceDialogFilter filter;
        filter.setAcceptedServiceClasses(QBluetooth::ObjectTransfer | QBluetooth::Telephony);
    \endcode

    If the filter should accept devices regardless of their service classes,
    pass QBluetooth::AllServiceClasses to this method. (This is the default
    value.)

    \sa acceptedServiceClasses()
 */
void QBluetoothRemoteDeviceDialogFilter::setAcceptedServiceClasses( QBluetooth::ServiceClasses serviceClasses )
{
    m_serviceClasses = serviceClasses;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value is QBluetooth::AllServiceClasses (i.e. the filter will accept
    devices with any service class).

    \sa setAcceptedServiceClasses()
 */
QBluetooth::ServiceClasses QBluetoothRemoteDeviceDialogFilter::acceptedServiceClasses() const
{
    return m_serviceClasses;
}

/*!
    Returns whether this filter allows the device \a device to be displayed
    in the associated device selector.

    If the accepted service class is QBluetooth::AllServiceClasses, the
    default implementation will also accept devices that have invalid service
    class values.
 */
bool QBluetoothRemoteDeviceDialogFilter::filterAcceptsDevice( const QBluetoothRemoteDevice &device )
{
    if ( m_deviceMajors.size() > 0 ) {
        if ( !m_deviceMajors.contains( device.deviceMajor() ) )
            return false;
    }

    // only do the & operation if the filter value is not AllServiceClasses,
    // otherwise you miss e.g. devices that have service class of 0
    if ( m_serviceClasses != QBluetooth::AllServiceClasses ) {
        if ( !(m_serviceClasses & device.serviceClasses()) )
            return false;
    }

    return true;
}



/*!
   \internal
   \class QBluetoothRemoteDeviceDialogPrivate
   Private implementation class for QBluetoothRemoteDeviceDialog.
 */

const QString QBluetoothRemoteDeviceDialogPrivate::SETTINGS_FAVORITES_KEY = "Favorites";

QBluetoothRemoteDeviceDialogPrivate::QBluetoothRemoteDeviceDialogPrivate( const QBluetoothAddress &localAddr, bool showPairedAndFavorites, QBluetoothRemoteDeviceDialog *parent )
    : QWidget( parent ),
      m_parent( parent ),
      m_local( new QBluetoothLocalDevice( localAddr, this ) ),
      m_showPairedAndFavorites( showPairedAndFavorites ),
      m_sdap( new QBluetoothSdpQuery() ),
      m_devicesTableController( new DevicesTableController( this ) ),
      m_neverShownBefore( true ),
      m_discovering( false ),
      m_cancelledDiscovery( false ),
      m_settings( "Trolltech", "Bluetooth" ),
      m_deviceInfoDialog( 0 ),
      m_cancellingSearch( false )
{
    initParentUI();
    initWidgets();
    initActions();

    if ( !m_local->isValid() ) {
        hide();

        QLayout *layout = m_parent->layout();
        if ( !layout )
            layout = new QVBoxLayout( m_parent );
        QLabel *label = new QLabel( tr( "<P>No bluetooth adapters found" ) );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        label->setWordWrap( true );
        layout->addWidget( label );
        return;
    }

    QStringList favorites = m_settings.value( SETTINGS_FAVORITES_KEY ).toStringList();
    for (int i=0; i<favorites.size(); i++)
        m_favorites.insert(favorites[i], true);

    // call parent's accept() when a device is activated/validated
    connect( this, SIGNAL(deviceActivatedOk()),
             m_parent, SLOT(accept()) );

    // device discovery
    connect( m_local, SIGNAL(remoteDeviceFound(
            const QBluetoothRemoteDevice&)),
            SLOT(discoveredDevice(const QBluetoothRemoteDevice &)) );
    connect( m_local, SIGNAL(discoveryCompleted()),
             SLOT(completedDeviceDiscovery()) );
    connect( m_local, SIGNAL(discoveryCancelled()),
             SLOT(deviceDiscoveryCancelled()) );
    connect( m_local, SIGNAL(error(QBluetoothLocalDevice::Error,
             const QString &)),
             SLOT(localDeviceError(QBluetoothLocalDevice::Error, const QString &)));

    // service discovery / validating device services
    connect( m_sdap, SIGNAL(searchComplete(const QBluetoothSdpQueryResult&)),
             SLOT(foundServices(const QBluetoothSdpQueryResult &)) );
    connect( m_sdap, SIGNAL(searchCancelled()),
             SLOT(serviceSearchCancelled()) );

    // remote alias changes
    connect( m_local, SIGNAL(remoteAliasChanged(const QBluetoothAddress &,
             const QString &)),
             SLOT(updateDeviceDisplay(const QBluetoothAddress &)) );
    connect( m_local, SIGNAL(remoteAliasRemoved(const QBluetoothAddress &)),
             SLOT(updateDeviceDisplay(const QBluetoothAddress &)) );

    // Remote Name Changes
    connect( m_local, SIGNAL(remoteNameUpdated(const QBluetoothAddress &, const QString &)),
             SLOT(remoteNameUpdated(const QBluetoothAddress &, const QString &)));
}

QBluetoothRemoteDeviceDialogPrivate::~QBluetoothRemoteDeviceDialogPrivate()
{
    cleanUp();

    delete m_sdap;
}

void QBluetoothRemoteDeviceDialogPrivate::cleanUp()
{
    if ( m_discovering )
        cancelDeviceDiscovery();
}

bool QBluetoothRemoteDeviceDialogPrivate::startDeviceDiscovery( bool clear )
{
    if ( m_discovering || m_validationWaitWidget->isVisible() ) {
        qLog(Bluetooth) << "Cannot start discovery, device selector is busy";
        return false;
    }

    if ( !m_local->discoverRemoteDevices() ) {
        qLog(Bluetooth) << "Can't start discovery, last error was:"
            << m_local->error();
        return false;
    }

    m_bluetoothStatusIcon->setBlinking( true );
    if ( clear )
        clearDevices();

    m_statusLabel->setText( tr( "Searching..." ) );
    m_discoveryAction->setText( tr( "Stop searching" ) );
    m_discoveryAction->setIcon( m_stopSearchIcon );

    m_cancelledDiscovery = false;
    m_discovering = true;

    qLog(Bluetooth) << "Start remote device discovery";

    return true;
}


void QBluetoothRemoteDeviceDialogPrivate::discoveredDevice( const QBluetoothRemoteDevice &device )
{
    addDeviceToDisplay( device );
}

void QBluetoothRemoteDeviceDialogPrivate::completedDeviceDiscovery()
{
    qLog(Bluetooth) << "completedDeviceDiscovery";

    // update status label
    int foundDevicesCount = m_devicesTableController->deviceCount();
    if ( foundDevicesCount == 0 )
        m_statusLabel->setText( tr( "No devices found" ) );
    else if ( foundDevicesCount == 1 )
        m_statusLabel->setText( tr( "1 device found" ) );
    else
        m_statusLabel->setText( tr( "%1 devices found",
                                "%1 = number (#>1) of devices" )
                .arg( foundDevicesCount ) );

    m_bluetoothStatusIcon->setBlinking( false );
    m_discovering = false;

    m_discoveryAction->setText( tr( "Search again" ) );
    m_discoveryAction->setIcon( m_searchIcon );
}

void QBluetoothRemoteDeviceDialogPrivate::cancelDeviceDiscovery()
{
    qLog(Bluetooth) << "called QBluetoothRemoteDeviceDialogPrivate::cancelDeviceDiscovery";

    if ( m_discovering && !m_cancelledDiscovery ) {
        m_cancelledDiscovery = true;
        qLog(Bluetooth) << "Trying to cancel discovery...";
        if ( !m_local->cancelDiscovery() ) {
            qLog(Bluetooth) << "Error cancelling discovery:"
                    << m_local->error();
        }
    }
}

void QBluetoothRemoteDeviceDialogPrivate::deviceDiscoveryCancelled()
{
    completedDeviceDiscovery();
}

void QBluetoothRemoteDeviceDialogPrivate::remoteNameUpdated(const QBluetoothAddress &dev, const QString &name)
{
    qLog(Bluetooth) << "remoteNameUpdated()" << dev.toString() << name;
    if ( dev.isValid() ) {
        int row = m_devicesTableController->row( dev );
        if ( row != -1 ) {
            m_devicesTableController->refreshDevice( row, *m_local, name );
        }
    }
}

void QBluetoothRemoteDeviceDialogPrivate::discoveryActionTriggered()
{
    if ( m_discovering ) {
        cancelDeviceDiscovery();
    } else {
        if ( !startDeviceDiscovery( true ) ) {
            QMessageBox::warning( this, tr( "Discovery Error" ),
                                  tr( "<P>Unable to start device discovery" ) );
        }
    }
}

void QBluetoothRemoteDeviceDialogPrivate::localDeviceError( QBluetoothLocalDevice::Error error, const QString &message )
{
    qLog(Bluetooth) << "QBluetoothRemoteDeviceDialogPrivate::localDeviceError:"
            << error << message;
}

void QBluetoothRemoteDeviceDialogPrivate::pairingRequested( int row )
{
    int result = QMessageBox::question( this, tr( "Pair Device" ),
                                        tr( "<P>Would you like to pair with this device?" ),
                                        QMessageBox::Yes,
                                        QMessageBox::No|QMessageBox::Default );

    if ( result != QMessageBox::Yes )
        return;

    if ( m_discovering ) {
        QWaitWidget waitWidget( this );
        waitWidget.setCancelEnabled( true );

        cancelDeviceDiscovery();
        qLog(Bluetooth) << "Discovery in progress, delaying pairing";
        while ( m_discovering ) {
            qApp->processEvents();
            if ( waitWidget.wasCancelled() )
                return;
        }
    }

    performPairingOperation( row, QBluetoothPairingAgent::Pair );
}

void QBluetoothRemoteDeviceDialogPrivate::unpairingRequested( int row )
{
    int result = QMessageBox::question( this, tr( "Pair Device" ),
                                        tr( "<P>Would you like to unpair from this device?" ),
                                        QMessageBox::Yes,
                                        QMessageBox::No|QMessageBox::Default );

    if ( result != QMessageBox::Yes )
        return;

    performPairingOperation( row, QBluetoothPairingAgent::Unpair );
}

void QBluetoothRemoteDeviceDialogPrivate::performPairingOperation( int row, QBluetoothPairingAgent::Operation op )
{
    const QBluetoothAddress &addr = m_devicesTableController->deviceAt( row );
    QBluetoothPairingAgent agent( op, addr, *m_local );
    QBluetoothPairingAgent::Result result = agent.exec();

    switch( result ) {
        case QBluetoothPairingAgent::Success:
            m_devicesTableController->setPaired(
                    m_devicesTableController->row( addr ),
            ( op == QBluetoothPairingAgent::Pair ) );
            break;

        case QBluetoothPairingAgent::Failed:
            agent.showErrorDialog();
            break;

        case QBluetoothPairingAgent::Cancelled:
            // do nothing if cancelled
            break;

        default:
            qLog(Bluetooth) << "Bad result from internal QBluetoothPairingAgent!";
    }
}

bool QBluetoothRemoteDeviceDialogPrivate::serviceProfilesMatch( const QList<QBluetoothSdpRecord> services, QSet<QBluetooth::SDPProfile> profiles )
{
    QSetIterator<QBluetooth::SDPProfile> iter( profiles );
    for ( int i=0; i<services.size(); i++ ) {
        while ( iter.hasNext() ) {
            QBluetooth::SDPProfile profile = iter.next();
            if (services[i].isInstance(profile))
                return true;
        }
        iter.toFront();
    }
    return false;
}

void QBluetoothRemoteDeviceDialogPrivate::foundServices( const QBluetoothSdpQueryResult &result )
{
    m_cancellingSearch = false;

    const QList<QBluetoothSdpRecord> services = result.services();

    qLog(Bluetooth) << "RemoteDeviceDialog: foundServices" << services.count();

    if ( !m_deviceUnderValidation.isValid() ) {
        qLog(Bluetooth) << "foundServices(): no device to validate,"
                << "validation was cancelled?";
        deviceValidationCompleted();
        return;
    }

    if ( !result.isValid() ) {
        qLog(Bluetooth) << "Error occured in service search:" << result.error();

        QMessageBox::warning( this, tr( "Service Error" ),
                              tr( "<P>Unable to verify services for %1. Try another device." )
                                      .arg( m_devicesTableController->deviceLabelAt(
                                      m_devicesTableController->row( m_deviceUnderValidation ) ) ) );
        deviceValidationCompleted();
        return;
    }

    QBluetoothAddress addr = m_deviceUnderValidation;
    deviceValidationCompleted();

    if ( serviceProfilesMatch( services, m_validProfiles ) ) {

        // device is now validated, emit the activation signal
        qLog(Bluetooth) << "Device successfully validated";
        emit deviceActivatedOk();

    } else {
        QMessageBox::warning( this, tr( "Service Error" ),
            tr( "<P>Device does not have the necessary services. Try another device." ) );
    }
}

void QBluetoothRemoteDeviceDialogPrivate::serviceSearchCancelled()
{
    qLog(Bluetooth) << "serviceSearchCancelled()";
    m_cancellingSearch = false;
}

void QBluetoothRemoteDeviceDialogPrivate::deviceValidationCompleted()
{
    m_validationWaitWidget->hide();
    m_deviceUnderValidation = QBluetoothAddress();  // reset
}

void QBluetoothRemoteDeviceDialogPrivate::deviceValidationCancelled()
{
    qLog(Bluetooth) << "deviceValidationCancelled";

    if (m_cancellingSearch) {
        // already waiting for a search to finish, so don't need to call
        // m_sdap.cancelSearch()
        qLog(Bluetooth) << "Already waiting for search to finish";
        deviceValidationCompleted();
    } else {
        qLog(Bluetooth) << "Cancelling search...";
        m_cancellingSearch = true;
        m_sdap->cancelSearch();
        deviceValidationCompleted();
    }
}

void QBluetoothRemoteDeviceDialogPrivate::changedDeviceSelection()
{
    const QBluetoothAddress &addr = m_devicesTableController->selectedDevice();

    if ( isFavorite( addr ) )
        m_favoritesAction->setText( tr( "Remove from favorites" ) );
    else
        m_favoritesAction->setText( tr( "Add to favorites" ) );

    enableDeviceActions( addr.isValid() );
}

void QBluetoothRemoteDeviceDialogPrivate::activatedDevice( int row )
{
    //if ( m_discovering )
    //    cancelDeviceDiscovery();

    QBluetoothAddress addr = m_devicesTableController->deviceAt(row);

    if ( m_validProfiles.size() == 0 ) {
        // no need to validate device
        qLog(Bluetooth) << "Emitting deviceActivatedOk" << addr.toString();
        emit deviceActivatedOk();

    } else {
        qLog(Bluetooth) << "Validating services for" << addr.toString();

        m_validationWaitWidget->show();
        m_deviceUnderValidation = addr;
        validateServices();
    }
}

void QBluetoothRemoteDeviceDialogPrivate::validateServices()
{
    if (!m_validationWaitWidget->isVisible()) {
        qLog(Bluetooth) << "RemoteDeviceDialog: validation was cancelled";
        return;
    }

    if (m_cancellingSearch) {
        qLog(Bluetooth) << "RemoteDeviceDialog: waiting for service search"
                << "to cancel...";
        QTimer::singleShot(100, this, SLOT(validateServices()));
        return;
    }

    if (!m_deviceUnderValidation.isValid()) {
        deviceValidationCompleted();
        return;
    }

    if ( !m_sdap->searchServices( m_deviceUnderValidation, *m_local, QBluetoothSdpUuid::L2cap) ) {
        QMessageBox::warning( this, tr( "Service Error" ),
            tr( "<P>Unable to verify services. Try another device." ) );
        deviceValidationCompleted();
    }
}


void QBluetoothRemoteDeviceDialogPrivate::enableDeviceActions( bool enable )
{
    for ( int i=0; i<m_deviceActions.size(); i++ )
        m_deviceActions.at(i)->setEnabled( enable );
}

void QBluetoothRemoteDeviceDialogPrivate::addDeviceToDisplay( const QBluetoothRemoteDevice &device )
{
    if ( !m_filter.filterAcceptsDevice( device ) ) {
        qLog(Bluetooth) << "Device" << device.name()
                << "rejected by filter";
        return;
    }

    m_devicesTableController->addDevice( device, *m_local );
}

void QBluetoothRemoteDeviceDialogPrivate::clearDevices()
{
    m_devicesTableController->clear();
    m_statusLabel->setText("");
    enableDeviceActions( false );

    m_devicesTableController->redrawTable();
}

void QBluetoothRemoteDeviceDialogPrivate::addFavorite( const QBluetoothAddress &addr )
{
    m_favorites.insert(addr.toString(), true);
    m_settings.setValue( SETTINGS_FAVORITES_KEY, QVariant( m_favorites.keys() ) );
}

void QBluetoothRemoteDeviceDialogPrivate::removeFavorite( const QBluetoothAddress &addr )
{
    m_favorites.remove(addr.toString());
    m_settings.setValue( SETTINGS_FAVORITES_KEY, QVariant( m_favorites.keys() ) );
}

bool QBluetoothRemoteDeviceDialogPrivate::isFavorite( const QBluetoothAddress &addr )
{
    return m_favorites.contains( addr.toString() );
}

void QBluetoothRemoteDeviceDialogPrivate::loadPairedAndFavorites()
{
    // display all found favourites
    QList<QString> favorites = m_favorites.keys();
    for ( int i=0; i<favorites.size(); i++ ) {
        QBluetoothRemoteDevice device =
                QBluetoothRemoteDevice( QBluetoothAddress(favorites[i]) );
        m_local->updateRemoteDevice( device );
        addDeviceToDisplay(device);
    }

    // display paired devices
    QList<QBluetoothAddress> paired = m_local->pairedDevices();
    for ( int i=0; i<paired.size(); i++ ) {
        QBluetoothRemoteDevice device( paired[i] );
        m_local->updateRemoteDevice( device );
        addDeviceToDisplay(device);
    }
}

void QBluetoothRemoteDeviceDialogPrivate::favoritesActionTriggered()
{
    const QBluetoothAddress &selected = m_devicesTableController->selectedDevice();
    if ( m_favoritesAction->text() == tr( "Remove from favorites" ) ) {
        removeFavorite( selected );

        m_favoritesAction->setText( tr( "Add to favorites" ) );

    } else {
        addFavorite( selected );

        m_favoritesAction->setText( tr( "Remove from favorites" ) );

    }
}

void QBluetoothRemoteDeviceDialogPrivate::showRemoteProperties()
{
    QBluetoothAddress selected = m_devicesTableController->selectedDevice();

    for ( int i=0; i<m_devicesTableController->deviceCount(); i++ ) {
        if ( m_devicesTableController->deviceAt(i) == selected ) {
            qLog(Bluetooth) << "Show properties for" << selected.toString();

            if ( !m_deviceInfoDialog ) {
                m_deviceInfoDialog = new RemoteDevicePropertiesDialog(
                        m_local->address(), this );
            }
            const QBluetoothRemoteDevice device =
                    m_devicesTableController->deviceObjectAt(i);
            m_deviceInfoDialog->setRemoteDevice( device,
                    find_device_icon( device ).pixmap( 20 ) );

            QtopiaApplication::execDialog( m_deviceInfoDialog );
            return;
        }
    }
}

void QBluetoothRemoteDeviceDialogPrivate::updateDeviceDisplay( const QBluetoothAddress &addr )
{
    int row = m_devicesTableController->row( addr );
    if ( row != -1 ) {
        m_devicesTableController->refreshDevice( row, *m_local );
    }
}

void QBluetoothRemoteDeviceDialogPrivate::initParentUI()
{
    QVBoxLayout *layout = new QVBoxLayout( m_parent );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( this );

    m_parent->setWindowTitle( tr( "Bluetooth devices" ) );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QtopiaApplication::setMenuLike( m_parent, true );
#endif
}

void QBluetoothRemoteDeviceDialogPrivate::initWidgets()
{
    // base layout
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 2 );
    layout->setSpacing( 2 );

    // horizontal layout for discovery status text + flashing icon
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin( 0 );
    hLayout->setSpacing( 0 );

    // status text
    m_statusLabel = new QLabel( this );
    m_statusLabel->setAlignment( Qt::AlignLeft );
    hLayout->addWidget( m_statusLabel );

    // status icon
    m_bluetoothStatusIcon = new BluetoothIconBlinker( 300, this );
    hLayout->addWidget( m_bluetoothStatusIcon->iconLabel() );

    layout->addLayout( hLayout );

    // table of devices
    layout->addWidget( m_devicesTableController->devicesTable() );

    // wait widgets (fill up the screen with a "please wait" type of screen)
    //m_pairingWaitWidget = new QWaitWidget( this );
    m_validationWaitWidget = new QWaitWidget( this );
    m_validationWaitWidget->setText( tr( "<P>Validating device..." ) );

    // allow wait widget to be cancelled by user
    m_validationWaitWidget->setCancelEnabled( true );
    connect( m_validationWaitWidget, SIGNAL(cancelled()),
             SLOT(deviceValidationCancelled()) );
}

void QBluetoothRemoteDeviceDialogPrivate::initActions()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_menu = QSoftMenuBar::menuFor(this);
#else
    QToolBar *bar = new QToolBar(this);
    addToolBar(bar);
    bar->setMovable(false);
    m_menu = bar;

    QMenuBar *mb = new QMenuBar(bar);
    m_menu = mb->addMenu(tr("Options"));
#endif

    // properties action
    QAction *detailsAction = new QAction( QIcon( ":icon/view" ),
                                          tr( "View details..." ), this );
    connect( detailsAction, SIGNAL(triggered()),
             SLOT(showRemoteProperties()) );
    addDeviceAction( detailsAction, false );

    // add/remove favorites action
    m_favoritesAction = new QAction( tr( "Add to favorites" ), this );
    connect( m_favoritesAction, SIGNAL(triggered()),
             SLOT(favoritesActionTriggered()) );
    addDeviceAction( m_favoritesAction, false );

    // device discovery action
    m_searchIcon = QIcon( ":icon/find" );
    m_stopSearchIcon = QIcon( ":icon/reset" );
    m_discoveryAction = new QAction( m_searchIcon,
                                     tr( "Search for devices" ), this );
    connect( m_discoveryAction, SIGNAL(triggered()),
             SLOT(discoveryActionTriggered()) );
    m_discoveryAction->setEnabled( true );
    m_discoveryAction->setVisible( true );
    m_menu->addAction( m_discoveryAction );

    // this needs to be set so you know where to insert the separator
    // between the device actions (which are customisable) and the default
    // actions (which are always present)
    m_lastPersistentAction = detailsAction;
}

void QBluetoothRemoteDeviceDialogPrivate::setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles )
{
    m_validProfiles = profiles;
}

QSet<QBluetooth::SDPProfile> QBluetoothRemoteDeviceDialogPrivate::validationProfiles() const
{
    return m_validProfiles;
}

void QBluetoothRemoteDeviceDialogPrivate::setFilter( const QBluetoothRemoteDeviceDialogFilter &filter )
{
    m_filter = filter;
}

QBluetoothRemoteDeviceDialogFilter QBluetoothRemoteDeviceDialogPrivate::filter() const
{
    return m_filter;
}

void QBluetoothRemoteDeviceDialogPrivate::addDeviceAction( QAction *action, bool isExternalAction )
{
    m_deviceActions.append( action );
    action->setEnabled( m_devicesTableController->selectedDevice().isValid() );
    action->setVisible( true );

    // if action is externally added action, group them separately from
    // the default device selector actions (i.e. put them above the
    // extra menu separator)
    if ( isExternalAction ) {
        if ( !m_deviceActionsSeparator ) {
            m_deviceActionsSeparator = m_menu->insertSeparator(
                    m_lastPersistentAction );
        }
        m_menu->insertAction( m_deviceActionsSeparator, action );
    } else {
        m_menu->addAction( action );
    }
}

void QBluetoothRemoteDeviceDialogPrivate::removeDeviceAction( QAction *action )
{
    int i = m_deviceActions.indexOf( action );
    if ( i == -1 )
        return;

    m_deviceActions.removeAt( i );

    m_menu->removeAction(action);

    // if no more device actions, remove the separator
    if ( m_deviceActions.size() == 0 ) {
        delete m_deviceActionsSeparator;
        m_deviceActionsSeparator = 0;
    }
}

void QBluetoothRemoteDeviceDialogPrivate::showEvent( QShowEvent *e )
{
    if ( m_neverShownBefore ) {
        if ( m_showPairedAndFavorites )
            loadPairedAndFavorites();

        if ( m_devicesTableController->deviceCount() == 0 ) {
            startDeviceDiscovery( true );
        } else {
            m_devicesTableController->showDiscoveryButton();
        }

        m_neverShownBefore = false;
    }

    m_devicesTableController->redrawTable();
    QWidget::showEvent( e );
}

QBluetoothAddress QBluetoothRemoteDeviceDialogPrivate::selectedDevice() const
{
    return m_devicesTableController->selectedDevice();
}


/*!
    \class QBluetoothRemoteDeviceDialog
    \brief The QBluetoothRemoteDeviceDialog class allows the user to perform a bluetooth device discovery and select a particular device.

    When a remote device dialog is first displayed, it shows all paired and
    favorite devices, and allows a device discovery to be activated in order
    to display remote devices found in the vicinity. If the system has no
    paired or favorite devices, a device discovery will be started immediately.

    The setFilter() function can be used to ensure that the device dialog
    only displays certain types of devices.

    Custom menu actions can be added to a device dialog through
    QWidget::addAction(). Any added actions will be enabled when a device is
    selected, and disabled when no devices are selected. They are also
    disabled during device discoveries.

    The getRemoteDevice() static method is the easiest way to run a device
    dialog. It runs a modal device selection dialog, then returns the
    address of the device that was activated by the user.

    \ingroup qtopiabluetooth
    \sa QBluetoothRemoteDeviceDialogFilter
 */

/*!
    Constructs a QBluetoothRemoteDeviceDialog with the given parent widget
    \a parent and the window flags \a flags. Paired and favorite devices will be
    listed in the device selector when it is first displayed.
 */
QBluetoothRemoteDeviceDialog::QBluetoothRemoteDeviceDialog( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    m_data = new QBluetoothRemoteDeviceDialogPrivate(
            QBluetoothLocalDevice().address(), true, this );
}

/*!
    Constructs a QBluetoothRemoteDeviceDialog with the given parent widget
    \a parent and the window flags \a flags. If \a showPairedAndFavorites is
    \c true, paired and favorite devices (as determined by user settings) will be
    listed in the device selector when it is first displayed.
 */
QBluetoothRemoteDeviceDialog::QBluetoothRemoteDeviceDialog( bool showPairedAndFavorites, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    m_data = new QBluetoothRemoteDeviceDialogPrivate(
            QBluetoothLocalDevice().address(), showPairedAndFavorites, this );
}

/*!
    Destructs a QBluetoothRemoteDeviceDialog object.
 */
QBluetoothRemoteDeviceDialog::~QBluetoothRemoteDeviceDialog()
{
}

/*!
    Shows a remote device dialog with the validation profiles \a profiles,
    the device filter \a filter, and the parent \a parent. If \a
    showPairedAndFavorites is \c true, the user's paired and favorite devices
    (as determined by user settings) will be shown in the device dialog when
    it is first displayed. (Note that these devices are also subject to the
    given filters.)

    Returns the address of the remote device that was activated, or
    an invalid address if the dialog was cancelled.

    Note that the user will not be able to activate a device if it does not
    have the necessary profiles as set by setValidationProfiles().

    \sa setValidationProfiles(), QBluetoothRemoteDeviceDialogFilter
 */
QBluetoothAddress QBluetoothRemoteDeviceDialog::getRemoteDevice(
        QWidget *parent,
        QSet<QBluetooth::SDPProfile> profiles,
        const QBluetoothRemoteDeviceDialogFilter &filter,
        bool showPairedAndFavorites )
{
    QBluetoothRemoteDeviceDialog selector( showPairedAndFavorites, parent );
    selector.setFilter( filter );
    selector.setValidationProfiles( profiles );

    if ( QtopiaApplication::execDialog( &selector ) == QDialog::Accepted )
        return selector.selectedDevice();
    else
        return QBluetoothAddress();
}

/*!
    Set the validation SDP profiles to \a profiles.

    When this is set, a device can only be chosen if it has at least one
    service that matches one or more of the given profiles.

    For example, a dialog like this will only allow the user to select a
    device if it has a service with the OBEX Object Push profile:

    \code
        QSet<QBluetooth::SDPProfile> profiles;
        profiles.insert(QBluetooth::ObjectPushProfile);
        QBluetoothRemoteDeviceDialog dialog;
        dialog.setValidationProfiles(profiles);
    \endcode

    (If the user chooses a device that does not have the Object Push Profile,
    the dialog will not be closed, and a message box will be displayed
    informing the user that the selected device does not have the necessary
    services.)

    \sa validationProfiles()
 */
void QBluetoothRemoteDeviceDialog::setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles )
{
    m_data->setValidationProfiles( profiles );
}

/*!
    Returns the SDP profiles that are used to validate an activated device.

    \sa setValidationProfiles()
 */
QSet<QBluetooth::SDPProfile> QBluetoothRemoteDeviceDialog::validationProfiles() const
{
    return m_data->validationProfiles();
}

/*!
    Sets the filter that is used to filter displayed devices to \a filter.

    \sa filter()
 */
void QBluetoothRemoteDeviceDialog::setFilter( const QBluetoothRemoteDeviceDialogFilter &filter )
{
    m_data->setFilter( filter );
}

/*!
    Returns the filter that is used to filter displayed devices.

    \sa setFilter()
 */
QBluetoothRemoteDeviceDialogFilter QBluetoothRemoteDeviceDialog::filter() const
{
    return m_data->filter();
}

/*!
    Returns the address of the currently selected device.

    Note that a dialog's validation profiles (see setValidationProfiles())
    are only applied when a device is activated, and not when they are merely
    selected. Therefore, if any validation profiles have been set, the
    programmer should be aware that the device returned by this method may not
    have been validated against these profiles.

    \sa setValidationProfiles()
 */
QBluetoothAddress QBluetoothRemoteDeviceDialog::selectedDevice() const
{
    return m_data->selectedDevice();
}

/*!
    \reimp
 */
void QBluetoothRemoteDeviceDialog::done(int r)
{
    m_data->cleanUp();
    QDialog::done(r);
}

/*!
    \reimp
 */
void QBluetoothRemoteDeviceDialog::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
        case QEvent::ActionAdded:
            m_data->addDeviceAction( action, true );
            break;
        case QEvent::ActionRemoved:
            m_data->removeDeviceAction( action );
            break;
        default:
            break;
        
    }
}
