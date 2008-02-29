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

#include <qtopiacomm/private/qbluetoothdeviceselector_p.h>
#include <qtopiacomm/private/remotedevicepropertiesdialog_p.h>

#include <qsdap.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusinterface.h>
#include <qtdbus/qdbusreply.h>
#include <qtopialog.h>

#include <QTimer>

#include <QtopiaApplication>
#include <QAction>
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


/*!
    \internal
    \class DevicesTableController
    Controls a QTableWidget that displays devices and their pairing status.
 */

const Qt::ItemFlags DevicesTableController::ITEM_FLAGS =
        Qt::ItemIsSelectable | Qt::ItemIsEnabled;

DevicesTableController::DevicesTableController(
                                    QBluetoothDeviceSelector_Private *parent )
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


// could possibly take an argument for whether to call updateRemoteDevice
// on the 'local' device to update device name, device class, etc.
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
    item->setIcon( QIcon( m_parent->m_devicePixmaps.value(
                                         remote.deviceMajor() ) ) );

    // show alias or device name or address
    QString alias;
    QBluetoothReply<QString> reply = local.remoteAlias( remote.address() );
    if ( !reply.isError() )
        alias = reply.value();
    if ( alias.isEmpty() ) {
        // use remote name (or updated name if provided), or address
        QString remoteName = ( updatedName.isNull() ) ?
                remote.name() : updatedName;
        if ( remoteName.isEmpty() )
            item->setText( remote.address().toString() );
        else
            item->setText( remoteName );
    } else {
        item->setText( alias );
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

QList<QBluetoothAddress> DevicesTableController::displayedDevices() const
{
    QList<QBluetoothAddress> devices;
    for ( int i=0; i<m_displayedDevices.size(); i++ )
        devices.append( m_displayedDevices[i].address() );
    return devices;
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

    m_bluetoothOfflineIcon.load( ":icon/bluetooth/bluetooth-offline" );
    m_bluetoothOnlineIcon.load( ":icon/bluetooth/bluetooth-online" );

    // TODO shouldn't need to do this, but at the moment they're not same size
    m_bluetoothOnlineIcon =
        m_bluetoothOnlineIcon.scaled( m_bluetoothOfflineIcon.size() );

    m_icon->setPixmap( m_bluetoothOfflineIcon );
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
        m_icon->setPixmap( m_bluetoothOnlineIcon );
    } else {
        m_timer->stop();
        m_icon->setPixmap( m_bluetoothOfflineIcon );
    }
}

QLabel *BluetoothIconBlinker::iconLabel() const
{
    return m_icon;
}

void BluetoothIconBlinker::toggleIconImage()
{
    if ( m_icon->pixmap()->serialNumber() == m_bluetoothOnlineIcon.serialNumber() )
        m_icon->setPixmap( m_bluetoothOfflineIcon );
    else
        m_icon->setPixmap( m_bluetoothOnlineIcon );
}



/*!
    \class QBluetoothDeviceSelectorFilter
    \brief The QBluetoothDeviceSelectorFilter class provides a filter on the devices that are displayed by a QBluetoothDeviceSelector.

    The QBluetoothDeviceSelectorFilter class provides a filter on the devices
    that are displayed by a QBluetoothDeviceSelector. This allows the
    programmer to control whether particular devices should be displayed to
    the end user.

    For example, to display a device selector that only displays computers
    and phones:
    \code
        QBluetoothDeviceSelectorFilter filter;
        QSet<QBluetooth::DeviceMajor> majors;
        majors.insert(QBluetooth::Computer);
        majors.insert(QBluetooth::Phone);
        filter.setAcceptedDeviceMajors(majors);

        QBluetoothDeviceSelector selector;
        selector.setFilter(filter);
    \endcode

    By default the QBluetoothDeviceSelectorFilter will accept all devices,
    including those with invalid class of device information.

    \sa QBluetoothDeviceSelector
 */

/*!
    Constructs a QBluetoothDeviceSelectorFilter that will accept all devices.
 */
QBluetoothDeviceSelectorFilter::QBluetoothDeviceSelectorFilter()
    : m_serviceClasses( QBluetooth::AllServiceClasses )
{
}

/*!
    Destructs a QBluetoothDeviceSelectorFilter.
 */
QBluetoothDeviceSelectorFilter::~QBluetoothDeviceSelectorFilter()
{
}

/*!
    Constructs a new QBluetoothDeviceSelectorFilter from \a other.
 */
QBluetoothDeviceSelectorFilter::QBluetoothDeviceSelectorFilter( const QBluetoothDeviceSelectorFilter &other )
{
    operator=( other );
}

