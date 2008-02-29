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

#include "networkui.h"
#include "addnetwork.h"
#ifdef QTOPIA_CELL
#include "wapui.h"
#endif
#ifdef QTOPIA_VPN
#include "vpnui.h"
#endif

#include <QFile>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QTimer>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <QAction>
#include <QMenu>
#include <qsoftmenubar.h>
#else
#include <QPushButton>
#endif
#include <QDSData>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopianetwork.h>
#include <qtopianetworkinterface.h>
#include <qtranslatablesettings.h>
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>
#include <qnetworkstate.h>
#include <qnetworkdevice.h>
#include <qvaluespace.h>

#include <qsoftmenubar.h>

#ifdef QTOPIA_CELL
#include <qotareader.h>
#include <qsmsmessage.h>
#endif

#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>


class NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    NetworkMonitor( NetworkWidgetItem* item );
public slots:
    void setDefaultGateway( const QString& configFile );
    void updateStatus( int state );

private slots:
    void update();
    void deviceStateChanged(QtopiaNetworkInterface::Status,bool);
private:
    NetworkWidgetItem* netItem;
    QTimer* pending;
    QString config;
    QNetworkDevice* dev;
};


class NetworkWidgetItem : public QTableWidgetItem
{
public:
    NetworkWidgetItem(QTableWidgetItem* name) :
        QTableWidgetItem(), state(QtopiaNetworkInterface::Unknown),
        monitor(0), nameItem( name ), defaultGateway( false ), lifeExtention( false )
    {
        monitor = new NetworkMonitor( this );
        setStatus( state );
    }

    virtual ~NetworkWidgetItem()
    {
        if ( monitor )
            delete monitor;
        monitor = 0;
    }

    void setStatus(const QtopiaNetworkInterface::Status status)
    {
        QString p;

        QtopiaNetwork::Type itemType = QtopiaNetwork::toType( config() );
        //TODO: this is bad, it should determine the path
        //dynamically and not statically
        if ( itemType & QtopiaNetwork::Dialup )
            p = "Network/dialup/dialup-";
        else if ( itemType & QtopiaNetwork::GPRS )
            p = "Network/dialup/GPRS-";
        else if ( itemType & QtopiaNetwork::WirelessLAN )
            p = "Network/lan/WLAN-";
        else if ( itemType & QtopiaNetwork::LAN )
            p = "Network/lan/LAN-";
        else if ( itemType & QtopiaNetwork::Bluetooth )
            p = "bluetooth/bluetooth-";

        state = status;
        switch (status) {
            case QtopiaNetworkInterface::Unavailable:
                setText(NetworkUI::tr("Unavailable"));
                nameItem->setIcon(QPixmap(":icon/"+p+"notavail"));
                break;
            case QtopiaNetworkInterface::Down:
                setText(NetworkUI::tr("Offline"));
                nameItem->setIcon(QPixmap(":icon/"+p+"offline"));
                break;
            case QtopiaNetworkInterface::Up:
                setText(NetworkUI::tr("Online"));
                nameItem->setIcon(QPixmap(":icon/"+p+"online"));
                break;
            case QtopiaNetworkInterface::Pending:
                setText(NetworkUI::tr("Pending"));
                if ( onlinePicShown )
                    nameItem->setIcon(QPixmap(":icon/"+p+"online"));
                else
                    nameItem->setIcon(QPixmap(":icon/"+p+"offline"));
                onlinePicShown = !onlinePicShown;
                break;
            case QtopiaNetworkInterface::Demand:
                setText(NetworkUI::tr("On Demand"));
                nameItem->setIcon(QPixmap(":icon/"+p+"demand"));
                break;
            default:
                setText(NetworkUI::tr("Unknown"));
                nameItem->setIcon(QPixmap(":icon/"+p+"unknown"));
                break;
        }
    }

    QtopiaNetworkInterface::Status status() const
    {
        return state;
    }

    bool isDefaultGateway() const
    {
        return defaultGateway;
    }

    void setExtendedLifeTime( bool isExtended )
    {
        lifeExtention = isExtended;
    }
    
    bool extendedLifeTime( )
    {
        return lifeExtention;
    }

private:
    void setDefaultGateway( bool isGateway )
    {
        defaultGateway = isGateway;
    }


