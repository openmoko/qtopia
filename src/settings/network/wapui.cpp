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

#include "wapui.h"
#include "addwapui.h"

#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QWidget>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>

#ifdef QTOPIA_PHONE
#include <QAction>
#include <QMenu>
#else
#include <QPushButton>
#endif


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

#ifdef QTOPIA_PHONE
    connect( wapList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(selectDefaultWAP()));

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );

    wap_add = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect( wap_add, SIGNAL( triggered(bool) ), this, SLOT( addWap() ) );
    contextMenu->addAction(wap_add);

    wap_remove = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    connect( wap_remove, SIGNAL(triggered(bool)), this, SLOT(removeWap()) );
    contextMenu->addAction(wap_remove);

    wap_props = new QAction( QIcon(":icon/settings"), tr("Properties..."), this );
    connect( wap_props, SIGNAL(triggered(bool)), this, SLOT(doWap()) );
    contextMenu->addAction(wap_props);

#else
    QGridLayout* grid = new QGridLayout();
    vb->addItem( grid );
    grid->setSpacing( 4 );
    grid->setMargin( 4 );

    defaultPB = new QPushButton( tr("Default"), this );
    connect( defaultPB, SIGNAL(clicked()), this, SLOT(selectDefaultWAP()) );
    grid->addWidget( defaultPB, 0, 0 );

    propPB = new QPushButton( tr("Properties..."), this );
    propPB->setIcon( QIcon(":icon/settings") );
    connect( propPB, SIGNAL(clicked()), this, SLOT(doWap()));
    grid->addWidget( propPB, 0, 1 );

    addPB = new QPushButton( tr("New..."), this );
    addPB->setIcon( QIcon(":icon/new") );
    connect( addPB, SIGNAL(clicked()), this, SLOT(addWap()));
    grid->addWidget( addPB, 1, 0 );

    removePB = new QPushButton( tr("Delete"), this );
    removePB->setIcon( QIcon(":icon/trash") );
    connect( removePB, SIGNAL(clicked()), this, SLOT(removeWap()));
    grid->addWidget( removePB, 1, 1 );
#endif

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
        if ( config == defaultWap ) {
            hasDefault = true;
            QFont f = item->font();
            f.setBold( config == defaultWap );
            item->setFont( f );
            dfltAccount->setText( name );
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
#ifdef QTOPIA_PHONE
        wap_remove->setEnabled( false );
        wap_remove->setVisible( false );
        wap_props->setEnabled( false );
        wap_props->setVisible( false );
#else
        propPB->setEnabled( false );
        removePB->setEnabled( false );
        defaultPB->setEnabled( false );
#endif
    } else {
        QListWidgetItem* item = wapList->currentItem();
        if ( !item ) {
#ifdef QTOPIA_PHONE
            wap_remove->setEnabled( false );
            wap_remove->setVisible( false );
            wap_props->setEnabled( false );
            wap_props->setVisible( false );
#else
            propPB->setEnabled( false );
            defaultPB->setEnabled( false );
            removePB->setEnabled( false );
#endif
        } else {
#ifdef QTOPIA_PHONE
            wap_remove->setEnabled( true );
            wap_remove->setVisible( true );
            wap_props->setEnabled( true );
            wap_props->setVisible( true );
#else
            removePB->setEnabled( true );
            defaultPB->setEnabled( true );
            propPB->setEnabled( true );
#endif
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