/*!
    Assigns the contents of \a other to the current object.
*/
QBluetoothDeviceSelectorFilter &QBluetoothDeviceSelectorFilter::operator=(const QBluetoothDeviceSelectorFilter &other)
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
bool QBluetoothDeviceSelectorFilter::operator==(const QBluetoothDeviceSelectorFilter &other)
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
        QBluetoothDeviceSelectorFilter filter;
        QSet<QBluetooth::DeviceMajor> majors;
        majors.insert(QBluetooth::Computer);
        majors.insert(QBluetooth::Phone);
        filter.setAcceptedDeviceMajors(majors);
    \endcode

    If the filter should accept devices regardless of their device majors,
    pass an empty set to this method. (This is the default value.)

    \sa acceptedDeviceMajors()
 */
void QBluetoothDeviceSelectorFilter::setAcceptedDeviceMajors( const QSet<QBluetooth::DeviceMajor> &deviceMajors )
{
    m_deviceMajors = deviceMajors;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value is an empty set (i.e. the filter will accept devices with
    any device major).

    \sa setAcceptedDeviceMajors()
 */
QSet<QBluetooth::DeviceMajor> QBluetoothDeviceSelectorFilter::acceptedDeviceMajors() const
{
    return m_deviceMajors;
}

/*!
    Sets the filter to accept devices that match the given \a serviceClasses.

    For example, to create a filter that only accepts devices with the
    ObjectTransfer and Telephony service classes:
    \code
        QBluetoothDeviceSelectorFilter filter;
        filter.setAcceptedServiceClasses(QBluetooth::ObjectTransfer | QBluetooth::Telephony);
    \endcode

    If the filter should accept devices regardless of their service classes,
    pass QBluetooth::AllServiceClasses to this method. (This is the default
    value.)

    \sa acceptedServiceClasses()
 */
void QBluetoothDeviceSelectorFilter::setAcceptedServiceClasses( QBluetooth::ServiceClasses serviceClasses )
{
    m_serviceClasses = serviceClasses;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value QBluetooth::AllServiceClasses (i.e. the filter will accept
    devices with any service class).

    \sa setAcceptedServiceClasses()
 */
QBluetooth::ServiceClasses QBluetoothDeviceSelectorFilter::acceptedServiceClasses() const
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
bool QBluetoothDeviceSelectorFilter::filterAcceptsDevice( const QBluetoothRemoteDevice &device )
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
   \class QBluetoothDeviceSelector_Private
   Private implementation class for QBluetoothDeviceSelector.
 */

const QString QBluetoothDeviceSelector_Private::FAVORITES_ADD_TEXT = tr( "Add to favorites" );
const QString QBluetoothDeviceSelector_Private::FAVORITES_REMOVE_TEXT = tr( "Remove from favorites" );
const QString QBluetoothDeviceSelector_Private::SETTINGS_FAVORITES_KEY = "Favorites";

QBluetoothDeviceSelector_Private::QBluetoothDeviceSelector_Private( const QBluetoothAddress &localAddr, bool loadFavoritesOnShow, QBluetoothDeviceSelector *parent )
    : QWidget( parent ),
      m_parent( parent ),
      m_local( new QBluetoothLocalDevice( localAddr, this ) ),
      m_loadFavoritesOnShow( loadFavoritesOnShow ),
      m_sdap( new QSDAP() ),
      m_devicesTableController( new DevicesTableController( this ) ),
      m_neverShownBefore( true ),
      m_discovering( false ),
      m_cancelledDiscovery( false ),
      m_deviceActions( new QList<QAction *>() ),
      m_forcePairing( false ),
      m_settings( new QSettings("Trolltech", "bluetooth", this) ),
      m_devicePixmaps( defaultDevicePixmaps() ),
      m_deviceInfoDialog( 0 ),
      m_periodicDiscoveryDisabled( false )
{
    initParentUI();
    initWidgets();
    initActions();
    initDBus();

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

    // make the parent selector accept when a device is selected
    connect( m_parent, SIGNAL(deviceActivated(const QBluetoothAddress &)),
             m_parent, SLOT(accept()) );

    // hook up signals to the parent
    connect( this, SIGNAL(deviceSelectionChanged()),
             m_parent, SIGNAL(deviceSelectionChanged()) );
    connect( this, SIGNAL(deviceActivated(const QBluetoothAddress &)),
             m_parent, SIGNAL(deviceActivated(const QBluetoothAddress &)) );

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
    connect( m_local, SIGNAL(discoveryStarted()),
             m_parent, SIGNAL(deviceDiscoveryStarted()) );  // connect straight on

    // service discovery / validating device services
    connect( m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult&)),
             SLOT(foundServices(const QSDAPSearchResult &)) );

    // remote alias changes
    connect( m_local, SIGNAL(remoteAliasChanged(const QBluetoothAddress &,
                                                const QString &)),
             SLOT(updateDeviceDisplay(const QBluetoothAddress &)) );
    connect( m_local, SIGNAL(remoteAliasRemoved(const QBluetoothAddress &)),
             SLOT(updateDeviceDisplay(const QBluetoothAddress &)) );
}