    QString config() const
    {
        if ( nameItem )
            return nameItem->data(Qt::UserRole).toString();
        return QString();
    }

private:
    QtopiaNetworkInterface::Status state;
    NetworkMonitor* monitor;
    QTableWidgetItem* nameItem;
    bool defaultGateway;
    bool onlinePicShown;
    bool lifeExtention;

    friend class NetworkMonitor;
};

/*!
  \internal

  class NetworkMonitor
*/
NetworkMonitor::NetworkMonitor(  NetworkWidgetItem* item )
    : QObject(), netItem(item), pending( 0 )
{
    QNetworkState *netState = new QNetworkState( this );
    setDefaultGateway( netState->gateway() );
    connect( netState, SIGNAL(defaultGatewayChanged(QString,const QNetworkInterface&)),
            this, SLOT(setDefaultGateway(const QString&)) );

    pending = new QTimer( this );
    connect( pending, SIGNAL(timeout()), this, SLOT(update()) );

    dev = new QNetworkDevice( netItem->config(), this );
    deviceStateChanged( dev->state(), false );
    connect( dev, SIGNAL(stateChanged(QtopiaNetworkInterface::Status,bool)),
            this, SLOT(deviceStateChanged(QtopiaNetworkInterface::Status,bool)) );

}

void NetworkMonitor::updateStatus( int state )
{
    QtopiaNetworkInterface::Status s = (QtopiaNetworkInterface::Status) state;
    if ( netItem->status() == s )
        return;


    if ( (s == QtopiaNetworkInterface::Pending || s== QtopiaNetworkInterface::Demand )
            && (netItem->status() == QtopiaNetworkInterface::Down) ) {
        /*
            The current limitation is that a device must be in the Pending, Demand or Up state in order to extend 
            its life time. Hence we just set the life time flag whenever we discover a state transition from Down state
            to any of the other states mentioned above. Consequently we have to ensure that the current device
            was actually started by netsetup and not any other application.
        
            All NetworkWidgetItems with the extendedLifeTime flag were started by NetworkUI.
            This way we can keep track of who started the network device as only NetworkUI can set the extended 
            life time flag. It prevents that devices gain the extention just because the netsetup application 
            happens to be running in the background when another application starts a new device/interface.
        */
        if ( netItem->extendedLifeTime() ) {
            // when netsetup starts an interface it always sets the extended life time flag for the session
            QtopiaNetwork::extendInterfaceLifetime( netItem->config(), true );
            // reset the flag
            netItem->setExtendedLifeTime( false );
        }
    }

    netItem->setStatus( s );
    switch( state ) {
        default:
            netItem->setDefaultGateway( false );
        case QtopiaNetworkInterface::Up:
        case QtopiaNetworkInterface::Demand:
            if ( pending->isActive() )
                pending->stop();
            break;
        case QtopiaNetworkInterface::Pending:
            if ( !pending->isActive() )
                pending->start( 600 );
            break;
    }
}

void NetworkMonitor::deviceStateChanged( QtopiaNetworkInterface::Status s, bool error )
{

    if ( qLogEnabled(Network) && error ) {
        qLog(Network) <<"******" << netItem->config() << dev->errorString() << dev->interfaceName();
    }
    updateStatus( s );
}

void NetworkMonitor::setDefaultGateway( const QString& configFile )
{
    netItem->setDefaultGateway( netItem->config() == configFile );
}

void NetworkMonitor::update()
{
    netItem->setStatus( QtopiaNetworkInterface::Pending ); //forces picture change
}



NetworkUI::NetworkUI( QWidget * parent, Qt::WFlags fl)
    : QDialog( parent, fl )
{
    QValueSpaceItem* devSpace = new QValueSpaceItem( "/Network/Interfaces", this );
    connect( devSpace, SIGNAL(contentsChanged()), this, SLOT(updateConfig()) );

    init();

    new NetworkSetupService(this);
}

NetworkUI::~NetworkUI()
{
}

