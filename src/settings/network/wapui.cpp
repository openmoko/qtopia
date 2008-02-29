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

#include "wapui.h"
#include "addwapui.h"

#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QWidget>

#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>
#include <QAction>
#include <QMenu>

WapUI::WapUI( QWidget* parent, Qt::WFlags fl)
    : QWidget( parent, fl )
{
    init();
}

WapUI::~WapUI()
{
}

void WapUI::init()
{
    setWindowTitle( tr("WAP") );
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 2 );
    vb->setSpacing( 4 );
    QLabel* defaultLabel = new QLabel( tr("Standard account:"), this );
    vb->addWidget( defaultLabel );
    dfltAccount = new QLabel(tr("<none selected>"), this );
    dfltAccount->setAlignment( Qt::AlignCenter );
    QFont f = dfltAccount->font();
    f.setBold( true );
    dfltAccount->setFont( f );
    vb->addWidget( dfltAccount );

    QLabel* label = new QLabel(
            tr("Available WAP accounts:"), this);
    label->setWordWrap( true );
    vb->addWidget( label );

    wapList = new QListWidget( this );
    wapList->setWhatsThis( tr("the selected account is used as default account") );
    wapList->setAlternatingRowColors( true );
    wapList->setSelectionBehavior( QAbstractItemView::SelectRows );
    wapList->setSelectionMode( QAbstractItemView::SingleSelection );
    vb->addWidget( wapList );
    connect( wapList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateActions()));

    connect( wapList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(selectDefaultWAP()));

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );

    wap_add = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect( wap_add, SIGNAL(triggered(bool)), this, SLOT(addWap()) );
    contextMenu->addAction(wap_add);

    wap_remove = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    connect( wap_remove, SIGNAL(triggered(bool)), this, SLOT(removeWap()) );
    contextMenu->addAction(wap_remove);

    wap_props = new QAction( QIcon(":icon/settings"), tr("Properties..."), this );
    connect( wap_props, SIGNAL(triggered(bool)), this, SLOT(doWap()) );
    contextMenu->addAction(wap_props);

    loadConfigs();
    updateActions();
}

QStringList WapUI::availableWapConfigs()
{
    QString path = Qtopia::applicationFileName("Network", "wap");
    QDir configDir(path);
    configDir.mkdir(path);

    QStringList files = configDir.entryList( QStringList("*.conf") );
    QStringList results;
    foreach( QString item, files ) {
        results.append( configDir.filePath( item ) );
    }
    return results;
}

void WapUI::loadConfigs()
{
    QSettings cfg("Trolltech", "Network");
    cfg.beginGroup("WAP");
    QString defaultWap = cfg.value("DefaultAccount").toString();
    cfg.endGroup();
    QStringList configList = availableWapConfigs();
    wapList->clear();
    bool hasDefault = false;
    foreach( QString config, configList ) {
        QSettings cfg( config, QSettings::IniFormat );
        QString name = cfg.value("Info/Name").toString();
        if (name.isEmpty()) {
            QFile::remove( config );
            continue;
        }
        QListWidgetItem* item = new QListWidgetItem( wapList );
        item->setData( ConfigRole, config );
        item->setText(name);
        item->setIcon( QIcon(":icon/wap") );

        const bool defaultConfig( config == defaultWap );
        item->setData( DefaultRole, defaultConfig );

        if ( defaultConfig ) {
            hasDefault = true;
            dfltAccount->setText( name );

            QFont f = item->font();
            f.setBold( true );
            item->setFont( f );
        }
    }

    if ( !hasDefault )
        dfltAccount->setText( tr("<none selected>") );
    if ( wapList->count() )
        wapList->setCurrentRow( 0 );

    wapList->setEditFocus( true );
}

void WapUI::updateActions()
{
    if ( !wapList->count() || wapList->currentRow() < 0 ) {
        wap_remove->setEnabled( false );
        wap_remove->setVisible( false );
        wap_props->setEnabled( false );
        wap_props->setVisible( false );
    } else {
        QListWidgetItem* item = wapList->currentItem();
        if ( !item ) {
            wap_remove->setEnabled( false );
            wap_remove->setVisible( false );
            wap_props->setEnabled( false );
            wap_props->setVisible( false );
        } else {
            wap_remove->setEnabled( true );
            wap_remove->setVisible( true );
            wap_props->setEnabled( true );
            wap_props->setVisible( true );
        }
    }
}

void WapUI::addWap()
{
    AddWapUI dlg( QString(""), this, 0 );
    dlg.showMaximized();
    dlg.exec();


    loadConfigs();
    updateActions();
    updateNetStates();
}

void WapUI::removeWap()
{
    if (wapList->currentRow() >= 0) {
        QListWidgetItem* item = wapList->currentItem();
        if ( !item )
            return;

        QString file = item->data( WapUI::ConfigRole ).toString();
        if ( Qtopia::confirmDelete( this, tr("WAP"), item->text() ) )
            QFile::remove( file );

        if ( item->data(DefaultRole).toBool()) {
            QSettings cfg("Trolltech", "Network");
            cfg.beginGroup("WAP");
            cfg.setValue("DefaultAccount", QString());
            cfg.endGroup();
        }
    }

    loadConfigs();
    updateActions();
    updateNetStates();
}

void WapUI::doWap()
{
    QListWidgetItem* item = wapList->currentItem();
    if ( !item )
        return;

    AddWapUI dlg( item->data( WapUI::ConfigRole ).toString(), this );
    dlg.showMaximized();
    if (dlg.exec() == QDialog::Accepted)
        updateNetStates();

    loadConfigs();
    updateActions();

}

void WapUI::selectDefaultWAP()
{
    if ( !wapList->count() || wapList->currentRow() < 0 )
        return;

    QListWidgetItem* item = wapList->currentItem();
    if ( !item )
        return;

    QFont f = item->font();
    f.setBold( false );
    for ( int i = wapList->count()-1; i >= 0; i-- ) {
        wapList->item( i )->setData( DefaultRole, false );
        wapList->item( i )->setFont( f );
    }
    f.setBold( true );
    item->setFont( f );
    item->setData( DefaultRole, true );
    dfltAccount->setText( item->text() );
    QSettings cfg("Trolltech", "Network");
    cfg.beginGroup("WAP");
    cfg.setValue("DefaultAccount", item->data(ConfigRole));
    cfg.endGroup();
    updateNetStates();
}

void WapUI::updateNetStates()
{
    QtopiaIpcAdaptor o( "QPE/NetworkState" );
    o.send( MESSAGE(wapChanged()) );
}