QBluetoothDeviceSelector_Private::~QBluetoothDeviceSelector_Private()
{
    if ( m_discovering )
        cancelDeviceDiscovery();

    // restart the periodic discovery if we stopped it
    restartPeriodicDiscovery();

    delete m_sdap;
    delete m_deviceActions;
}

QHash<QBluetooth::DeviceMajor, QPixmap> QBluetoothDeviceSelector_Private::defaultDevicePixmaps()
{
    QHash<QBluetooth::DeviceMajor, QPixmap> icons;
    icons[QBluetooth::Computer] = QPixmap(":icon/phone/computer");
    icons[QBluetooth::Phone] = QPixmap(":icon/phone/phone");
    icons[QBluetooth::Imaging] = QPixmap(":icon/phone/printer");
    return icons;
}

bool QBluetoothDeviceSelector_Private::startDeviceDiscovery( bool clear )
{
    if ( m_discovering || m_validationWaitWidget->isVisible() ) {
        qLog(Bluetooth) << "Cannot start discovery, device selector is busy";
        return false;
    }

    // disable periodic discovery so it won't interfere with this operation
    setPeriodicDiscoveryEnabled( false );

    if ( !m_local->discoverRemoteDevices() ) {
        qLog(Bluetooth) << "Can't start discovery, last error was:"
            << m_local->lastError();
        return false;
    }

    m_bluetoothStatusIcon->setBlinking( true );
    if ( clear )
        clearDevices();

    m_statusLabel->setText( tr( "Searching..." ) );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_discoveryAction->setText( tr( "Stop searching" ) );
    m_discoveryAction->setIcon( m_stopSearchIcon );
#endif

    m_cancelledDiscovery = false;
    m_discovering = true;

    qLog(Bluetooth) << "Start remote device discovery";

    return true;
}


void QBluetoothDeviceSelector_Private::discoveredDevice( const QBluetoothRemoteDevice &device )
{
    addDeviceToDisplay( device );
}

void QBluetoothDeviceSelector_Private::completedDeviceDiscovery()
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

#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_discoveryAction->setText( tr( "Search again" ) );
    m_discoveryAction->setIcon( m_searchIcon );
#endif

    emit m_parent->deviceDiscoveryCompleted();
}

void QBluetoothDeviceSelector_Private::cancelDeviceDiscovery()
{
    qLog(Bluetooth) << "called QBluetoothDeviceSelector_Private::cancelDeviceDiscovery";

    if ( m_discovering && !m_cancelledDiscovery ) {
        m_cancelledDiscovery = true;
        qLog(Bluetooth) << "cancelDeviceDiscovery";
        if ( !m_local->cancelDiscovery() ) {
            qLog(Bluetooth) << "Error cancelling discovery:"
                    << m_local->lastError();
        }
    }
}

void QBluetoothDeviceSelector_Private::deviceDiscoveryCancelled()
{
    completedDeviceDiscovery();
}

void QBluetoothDeviceSelector_Private::remoteNameUpdated(const QString &dev, const QString &name)
{
    qLog(Bluetooth) << "remoteNameUpdated()" << dev << name;
    QBluetoothAddress addr( dev);
    if ( addr.valid() ) {
        int row = m_devicesTableController->row( addr );
        if ( row != -1 ) {
            m_devicesTableController->refreshDevice( row, *m_local, name );
        }
    }
}

void QBluetoothDeviceSelector_Private::discoveryActionTriggered()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if ( m_discovering ) {
        cancelDeviceDiscovery();
    } else {
        if ( !startDeviceDiscovery( true ) ) {
            QMessageBox::warning( this, tr( "Discovery Error" ),
                    tr( "<P>Unable to start device discovery" ) );
        }
    }
#endif
}

void QBluetoothDeviceSelector_Private::localDeviceError( QBluetoothLocalDevice::Error error, const QString &message )
{
    qLog(Bluetooth) << "QBluetoothDeviceSelector_Private::localDeviceError:"
        << error << message;
}