void NetworkUI::init()
{
    setWindowTitle( tr("Internet") );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );

    tabWidget = new QTabWidget( this );
    //tabWidget->setTabShape( QTabWidget::Triangular );

    QWidget* dataPage = new QWidget( );
    QVBoxLayout *vb = new QVBoxLayout( dataPage );
    vb->setSpacing( 4 );
    vb->setMargin( 2 );

    QLabel *label = new QLabel( tr("These are the network services you currently have available."));
    label->setWordWrap( true );
    vb->addWidget(label);

    table = new QTableWidget();
    table->setColumnCount( 2 );
    table->setEditTriggers( QAbstractItemView::NoEditTriggers  );
    table->setAlternatingRowColors( true );
    table->setShowGrid( false );
    table->horizontalHeader()->setStretchLastSection( true );
    table->verticalHeader()->hide();
    table->horizontalHeader()->hide();
    table->setSelectionBehavior( QAbstractItemView::SelectRows );
    table->setSelectionMode( QAbstractItemView::SingleSelection );
    vb->addWidget( table );
    connect(table, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
              this, SLOT(updateActions()));
    connect(table, SIGNAL(itemChanged(QTableWidgetItem*)),
             this, SLOT(updateActions()));

#ifdef QTOPIA_KEYPAD_NAVIGATION
    connect( table, SIGNAL(itemActivated(QTableWidgetItem*)),
            this, SLOT(serviceSelected()));

    contextMenu = QSoftMenuBar::menuFor( dataPage );

    a_add = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect( a_add, SIGNAL( triggered(bool) ), this, SLOT( addService() ) );
    contextMenu->addAction(a_add);

    a_remove = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    connect( a_remove, SIGNAL(triggered(bool)), this, SLOT(removeService()) );
    contextMenu->addAction(a_remove);

    a_gateway = new QAction( QIcon(":icon/defaultgateway"), tr("Default Gateway"), this );
    connect( a_gateway, SIGNAL( triggered(bool) ), this, SLOT(setGateway()) );
    contextMenu->addAction( a_gateway );

    a_props = new QAction( QIcon(":icon/settings"), tr("Properties..."), this );
    connect( a_props, SIGNAL(triggered(bool)), this, SLOT(doProperties()) );
    a_props->setData( QString() );
    contextMenu->addAction(a_props);
#else
    QGridLayout* grid = new QGridLayout();
    vb->addItem( grid );
    grid->setSpacing( 4 );
    grid->setMargin( 4 );
    startPB = new QPushButton( tr("Start/Stop"), dataPage );
    connect( startPB, SIGNAL(clicked()), this, SLOT(serviceSelected()));
    grid->addWidget( startPB, 0, 0 );

    propPB = new QPushButton( tr("Properties..."), dataPage );
    propPB->setIcon( QIcon(":icon/settings") );
    connect( propPB, SIGNAL(clicked()), this, SLOT(doProperties()));
    grid->addWidget( propPB, 0, 1 );

    addPB = new QPushButton( tr("New..."), dataPage );
    addPB->setIcon( QIcon(":icon/new") );
    connect( addPB, SIGNAL(clicked()), this, SLOT(addService()));
    grid->addWidget( addPB, 1, 0 );

    removePB = new QPushButton( tr("Delete"), dataPage );
    removePB->setIcon( QIcon(":icon/trash") );
    connect( removePB, SIGNAL(clicked()), this, SLOT(removeService()));
    grid->addWidget( removePB, 1, 1 );

    //TODO add gateway option
#endif
    tabWidget->addTab( dataPage, tr("Data") );
    tabWidget->setTabIcon( 0, QIcon(":icon/internet") );

#ifdef QTOPIA_CELL
    wapPage = new WapUI();
    tabWidget->addTab( wapPage, tr("WAP") );
    tabWidget->setTabIcon( 1, QIcon(":icon/wap") );
#endif

#ifdef QTOPIA_VPN
    vpnPage = new VpnUI();
    tabWidget->addTab( vpnPage, tr("VPN") );
#endif //QTOPIA_VPN

    layout->addWidget( tabWidget );
    connect( tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
    tabWidget->setCurrentIndex( 0 );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    table->installEventFilter( this );
#ifdef QTOPIA_CELL
    wapPage->wapList->installEventFilter( this );
#endif
#ifdef QTOPIA_VPN
    vpnPage->vpnList->installEventFilter( this );
#endif //QTOPIA_VPN
#endif //QTOPIA_KEYPAD_NAVIGATION

    // call update twice because the first call will create the
    // remote signal connections and the second call updates the states
    updateConfig();

    if ( table->rowCount() )
        table->selectRow( 0 );
}

