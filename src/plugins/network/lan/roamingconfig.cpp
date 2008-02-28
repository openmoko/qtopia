/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "roamingconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QKeyEvent>
#include <QMultiHash>
#include "roamingmonitor.h" //includes wireless.h

RoamingPage::RoamingPage( const QtopiaNetworkProperties& cfg, QWidget* parent, Qt::WFlags fl )
    : QWidget( parent, fl ), currentSelection( 0 )
{
#if WIRELESS_EXT > 13
    ui.setupUi( this );
    init ( cfg );

    connect( ui.autoConnect, SIGNAL(stateChanged(int)), this, SLOT(reconnectToggled(int)) );
    connect( ui.knownNetworks, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(listActivated(QListWidgetItem*)) );
    ui.knownNetworks->installEventFilter(this);
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT

}

RoamingPage::~RoamingPage()
{
}

QtopiaNetworkProperties RoamingPage::properties()
{
    QtopiaNetworkProperties results;
#if WIRELESS_EXT > 13
    saveConfig();

    results.insert( "WirelessNetworks/size", props.take( QString("size") ) );
    results.insert( "WirelessNetworks/Timeout", props.take( QString("Timeout") ) );
    results.insert( "WirelessNetworks/AutoConnect", props.take( QString("AutoConnect") ) );

    QHash<int,int> oldToNew;
    int old;
    QList<QVariant> essidList = props.values("ESSID");
    for(int i = 0; i< ui.knownNetworks->count(); ++i) {
        old = essidList.indexOf( ui.knownNetworks->item( i )->text() );
        oldToNew.insert( old, i+1 );
    }

    const QList<QString> keys = props.keys();
    const QString prefix( "WirelessNetworks/%1/" );
    foreach( QString k , keys ) {
        QList<QVariant> entries = props.values( k );
        for( int i=0; i<entries.count(); ++i)
            results.insert( prefix.arg(oldToNew[i])+k, entries[i]);
    }
#endif // WIRELESS_EXT
    return results;
}

void RoamingPage::setProperties( const QtopiaNetworkProperties& cfg )
{
#if WIRELESS_EXT > 13
    init( cfg );
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT
}

void RoamingPage::init( const QtopiaNetworkProperties& cfg )
{
#if WIRELESS_EXT > 13
    props.clear();
    ui.knownNetworks->clear();

    const QList<QString> keys = cfg.keys();
    QString normalizedKey;
    foreach( QString k, keys ) {
        normalizedKey = k.mid(k.lastIndexOf(QChar('/'))+1);
        props.insert( normalizedKey, cfg.value( k ) );
        if ( normalizedKey == QLatin1String("ESSID") ) {
            QListWidgetItem* item = new QListWidgetItem( cfg.value( k ).toString(), ui.knownNetworks );
            Q_UNUSED(item)
        }
    }
    readConfig();
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT
}

void RoamingPage::readConfig()
{
#if WIRELESS_EXT > 13
    const int tout = props.value("Timeout", 10).toInt();
    ui.timeout->setValue( tout );
    ui.autoConnect->setChecked( props.value("AutoConnect", false).toBool() );
    reconnectToggled( ui.autoConnect->checkState() );
#endif // WIRELESS_EXT
}

void RoamingPage::saveConfig()
{
#if WIRELESS_EXT > 13
    props.replace( "Timeout", ui.timeout->value() );
    props.replace( "AutoConnect", ui.autoConnect->isChecked() );
#endif // WIRELESS_EXT
}

void RoamingPage::reconnectToggled( int newState )
{
#if WIRELESS_EXT > 13
    ui.timeout->setEnabled( newState == Qt::Checked );
    ui.timeoutLabel->setEnabled( newState == Qt::Checked );
#else
    Q_UNUSED( newState )
#endif // WIRELESS_EXT
}

void RoamingPage::listActivated(QListWidgetItem* item)
{
#if WIRELESS_EXT > 13
    if ( !item )
        return;
    if ( !currentSelection ) {
        ui.header->setText( tr("Moving %1", "%1=essid").arg(item->text()) );
        QFont f = item->font();
        f.setBold( true );
        item->setFont( f );
        item->setText( item->text() );
        currentSelection = item;
    }else if ( item == currentSelection ) {
        ui.header->setText( tr("Order of selection") );
        QFont f = item->font();
        f.setBold( false );
        currentSelection->setFont( f );
        currentSelection = 0;
    }
#else
    Q_UNUSED( item );
#endif //WIRELESS_EXT
}

bool RoamingPage::eventFilter( QObject* watched, QEvent* event )
{
#if WIRELESS_EXT > 13
    if ( watched == ui.knownNetworks &&
            0 != currentSelection )
    {
        if ( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease ) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);

            if ( event->type() == QEvent::KeyRelease &&  //ignore releases if key is one we watch out for
                    (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key()==Qt::Key_Back) )
                return true;

            int row = ui.knownNetworks->currentRow();
            if ( ke->key() == Qt::Key_Up ) {
                if ( row > 0 ) //top row cannot move further up
                {
                    ui.knownNetworks->takeItem( row );
                    ui.knownNetworks->insertItem( row-1, currentSelection );
                    ui.knownNetworks->setCurrentRow( row-1 );
                }
                return true;
            } else if ( ke->key() == Qt::Key_Down ) {
                if ( row < ui.knownNetworks->count()-1 ) { //bottom row cannot move further down
                    ui.knownNetworks->takeItem( row );
                    ui.knownNetworks->insertItem( row+1, currentSelection );
                    ui.knownNetworks->setCurrentRow( row+1 );
                }
                return true;
            } else if ( ke->key() == Qt::Key_Back ) {
                return true; //ignore back for as long as we have a selection
            }
        }
    }
#else
    Q_UNUSED(watched);
    Q_UNUSED(event);
#endif
    return false;
}
#endif // NO_WIRELESS_LAN