void QBluetoothDeviceSelector_Private::pairingRequested( int row )
{
    int result = QMessageBox::question( this, tr( "Pair Device" ),
            tr( "<P>Would you like to pair with this device?" ),
            QMessageBox::Yes,
            QMessageBox::No|QMessageBox::Default );

    if ( result != QMessageBox::Yes )
        return;

    // disable periodic discovery so it won't interfere with this operation
    setPeriodicDiscoveryEnabled( false );

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

void QBluetoothDeviceSelector_Private::unpairingRequested( int row )
{
    int result = QMessageBox::question( this, tr( "Pair Device" ),
            tr( "<P>Would you like to unpair from this device?" ),
            QMessageBox::Yes,
            QMessageBox::No|QMessageBox::Default );

    if ( result != QMessageBox::Yes )
        return;

    performPairingOperation( row, QBluetoothPairingAgent::Unpair );
}

void QBluetoothDeviceSelector_Private::performPairingOperation( int row, QBluetoothPairingAgent::Operation op )
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

bool QBluetoothDeviceSelector_Private::serviceProfilesMatch( const QList<QSDPService> services, QSet<QBluetooth::SDPProfile> profiles )
{
    QSetIterator<QBluetooth::SDPProfile> iter( profiles );
    for ( int i=0; i<services.size(); i++ ) {
        while ( iter.hasNext() ) {
            QBluetooth::SDPProfile profile = iter.next();
            if ( QSDPService::isInstance( services[i], profile ) ) {
                return true;
            }
        }
        iter.toFront();
    }
    return false;
}

void QBluetoothDeviceSelector_Private::foundServices( const QSDAPSearchResult &result )
{
    const QList<QSDPService> services = result.services();

    qLog(Bluetooth) << "QBluetoothDeviceSelector_Private::foundServices"
        << services.count();

    if ( !m_deviceUnderValidation.valid() ) {
        qLog(Bluetooth) << "foundServices(): no device to validate,"
            << "validation was cancelled?";
        deviceValidationCompleted();
        return;
    }

    if ( result.errorOccurred() ) {
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

        if ( m_forcePairing && !m_local->isPaired( addr ) ) {
            if ( QBluetoothPairingAgent::pair( addr, *m_local ) != QBluetoothPairingAgent::Success ) {
                QMessageBox::warning( this, tr( "Pairing Error" ),
                    tr( "<P>Failed to pair with this device. Pairing is required, please try again or try another device." ) );
                return;
            }
        }

        // device is now validated, emit the activation signal
        qLog(Bluetooth) << "Device successfully validated";
        emit deviceActivated( addr );

    } else {
        QMessageBox::warning( this, tr( "Service Error" ),
            tr( "<P>Device does not have the necessary services. Try another device." ) );
    }
}

void QBluetoothDeviceSelector_Private::deviceValidationCompleted()
{
    qLog(Bluetooth) << "deviceValidationCompleted()";

    m_validationWaitWidget->hide();
    m_deviceUnderValidation = QBluetoothAddress();  // reset
}

void QBluetoothDeviceSelector_Private::deviceValidationCancelled()
{
    qLog(Bluetooth) << "deviceValidationCancelled";
    m_sdap->cancelSearch();
    deviceValidationCompleted();
}

void QBluetoothDeviceSelector_Private::changedDeviceSelection()
{
    const QBluetoothAddress &addr = m_devicesTableController->selectedDevice();

#ifdef QTOPIA_KEYPAD_NAVIGATION
    if ( isFavorite( addr ) )
        m_favoritesAction->setText( FAVORITES_REMOVE_TEXT );
    else
        m_favoritesAction->setText( FAVORITES_ADD_TEXT );

    enableDeviceActions( addr.valid() );
#endif

    emit deviceSelectionChanged();
}

void QBluetoothDeviceSelector_Private::activatedDevice( int row )
{
    qLog(Bluetooth) << "activatedDevice" << row;

    //if ( m_discovering )
    //    cancelDeviceDiscovery();

    QBluetoothAddress addr = m_devicesTableController->deviceAt(row);

    if ( m_validProfiles.size() == 0 ) {
        // no need to validate device
        qLog(Bluetooth) << "Emitting deviceActivated" << addr.toString();
        emit deviceActivated( addr );

    } else {
        qLog(Bluetooth) << "Validating services for" << addr.toString();

        m_validationWaitWidget->show();
        m_deviceUnderValidation = addr;

        if ( !m_sdap->searchServices( addr, *m_local, QSDPUUID::L2cap) ) {
            QMessageBox::warning( this, tr( "Service Error" ),
                tr( "<P>Unable to verify services for %1. Try another device." )
                .arg( m_devicesTableController->deviceLabelAt( row ) ) );
        }
    }
}

void QBluetoothDeviceSelector_Private::enableDeviceActions( bool enable )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    for ( int i=0; i<m_deviceActions->size(); i++ )
        m_deviceActions->at(i)->setEnabled( enable );
#endif
}

void QBluetoothDeviceSelector_Private::addDeviceToDisplay( const QBluetoothRemoteDevice &device )
{
    if ( !m_filter.filterAcceptsDevice( device ) ) {
        qLog(Bluetooth) << "Device" << device.name()
                << "rejected by filter";
        return;
    }

    m_devicesTableController->addDevice( device, *m_local );
}

void QBluetoothDeviceSelector_Private::clearDevices()
{
    m_devicesTableController->clear();
    m_statusLabel->setText("");
    enableDeviceActions( false );

    m_devicesTableController->redrawTable();
}