void NetworkUI::updateConfig()
{
    QStringList configs = QtopiaNetwork::availableNetworkConfigs();
    QList<int> invalidRows;
    for (int i = 0; i < table->rowCount(); i++) {
        QTableWidgetItem* item = table->item( i, 0 );
        if ( !item ) {
            invalidRows.append( i );
            continue;
        }
        if (!configs.contains(item->data(Qt::UserRole).toString())) {
            invalidRows.append( i );
            continue;
        } else {
            configs.removeAll( item->data(Qt::UserRole).toString() );            // the name may have changed
            QTranslatableSettings cfg( item->data(Qt::UserRole).toString(),
                    QSettings::IniFormat );
            cfg.beginGroup("Info");
            item->setText(cfg.value("Name").toString());
            cfg.endGroup();

        }
    }

    //remove deleted ifaces
    foreach( int row, invalidRows ) {
        qLog(UI) <<  "Removing " << row;
        if ( row == 0 && table->rowCount() >= 1 )
            table->setCurrentCell( 1, 0 );
        else if ( row == table->rowCount()-1 && row >= 0)
            table->setCurrentCell( row-1, 0 );
        table->removeRow( row);

    }

    //add new ifaces
    foreach( QString newConfig, configs ) {
        QTranslatableSettings cfg( newConfig, QSettings::IniFormat );
        QTableWidgetItem* nameItem = new QTableWidgetItem(
                cfg.value("Info/Name").toString() );
        nameItem->setData(Qt::UserRole, newConfig);
        QString type = cfg.value("Info/Type").toString();
        NetworkWidgetItem* statusItem = new NetworkWidgetItem( nameItem );

        int rowCount = table->rowCount() + 1;
        table->setRowCount( rowCount );
        table->setItem(rowCount-1, 0, nameItem);
        table->setItem(rowCount-1, 1, statusItem);
        table->resizeRowToContents( rowCount - 1 );
    }

    table->sortItems( 0 );
    table->resizeColumnToContents(1);
    table->setEditFocus( true );

    updateActions();
}


void NetworkUI::setCurrentTab(int idx)
{
    tabWidget->setCurrentIndex(idx);
}


// update status of buttons and actions
void NetworkUI::updateActions()
{
    if (!table->rowCount() || table->currentRow() < 0) {
#ifdef QTOPIA_KEYPAD_NAVIGATION
        a_remove->setVisible( false );
        a_props->setVisible( false );
        a_gateway->setVisible( false );
#else
        startPB->setEnabled( false );
        propPB->setEnabled( false );
        removePB->setEnabled( false );
#endif
        updateExtraActions( QString(), QtopiaNetworkInterface::Unknown );
    } else {
        QTableWidgetItem *nameItem = table->item( table->currentRow(), 0 );
        QTableWidgetItem *statusItem = table->item( table->currentRow(), 1 );
        if (!nameItem || !statusItem ){
#ifdef QTOPIA_KEYPAD_NAVIGATION
            a_remove->setVisible( false );
            a_props->setVisible( false );
            a_gateway->setVisible( false );
#else
            startPB->setEnabled( false );
            propPB->setEnabled( false );
            removePB->setEnabled( false );
#endif
            return;
        }
        NetworkWidgetItem *netItem = (NetworkWidgetItem*)statusItem;
        QtopiaNetworkInterface::Status type = netItem->status();
#ifdef QTOPIA_KEYPAD_NAVIGATION
        a_gateway->setVisible( false );
        switch ( type ) {
            case QtopiaNetworkInterface::Unknown:
                a_props->setVisible( false );
                a_remove->setVisible( true );
                break;
            case QtopiaNetworkInterface::Up:
            case QtopiaNetworkInterface::Demand:
                a_gateway->setVisible( !netItem->isDefaultGateway() );
            case QtopiaNetworkInterface::Pending:
                a_props->setVisible( false );
                a_remove->setVisible( false );
                break;
            default:
                a_props->setVisible( true );
                a_remove->setVisible( true );
                break;
        }
        if ( type == QtopiaNetworkInterface::Unknown || type == QtopiaNetworkInterface::Unavailable )
            QSoftMenuBar::setLabel( table, Qt::Key_Select, QSoftMenuBar::NoLabel );
        else
            QSoftMenuBar::setLabel( table, Qt::Key_Select, QSoftMenuBar::Select );

#else
        switch( type ){
            case  QtopiaNetworkInterface::Down:
                startPB->setEnabled( true );
                propPB->setEnabled( true );
                removePB->setEnabled( true );
                break;
            case  QtopiaNetworkInterface::Up:
            case  QtopiaNetworkInterface::Pending:
            case  QtopiaNetworkInterface::Demand:
                startPB->setEnabled( true );
                propPB->setEnabled( true );
                removePB->setEnabled( false );
                break;
            case  QtopiaNetworkInterface::Unavailable:
                propPB->setEnabled( true );
                startPB->setEnabled( false );
                removePB->setEnabled( true );
                break;
            default:
                removePB->setEnabled( true );
                propPB->setEnabled( false );
                startPB->setEnabled( false );
                break;
        }
#endif
        updateExtraActions( nameItem->data( Qt::UserRole ).toString(), type );
    }
}

