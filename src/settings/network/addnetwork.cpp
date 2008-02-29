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

#include "addnetwork.h"

#include <QLabel>
#include <QLayout>
#include <QListWidget>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopianetwork.h>
#include <qtranslatablesettings.h>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

AddNetworkUI::AddNetworkUI(QWidget* parent, Qt::WFlags fl)
    : QDialog( parent, fl )
{
    setModal( true );
    init();

#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
#endif
    setObjectName("add-service");
}

AddNetworkUI::~AddNetworkUI()
{
}

void AddNetworkUI::init()
{
    setWindowTitle( tr("Add Service") );

    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setMargin( 4 );
    vb->setSpacing( 2 );

    list = new QListWidget(this);
    list->setAlternatingRowColors( true );
    vb->addWidget(list);

    QStringList configList = QtopiaNetwork::availableNetworkConfigs(
        QtopiaNetwork::Any , Qtopia::qtopiaDir() + "etc/network");
    foreach( QString config, configList )
    {
        QtopiaNetwork::Type type = QtopiaNetwork::toType( config );
        if ( type & QtopiaNetwork::Hidden )
            continue;
        QTranslatableSettings cfg(config, QSettings::IniFormat);
        QListWidgetItem* item = new QListWidgetItem(
                cfg.value("Info/Name").toString(), list);

        QString image;
        if ( type & QtopiaNetwork::Dialup )
            image = ":icon/Network/dialup/dialup-online"; //analog dialup
        else if ( type & QtopiaNetwork::GPRS )
            image = ":icon/Network/dialup/GPRS-online"; //GPRS
        else if ( type & QtopiaNetwork::WirelessLAN )
            image = ":icon/Network/lan/WLAN-online";
        else if ( type & QtopiaNetwork::LAN )
            image = ":icon/Network/lan/LAN-online";
        else if ( type & QtopiaNetwork::Bluetooth )
            image = ":icon/bluetooth/bluetooth-online";

        if ( !image.isEmpty() )
            item->setIcon( QIcon( image ) );
        item->setData( Qt::UserRole+1, config );
    }
    list->sortItems();
    list->scrollToTop();

    hint = new QLabel( this );
    hint->setText("");
    hint->setWordWrap( true );
    hint->hide();
    vb->addWidget(hint);

    connect( list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateHint()));
    connect( list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemSelected()) );
#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( this, true );
#else

#endif
    if (list->count())
        list->setCurrentItem( list->item(0) );

    updateHint();
}

void AddNetworkUI::updateHint()
{
    QListWidgetItem* item = list->currentItem();
    if (!item)
        return;
    QString cfgFile = item->data(Qt::UserRole+1).toString();
    QTranslatableSettings cfg(cfgFile, QSettings::IniFormat);
    cfg.beginGroup("Info");
    QVariant v = cfg.value("Help");
    cfg.endGroup();

    QString desc = v.toString();
    if ( !desc.isEmpty() ) {
        hint->setText( desc );
        hint->show();
    } else {
        hint->hide();
    }
}

QString AddNetworkUI::selectedConfig() const
{
    if (!list->count())
        return QString();
    return list->currentItem()->data(Qt::UserRole+1).toString();
}

void AddNetworkUI::itemSelected()
{
    QDialog::done(1);
}