void QBluetoothDeviceSelector_Private::addFavorite( const QBluetoothAddress &addr )
{
    // true used as a dummy (unused) value
    m_settings->setValue( SETTINGS_FAVORITES_KEY + "/" + addr.toString(),
                          true );
}

void QBluetoothDeviceSelector_Private::removeFavorite( const QBluetoothAddress &addr )
{
    m_settings->remove( SETTINGS_FAVORITES_KEY + "/" + addr.toString() );
}

QList<QBluetoothAddress> QBluetoothDeviceSelector_Private::favorites()
{
    QSettings settings( "Trolltech", "bluetooth" );
    settings.beginGroup( SETTINGS_FAVORITES_KEY );
    QStringList addressStrings = settings.childKeys();

    QList<QBluetoothAddress> addresses;

    for ( int i=0; i<addressStrings.size(); i++ )
        addresses.append( QBluetoothAddress( addressStrings[i] ) );

    return addresses;
}

bool QBluetoothDeviceSelector_Private::isFavorite( const QBluetoothAddress &addr )
{
    return ( m_settings->value( SETTINGS_FAVORITES_KEY + "/" +
            addr.toString() ).isValid() );
}

void QBluetoothDeviceSelector_Private::loadFavorites()
{
    // load all paired as favourites if the selector is getting loaded
    // for the very first time
    const QString firstLoadKey = "firstLoad";
    QVariant firstLoadValue = m_settings->value( firstLoadKey );
    if ( !firstLoadValue.isValid() ) {
        QBluetoothReply< QList<QBluetoothAddress> > reply =
            m_local->pairedDevices();
        if ( reply.isError() ) {
            qLog(Bluetooth) << "QBluetoothDeviceSelector_Private:"
                << "error getting paired devices";
        } else {
            QList<QBluetoothAddress> pairedDevices = reply.value();
            qLog(Bluetooth) << "Paired devices:" << pairedDevices.size();
            for ( int i=0; i<pairedDevices.size(); i++ )
                addFavorite( pairedDevices[i] );
        }

        m_settings->setValue( firstLoadKey, QVariant( false ) );
    }

    // display all found favourites
    QList<QBluetoothAddress> favoriteAddrs = favorites();
    for ( int i=0; i<favoriteAddrs.size(); i++ ) {
        QBluetoothRemoteDevice device( favoriteAddrs[i] );
        m_local->updateRemoteDevice( device );
        addDeviceToDisplay(device);
    }
}

void QBluetoothDeviceSelector_Private::favoritesActionTriggered()
{
    const QBluetoothAddress &selected = m_devicesTableController->selectedDevice();
    if ( m_favoritesAction->text() == FAVORITES_REMOVE_TEXT ) {
        removeFavorite( selected );
#ifdef QTOPIA_KEYPAD_NAVIGATION
        m_favoritesAction->setText( FAVORITES_ADD_TEXT );
#endif
    } else {
        addFavorite( selected );
#ifdef QTOPIA_KEYPAD_NAVIGATION
        m_favoritesAction->setText( FAVORITES_REMOVE_TEXT );
#endif
    }
}

void QBluetoothDeviceSelector_Private::showRemoteProperties()
{
    QBluetoothAddress selected = m_devicesTableController->selectedDevice();

    for ( int i=0; i<m_devicesTableController->m_displayedDevices.size(); i++ ) {
        if ( m_devicesTableController->m_displayedDevices.at(i).address() ==
                selected ) {
            qLog(Bluetooth) << "Show properties for" << selected.toString();

            if ( !m_deviceInfoDialog ) {
                m_deviceInfoDialog = new RemoteDevicePropertiesDialog(
                        m_local->address(), this );
            }
            const QBluetoothRemoteDevice device =
                    m_devicesTableController->m_displayedDevices.at(i);
            m_deviceInfoDialog->setRemoteDevice( device,
                    m_devicePixmaps.value( device.deviceMajor(), QPixmap() ) );

            QtopiaApplication::execDialog( m_deviceInfoDialog );
            return;
        }
    }
}

void QBluetoothDeviceSelector_Private::updateDeviceDisplay( const QBluetoothAddress &addr )
{
    int row = m_devicesTableController->row( addr );
    if ( row != -1 ) {
        m_devicesTableController->refreshDevice( row, *m_local );
    }
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
QMenu *QBluetoothDeviceSelector_Private::contextMenu()
{
    return QSoftMenuBar::menuFor( this );
}
#endif

void QBluetoothDeviceSelector_Private::initParentUI()
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

void QBluetoothDeviceSelector_Private::initWidgets()
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

void QBluetoothDeviceSelector_Private::initActions()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *menu = contextMenu();

    // properties action
    QAction *detailsAction = new QAction( QIcon( ":icon/view" ),
            tr( "View details..." ), this );
    connect( detailsAction, SIGNAL(triggered()),
             SLOT(showRemoteProperties()) );
    addDeviceAction( detailsAction, false );

    // add/remove favorites action
    m_favoritesAction = new QAction( FAVORITES_ADD_TEXT, this );
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
    menu->addAction( m_discoveryAction );

    // this needs to be set so you know where to insert the separator
    // between the device actions (which are customisable) and the default
    // actions (which are always present)
    m_lastPersistentAction = detailsAction;
#endif
}