void NetworkUI::updateExtraActions( const QString& config, QtopiaNetworkInterface::Status newState )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    //reset all extra actions
    QList<QString> list = actionMap.keys();
    foreach( QString key, list ) {
        QList< QPointer<QAction> > actionList = actionMap.values( key );
        foreach( QPointer<QAction> action, actionList ) {
            if ( config == key && action ) {
                switch( newState ) {
                    case QtopiaNetworkInterface::Unavailable:
                    case QtopiaNetworkInterface::Unknown:
                        action->setVisible( false );
                        break;
                    default:
                        action->setVisible( true );
                        break;
                }
            } else if ( action ) {
                action->setVisible( false );
            }
        }
    }

    if ( config.isEmpty() )
       return;
    //new interface -> add additional actions
    if ( !actionMap.contains( config ) ) {
        QPointer<QtopiaNetworkInterface> iface =  QtopiaNetwork::loadPlugin( config );
        if (iface) {
            QStringList actions = iface->configuration()->types();
            for( int i = 1; i < actions.count(); i++ ) {
                QPointer<QAction> act = new QAction( actions.at( i ) + "..." , this );
                connect( act, SIGNAL(triggered(bool)), this, SLOT(doProperties()) );
                act->setData( actions.at(i) );
                actionMap.insert( config, act );
                 switch( newState ) {
                    case QtopiaNetworkInterface::Unavailable:
                    case QtopiaNetworkInterface::Unknown:
                        act->setVisible( false );
                        break;
                    default:
                        act->setVisible( true );
                        break;
                }
                contextMenu->addAction( act );
            }
        }
    }
    table->setEditFocus( true );
#else
    //TODO
#endif
}


void NetworkUI::addService(const QString& newConfig)
{
    QPointer<QtopiaNetworkInterface> iface =  QtopiaNetwork::loadPlugin( newConfig );
    if (iface) {
        QDialog* dlg = iface->configuration()->configure( this );
        if ( dlg ) {
            dlg->showMaximized();
            if ( dlg->exec() == QDialog::Accepted ) {
               qLog(Network) << "New Network interface configuration saved";
            } else {
                QFile::remove(newConfig);
            }
            delete dlg;
            delete iface;
        } else {
            qLog(Network) << newConfig << "doesn't have configure dialog";
        }
    } else {
        QFile::remove(newConfig);
    }

    //tell network server that there is a new interface
    updateIfaceStates();

}

void NetworkUI::addService()
{
    AddNetworkUI dlg(this);
    dlg.showMaximized();
    if ( dlg.exec() ) {
        QDir settingsDir(QtopiaNetwork::settingsDir());
        QString templateConfig = dlg.selectedConfig();
        QString filename;
        int n=0;
        QString basename = QFileInfo( templateConfig ).baseName();
        do {
            filename = settingsDir.filePath( basename +
                    QString::number(n++) + ".conf" );
        } while ( QFile::exists( filename ) );
        //copy template config to new config file
        {
            QFile input( templateConfig );
            if ( !input.open(QIODevice::ReadOnly) )
                return;
            QFile output(filename);
            if ( !output.open(QIODevice::WriteOnly) )
                return;
            output.write(input.readAll());
        }

        addService(filename);
   }
}

void NetworkUI::removeService()
{
    QTableWidgetItem* item = table->item( table->currentRow(), 0 );
    if ( !item )
        return;

    QString config = item->data( Qt::UserRole ).toString();
    QString name = item->text();
    if ( Qtopia::confirmDelete( this, tr("Network"), name ) ) {
        //we may have loaded the plugin when calling doProperties
        //now that this configuration is deleted, get rid of it
        QtopiaNetworkInterface* plugin = QtopiaNetwork::loadPlugin( config );
        if ( plugin )
            delete plugin;

        //this will trigger the deletion of the interface instance
        //loaded by the network server because the config file doesn't
        //exist anymore
        //we don't have to use QtopiaNetwork::privilegedInterfaceStop() because
        //we cant delete the interface until it's been stopped anyway
        QtopiaNetwork::stopInterface( config, true );
    }
}

