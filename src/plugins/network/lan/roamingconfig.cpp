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

#include "roamingconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QMultiHash>

RoamingPage::RoamingPage( const QtopiaNetworkProperties& cfg, QWidget* parent, Qt::WFlags fl )
    : QWidget( parent, fl )
{
#if WIRELESS_EXT > 13
    ui.setupUi( this );
    ui.up->setIcon( QIcon(":icon/up") );
    ui.down->setIcon( QIcon(":icon/down") );
    init ( cfg );

    connect( ui.up, SIGNAL(clicked()), this, SLOT(up()) );
    connect( ui.down, SIGNAL(clicked()), this, SLOT(down()) );
    connect( ui.autoConnect, SIGNAL(stateChanged(int)), this, SLOT(reconnectToggled(int)) );
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

void RoamingPage::up()
{
#if WIRELESS_EXT > 13
    QListWidgetItem* item = ui.knownNetworks->currentItem();
    if ( !item )
        return;

    const int oldRow = ui.knownNetworks->row( item );
    if ( oldRow > 0 ) {
        ui.knownNetworks->takeItem( oldRow );
        ui.knownNetworks->insertItem( oldRow-1, item );
        ui.knownNetworks->setCurrentRow( oldRow-1 );
    }
#endif // WIRELESS_EXT
}

void RoamingPage::down()
{
#if WIRELESS_EXT > 13
    QListWidgetItem* item = ui.knownNetworks->currentItem();
    if ( !item )
        return;

    const int oldRow = ui.knownNetworks->row( item );
    if ( oldRow < ui.knownNetworks->count()-1 ) {
        ui.knownNetworks->takeItem( oldRow );
        ui.knownNetworks->insertItem( oldRow+1, item );
        ui.knownNetworks->setCurrentRow( oldRow+1 );
    }
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
#endif // NO_WIRELESS_LAN