void QBluetoothDeviceSelector_Private::setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles )
{
    m_validProfiles = profiles;
}

QSet<QBluetooth::SDPProfile> QBluetoothDeviceSelector_Private::validationProfiles() const
{
    return m_validProfiles;
}

void QBluetoothDeviceSelector_Private::setForcePairing( bool force )
{
    m_forcePairing = force;
}

bool QBluetoothDeviceSelector_Private::forcePairing() const
{
    return m_forcePairing;
}

void QBluetoothDeviceSelector_Private::setFilter( const QBluetoothDeviceSelectorFilter &filter )
{
    m_filter = filter;
}

QBluetoothDeviceSelectorFilter QBluetoothDeviceSelector_Private::filter() const
{
    return m_filter;
}

void QBluetoothDeviceSelector_Private::addDeviceAction( QAction *action, bool isExternalAction )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_deviceActions->append( action );
    action->setEnabled( m_devicesTableController->selectedDevice().valid() );
    action->setVisible( true );

    // if action is externally added action, group them separately from
    // the default device selector actions (i.e. put them above the
    // extra menu separator)
    QMenu *menu = contextMenu();
    if ( isExternalAction ) {
        if ( !m_deviceActionsSeparator ) {
            m_deviceActionsSeparator = menu->insertSeparator(
                m_lastPersistentAction );
        }
        menu->insertAction( m_deviceActionsSeparator, action );
    } else {
        menu->addAction( action );
    }

#endif
}

void QBluetoothDeviceSelector_Private::removeDeviceAction( QAction *action )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    int i = m_deviceActions->indexOf( action );
    if ( i == -1 )
        return;

    m_deviceActions->removeAt( i );

    QMenu *menu = contextMenu();
    menu->removeAction(action);

    // if no more device actions, remove the separator
    if ( m_deviceActions->size() == 0 ) {
        delete m_deviceActionsSeparator;
        m_deviceActionsSeparator = 0;
    }
#endif
}