void NetworkUI::doProperties()
{
    QTableWidgetItem* item = table->item( table->currentRow(), 0 );
    if ( !item )
        return;

    QString config = item->data( Qt::UserRole ).toString();
    if (config.isEmpty())
        return;

    QtopiaNetworkInterface* plugin = QtopiaNetwork::loadPlugin( config );
    if ( !plugin )
        return;

    QString type;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction* a = qobject_cast<QAction*>(sender());
    if ( a )
        type = a->data().toString();
#else
    //TODO
#endif

    QDialog * dialog = plugin->configuration()->configure( this, type );
    if ( !dialog )
        return;

    dialog->showMaximized();
    if ( dialog->exec() == QDialog::Accepted ) {
        qLog(Network) << "Network interface has been configured.";
        QTimer::singleShot(500, this, SLOT(updateIfaceStates()));
    }

    if ( dialog )
        delete dialog;
}

void NetworkUI::setGateway()
{
    QTableWidgetItem* item = table->item( table->currentRow(), 0 );
    if ( !item)
        return;

    QString config = item->data( Qt::UserRole ).toString();
    if (config.isEmpty())
        return;
    if ( QNetworkState().gateway() != config )
        QtopiaNetwork::setDefaultGateway( config );
}

// starts and/or stops the selected interface
void NetworkUI::serviceSelected()
{
    int row = table->currentRow();

    if ( row < 0 )
        qLog(UI) << "No interface selected";

    QTableWidgetItem* item = table->item( table->currentRow(), 0 );
    if (!item)
        return;
    QTableWidgetItem* statusItem = table->item( table->currentRow(), 1 );
    if (!statusItem)
        return;
    NetworkWidgetItem* netItem = (NetworkWidgetItem*)statusItem;

    QString config = item->data( Qt::UserRole ).toString();

    switch (netItem->status())
    {
        case QtopiaNetworkInterface::Down:
            //the extended life time flag will extend configs life time
            //as soon as we change from Down to another state
            netItem->setExtendedLifeTime( true );
            QtopiaNetwork::startInterface( config );
            break;
        case QtopiaNetworkInterface::Up:
        case QtopiaNetworkInterface::Pending:
        case QtopiaNetworkInterface::Demand:
            //usually we would use QtopiaNetwork::stopInterface( config ) but
            //here we must ensure that it is stopped no matter what the session management
            //suggests
            QtopiaNetwork::privilegedInterfaceStop( config );
            break;
        case QtopiaNetworkInterface::Unavailable:
        case QtopiaNetworkInterface::Unknown:
        default:
            break;
    }
}

#ifdef QTOPIA_CELL
void NetworkUI::otaDatagram( QOtaReader *reader, const QByteArray& data,
                             const QString& sender )
{
    if ( reader ) {
        QOtaCharacteristicList *list = reader->parseCharacteristics( data );
        QString xml = reader->toXml( data );
        qLog(Network) << xml;
        delete reader;
        if ( list ) {
            const QtopiaNetworkProperties prop = list->toConfig();
            delete list;
            applyRemoteSettings( sender, prop );
        }
    }
}