void QBluetoothDeviceSelector_Private::showEvent( QShowEvent *e )
{
    if ( m_neverShownBefore ) {
        if ( m_loadFavoritesOnShow )
            loadFavorites();

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

QList<QBluetoothAddress> QBluetoothDeviceSelector_Private::displayedDevices() const
{
    return m_devicesTableController->displayedDevices();
}

QBluetoothAddress QBluetoothDeviceSelector_Private::selectedDevice() const
{
    return m_devicesTableController->selectedDevice();
}

void QBluetoothDeviceSelector_Private::initDBus()
{
    QDBusConnection dbc = QDBusConnection::systemBus();
    if (!dbc.isConnected())
        return;
    QDBusInterface iface("org.bluez", "/org/bluez",
                         "org.bluez.Manager", dbc);
    if (!iface.isValid())
        return;

    QDBusReply<QString> reply = iface.call("FindAdapter",
            m_local->address().toString());
    if (!reply.isValid())
        return;

    m_dbusIFace = new QDBusInterface("org.bluez", reply.value(),
            "org.bluez.Adapter", dbc);
    if (!m_dbusIFace->isValid()) {
        delete m_dbusIFace;
        m_dbusIFace = 0;
    }

    dbc.connect(m_dbusIFace->service(), m_dbusIFace->path(),
        m_dbusIFace->interface(), "RemoteNameUpdated",
        this, SIGNAL(remoteNameUpdated(const QString &, const QString &)));
}

// this functionality for starting/stopping the periodic discovery mode
// will be put into QBluetoothLocalDevice later.
bool QBluetoothDeviceSelector_Private::setPeriodicDiscoveryEnabled( bool enabled )
{
    qLog(Bluetooth) << "QBluetoothDeviceSelector_Private::setPeriodicDiscoveryEnabled";

    if (!m_dbusIFace || !m_dbusIFace->isValid()) {
        qLog(Bluetooth) << "setPeriodicDiscoveryEnabled: no DBUS interface, can't control periodic discovery";
        return false;
    }

    QDBusReply<bool> queryReply = m_dbusIFace->call("IsPeriodicDiscovery");
    if ( !queryReply.isValid() )
        return false;

    if ( queryReply.value() == enabled )
        return false;

    QDBusReply<void> reply = enabled ? m_dbusIFace->call("StartPeriodicDiscovery") :
            m_dbusIFace->call("StopPeriodicDiscovery");

    if ( !reply.isValid() ) {
        qLog(Bluetooth) << "Unable to enable/disable periodic discovery:"
            << reply.error().name() << reply.error().message();
        return false;
    }

    if ( enabled )
        m_periodicDiscoveryDisabled = false;
    else
        m_periodicDiscoveryDisabled = true;

    return true;
}

bool QBluetoothDeviceSelector_Private::restartPeriodicDiscovery()
{
    qLog(Bluetooth) << "restartPeriodicDiscovery";

    // only restart the periodic discovery if we disabled it
    if ( m_periodicDiscoveryDisabled ) {
        qLog(Bluetooth) << "restartPeriodicDiscovery: continue";
        return setPeriodicDiscoveryEnabled( true );
    }
    return false;
}


/*!
    \class QBluetoothDeviceSelector
    \brief The QBluetoothDeviceSelector class allows the user to perform a bluetooth device discovery and select a particular device.

    The QBluetoothDeviceSelector class allows the user to perform a bluetooth
    device discovery and select a particular device.

    When a device selector is first displayed, it shows all favorite devices,
    and allows a device discovery to be activated in order to display remote
    devices found in the vicinity. If the system has no favorite devices, a
    device discovery will be started immediately.

    A device selector can be configured to only display certain devices,
    using the setFilter() function. Also, if one or more validation profiles
    have been set using setValidationProfiles(), the deviceActivated() signal
    will only be emitted if the activated device has a service that matches
    the validation profile(s).

    Custom context menu actions can be added to a device selector through
    addDeviceAction(). (This is only available if keypad navigation is
    available.)

    The getRemoteDevice() static method is the easiest way to run a device
    selector. It runs a modal device selection dialog, then returns the
    address of the device that was activated by the user.

    By default, a device selector dialog will close as soon as a valid device
    is activated by the end user. If you prefer to keep the dialog open when
    a device is activated, disconnect the appropriate signal, like this:

    \code
    QBluetoothDeviceSelector selector;
    QObject::disconnect( &selector, SIGNAL(deviceActivated(const QBluetoothAddress &)),
                         &selector, SLOT(accept()) );
    \endcode

    \ingroup qtopiabluetooth
    \sa QBluetoothDeviceSelectorFilter, deviceActivated()
  */

/*!
    Constructs a QBluetoothDeviceSelector with the given parent widget
    \a parent and the window flags \a flags. If \a loadFavoritesOnShow is
    \c true, favorite devices (as determined by user settings) will be
    listed in the device selector when it is first displayed.
 */
QBluetoothDeviceSelector::QBluetoothDeviceSelector( bool loadFavoritesOnShow, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    m_private = new QBluetoothDeviceSelector_Private(
            QBluetoothLocalDevice().address(), loadFavoritesOnShow, this );
}

/*!
    Constructs a QBluetoothDeviceSelector with the given parent widget
    \a parent and the window flags \a flags. Favorite devices will be listed
    in the device selector when it is first displayed.
 */
QBluetoothDeviceSelector::QBluetoothDeviceSelector( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    m_private = new QBluetoothDeviceSelector_Private(
            QBluetoothLocalDevice().address(), true, this );
}

/*!
    Destructs a QBluetoothDeviceSelector object.
 */
QBluetoothDeviceSelector::~QBluetoothDeviceSelector()
{
}

/*!
    A convenience method for allowing an end user to select a bluetooth
    device.

    This opens a device selector with the validation profiles \a profiles,
    the device filter \a filter, and the parent \a parent. If \a
    loadFavoritesOnShow is \c true, the "Favorite devices" (as determined by
    user settings) will be shown in the device selector when it is first
    displayed. (Note that the displayed favorites are also subject to the
    given filter and validation profiles.)

    Returns the address of the remote device that was activated, or a
    an invalid address ("00:00:00:00:00:00") if the dialog was cancelled.

    \sa setValidationProfiles(), QBluetoothDeviceSelectorFilter
 */
QBluetoothAddress QBluetoothDeviceSelector::getRemoteDevice(
                                QSet<QBluetooth::SDPProfile> profiles,
                                const QBluetoothDeviceSelectorFilter &filter,
                                bool loadFavoritesOnShow,
                                QWidget *parent )
{
    QBluetoothDeviceSelector selector( loadFavoritesOnShow, parent );
    selector.setFilter( filter );
    selector.setValidationProfiles( profiles );

    if ( QtopiaApplication::execDialog( &selector ) == QDialog::Accepted )
        return selector.selectedDevice();
    else
        return QBluetoothAddress();
}

/*!
    Set the accepted SDP profiles to \a profiles.

    When this is set, a device can only be activated if it has at least one
    service that matches one of the given profiles.

    \sa validationProfiles(), deviceActivated()
 */
void QBluetoothDeviceSelector::setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles )
{
    m_private->setValidationProfiles( profiles );
}

/*!
    Returns the current SDP profile filters.

    \sa setValidationProfiles()
 */
QSet<QBluetooth::SDPProfile> QBluetoothDeviceSelector::validationProfiles() const
{
    return m_private->validationProfiles();
}

/*!
    Sets whether devices must be paired in order to be successfully
    activated by the user.

    If \a force is true, and the user activates a device that is not paired,
    this selector will start the pairing process. If the pairing process
    fails, the deviceActivated() signal will not be emitted.

    \sa forcePairing(), deviceActivated()
 */
void QBluetoothDeviceSelector::setForcePairing( bool force )
{
    m_private->setForcePairing( force );
}

/*!
    Returns whether devices must be paired in order to be successfully
    activated by the user. By default, this value is \c false.

    \sa setForcePairing(), deviceActivated()
 */
bool QBluetoothDeviceSelector::forcePairing() const
{
    return m_private->forcePairing();
}

/*!
    Sets the device selector to filter displayed devices using the filter
    \a filter.

    \sa filter()
 */
void QBluetoothDeviceSelector::setFilter( const QBluetoothDeviceSelectorFilter &filter )
{
    m_private->setFilter( filter );
}

/*!
    Returns the filter used by the device selector to filter displayed devices.

    \sa setFilter()
 */
QBluetoothDeviceSelectorFilter QBluetoothDeviceSelector::filter() const
{
    return m_private->filter();
}

/*!
    Add a device action \a action to the top of the context menu of this
    selector. (This is only available if keypad navigation is available.)

    Device actions are enabled when a device is selected, and disabled when
    no devices are selected. They are hidden altogether during a device
    discovery.
 */
void QBluetoothDeviceSelector::addDeviceAction( QAction *action )
{
    m_private->addDeviceAction( action, true );
}

/*!
    Remove a device action \a action from the context menu of this selector.
 */
void QBluetoothDeviceSelector::removeDeviceAction( QAction *action )
{
    m_private->removeDeviceAction( action );
}

/*!
    Returns a list of the devices that are currently displayed in the selector.
 */
QList<QBluetoothAddress> QBluetoothDeviceSelector::displayedDevices() const
{
    return m_private->displayedDevices();
}

/*!
    Returns the address of the currently selected device.

    Note that the SDP profile filter (set through setValidationFilters())
    and the enforcment of device pairing (set through setForcePairing()) are
    only applied when a device is activated, and not when they are merely
    selected. Therefore, if these conditions have been set, the device that is
    returned by this method may not have the desired attributes.

    \sa deviceSelectionChanged(), deviceActivated()
 */
QBluetoothAddress QBluetoothDeviceSelector::selectedDevice() const
{
    return m_private->selectedDevice();
}

/*!
    Triggers a new device discovery process within the device selector. If
    \a clear is \c true, any devices that are currently displayed in the
    device selector will be cleared from the screen before the discovery is
    initiated.

    Returns whether a device discovery was successfully started. The
    discovery will not start if a discovery or pairing process is currently in
    progress.

    This method should be carefully used by the programmer, to ensure that
    there is no disruption to user interaction with the device selector. It
    would be confusing for the user if a new discovery is started at the
    moment that the user is prepared to select or activate a particular
    device. However, this method could be useful if, for example, the user
    has interrupted the device discovery process (perhaps by starting a
    different operation on the local device) and the programmer wants to give
    the illusion of "continuing" a discovery process if this other process
    is cancelled, by calling \c startDeviceDiscovery(false).


    \sa deviceDiscoveryStarted(), deviceDiscoveryCompleted()
 */
bool QBluetoothDeviceSelector::startDeviceDiscovery( bool clear )
{
    return m_private->startDeviceDiscovery( clear );
}

/*!
    \fn void QBluetoothDeviceSelector::deviceSelectionChanged()

    This signal is emitted whenever the device selection has changed.
 */

/*!
    \fn void QBluetoothDeviceSelector::deviceActivated( const QBluetoothAddress &addr )

    This signal is emitted when a device is successfully activated. The \a addr
    value indicates the address of the activated device.

    If a device is activated by the user but its services do not match the
    current SDP profile filter, this signal will not be activated.

    \sa setValidationProfiles()
 */

/*!
    \fn void QBluetoothDeviceSelector::deviceDiscoveryStarted()

    This signal is emitted whenever a device discovery has started within the
    device selector. The discovery could have been started by the user or by a
    call to QBluetoothDeviceSelector::startDeviceDiscovery().
 */

/*!
    \fn void QBluetoothDeviceSelector::deviceDiscoveryCompleted()

    This signal is emitted whenever a device discovery has completed within the
    device selector. The discovery could have been started by the user or by a
    call to QBluetoothDeviceSelector::startDeviceDiscovery().
 */