void NetworkUI::applyRemoteSettings( const QString& from, const QtopiaNetworkProperties& prop )
{
    // Dump the settings to the debug stream.
    prop.debugDump();

    QString bearer = prop.value( "Wap/Bearer" ).toString();
    if ( !(bearer == "GPRS" || bearer == "GSM-GPRS"
            || bearer == "GSM-CSD" || bearer == "GSM/CSD") )
    {
        qLog(Network) << "Received OTA message for unknown bearer. Ignoring message";
        return;
    }

    // Ask the user if they wish to apply the settings.
    QString name = prop.value("Info/Name").toString();

    //get all wap configs
    QString path = Qtopia::applicationFileName("Network", "wap");
    QDir configDir(path);
    configDir.mkdir(path);
    QStringList files = configDir.entryList( QStringList("*.conf") );
    QStringList wapConfigs;
    foreach( QString item, files )
        wapConfigs.append( configDir.filePath( item ) );

    //get all dialup configs
    QStringList dialupConfigs = QtopiaNetwork::availableNetworkConfigs(
            QtopiaNetwork::Dialup);

    //ask user for permission to apply changes
    QString wapcfg;
    QString dialupcfg;
    QString text;
    if ( name.isEmpty() ) {
        // Set a default name based on the sender's phone number if necessary.
        name = from;
        if ( name.length() > 0 && name[0] == '+' )
            name = name.mid(1);
        text = tr("<qt>Received network settings.  Do you wish to apply them?</qt>");
    } else {
        //find existing wap config
        foreach( QString iter, wapConfigs ) {
            QTranslatableSettings cfg(iter, QSettings::IniFormat);
            if ( cfg.value("Info/Name").toString() == name ) {
                wapcfg = iter;
                break;
            }
        }
        //find existing dialup config
        foreach( QString iter, dialupConfigs ) {
            QTranslatableSettings cfg( iter, QSettings::IniFormat );
            if ( cfg.value("Info/Name").toString() == name ) {
                if ( (cfg.contains("Serial/APN") && prop.contains("Serial/APN") )
                        || (cfg.contains("Serial/Phone") && prop.contains("Serial/Phone") ) ){
                    dialupcfg = iter;
                    break;
                }
            }
        }

        if ( wapcfg.isEmpty() && dialupcfg.isEmpty() )
            text = tr("<qt>Received network settings for \"%1\".  Do you wish to apply them?</qt>").arg(name);
        else
            text = tr("<qt>Received updated network settings for \"%1\".  Do you wish to apply them?</qt>").arg(name);
    }

    QMessageBox box( tr("Internet"), text, QMessageBox::NoIcon,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No, QMessageBox::NoButton );
    if ( box.exec() != QMessageBox::Yes )
        return;

    qLog(Network) << "Saving OTA message";
    //apply changes
    if ( wapcfg.isEmpty() ) {
        QString path = Qtopia::applicationFileName("Network", "wap") + '/';
        QString newName = path +"wap.conf";
        int index = 0;
        while ( QFile::exists(newName) ) {
            index++;
            newName = path+"wap"+QString::number(index)+".conf";
        }
        wapcfg = newName;
    }
    //write new settings to WAP config
    QSettings cfg( wapcfg, QSettings::IniFormat );
    QMapIterator<QString,QVariant> iter(prop);
    while( iter.hasNext() ) {
        iter.next();
        QString key = iter.key();
        if (key.startsWith("Wap") || key.startsWith("MMS") ||
                key.startsWith("Info") || key.startsWith("Browser") ) {
            cfg.setValue( key, iter.value() );
        }
    }
    qLog(Network) << "Saving WAP details to" << wapcfg;


    if ( dialupcfg.isEmpty() ) {
        QString templateF;
        if ( bearer == "GPRS" || bearer == "GSM-GPRS" ) {
            //use GPRS dialup template as default
            templateF = Qtopia::qtopiaDir()+"etc/network/dialupGPRS.conf";
        } else if ( bearer == "GSM-CSD" || bearer == "GSM/CSD" ) {
            //use ordinary GSM dialup template
            templateF = Qtopia::qtopiaDir()+"etc/network/dialup.conf";
        }

        if (!QFile::exists(templateF))
            return;

        //copy template config to new config file
        QDir settingsDir(QtopiaNetwork::settingsDir());
        QString filename;
        int n=0;
        QString basename = QFileInfo( templateF ).baseName();
        do {
            filename = settingsDir.filePath( basename +
                    QString::number(n++) + ".conf" );
        } while ( QFile::exists( filename ) );

        //copy template config to new config file
        {
            QFile input( templateF );
            if ( !input.open(QIODevice::ReadOnly) )
                return;
            QFile output(filename);
            if ( !output.open(QIODevice::WriteOnly) )
                return;
            output.write(input.readAll());
        }
        //QFile::copy( templateF, filename);

        //add option to existing dialup account config
        qLog(Network) << "Saving dialup details to new dialup config " << filename;
        {
            QTranslatableSettings config( filename, QSettings::IniFormat );
            config.setValue("Info/Name", prop.value("Info/Name"));
            if ( prop.contains("Serial/APN") )
                config.setValue("Serial/APN", prop.value("Serial/APN"));
            else
                config.setValue("Serial/Phone", prop.value("Serial/Phone"));
            config.setValue("Properties/Password", prop.value("Properties/Password"));
            config.setValue("Properties/UserName", prop.value("Properties/UserName"));
            config.sync();
        }
        addService( filename );
        dialupcfg = filename;
    } else {
        //add option to existing dialup account config
        QSettings config( dialupcfg, QSettings::IniFormat );
        if ( prop.contains("Serial/APN") )
            config.setValue("Serial/APN", prop.value("Serial/APN"));
        else
            config.setValue("Serial/Phone", prop.value("Serial/Phone"));
        config.setValue("Properties/Password", prop.value("Properties/Password"));
        config.setValue("Properties/UserName", prop.value("Properties/UserName"));
        config.sync();
        qLog(Network) << "Saving dialup details to " << dialupcfg;
    }

    //link this wap account to existing dialup account
    cfg.setValue( "Info/DataAccount", dialupcfg );
}
#endif

void NetworkUI::updateIfaceStates()
{
    QtopiaIpcAdaptor o( "QPE/NetworkState" );
    o.send( MESSAGE(updateNetwork()) );
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
bool NetworkUI::eventFilter( QObject* watched, QEvent* event )
{
    int currentIndex = tabWidget->currentIndex();
    int count = tabWidget->count();
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft;
    if ( watched == table 
#ifdef QTOPIA_CELL
            || watched == wapPage->wapList
#endif
#ifdef QTOPIA_VPN
            || watched == vpnPage->vpnList
#endif
            ) {
        if ( event->type() == QEvent::KeyPress ) {
            QKeyEvent *ke = (QKeyEvent*) event;
            int key = ke->key();
            switch ( key ) {
                case Qt::Key_Back:
                    event->ignore();
                    break;
                case Qt::Key_Left:
                {
                    if ( rtl && currentIndex < count - 1 )
                        ++currentIndex;
                    else if ( !rtl && currentIndex > 0 )
                        --currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                }
                case Qt::Key_Right:
                {
                    if ( rtl && currentIndex > 0 )
                        --currentIndex;
                    else if ( !rtl && currentIndex < count - 1 )
                        ++currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                }
            }
        }
    }
    return false;
}
#endif

void NetworkUI::tabChanged( int index )
{
    switch( index ) {
        case 0:
            //table->setFocus();
            table->setEditFocus( true );
            break;
#ifdef QTOPIA_CELL
        case 1:
            //wapPage->wapList->setFocus();
            wapPage->wapList->setEditFocus( true );
            break;
#endif
#ifdef QTOPIA_VPN
# ifdef QTOPIA_CELL
        case 2:
# else
        case 1:
# endif
            vpnPage->vpnList->setEditFocus( true );
#endif
    }
}

/*!
    \service NetworkSetupService NetworkSetup
    \brief Provides network configuration service.

    The \i NetworkSetup service allows applications to configure the
    network.
*/

/*!
    \internal
*/
NetworkSetupService::~NetworkSetupService()
{
}

void NetworkSetupService::configureData()
{
    if ( !parent->isVisible() )
        parent->showMaximized();
    parent->setCurrentTab(0);
}

#ifdef QTOPIA_CELL
void NetworkSetupService::configureWap()
{
    if ( !parent->isVisible() )
        parent->showMaximized();
    parent->setCurrentTab(1);
}

// Extract SMS message sender from the auxillary data on a QDSActionRequest.
static QString extractSender( const QByteArray& auxData )
{
    if ( auxData.isEmpty() )
        return QString();
    QDataStream stream( auxData );
    QSMSMessage msg;
    stream >> msg;
    return msg.sender();
}

void NetworkSetupService::pushWapNetworkSettings
        ( const QDSActionRequest& request )
{
    parent->otaDatagram
        ( new QOtaReader( QOtaReader::Wap ), request.requestData().data(),
          extractSender( request.auxiliaryData() ) );
    QDSActionRequest( request ).respond();
}

void NetworkSetupService::pushNokiaNetworkSettings
        ( const QDSActionRequest& request )
{
    parent->otaDatagram
        ( new QOtaReader( QOtaReader::Nokia ), request.requestData().data(),
          extractSender( request.auxiliaryData() ) );
    QDSActionRequest( request ).respond();
}

#endif

#include "networkui.moc"
