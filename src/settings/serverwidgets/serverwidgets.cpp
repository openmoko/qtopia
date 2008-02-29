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

#include "serverwidgets.h"

#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>

#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>

ServerWidgetSettings::ServerWidgetSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    ui.setupUi( this );
    init();
    connect( ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this, SLOT(newWidgetSelected(QTreeWidgetItem*,int)) );
    connect( ui.shutdown, SIGNAL(clicked()), this, SLOT(shutdownClicked()) );
    //ui.treeWidget->installEventFilter( this );
}

ServerWidgetSettings::~ServerWidgetSettings()
{
}

void ServerWidgetSettings::init()
{
    setWindowTitle( tr("Server Widgets") );

    ui.treeWidget->header()->hide();
    ui.treeWidget->setColumnCount( 1 );

    QTreeWidgetItem* server =
            new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(tr("ServerInterface")), (int)ServerWidgetSettings::ServerInterface);
    QTreeWidgetItem* browser =
            new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(tr("BrowserScreen")), (int)ServerWidgetSettings::BrowserScreen);
    QTreeWidgetItem* messageBox =
            new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(tr("MessageBox")), (int)ServerWidgetSettings::MessageBox);
    QTreeWidgetItem* dialerScreen =
            new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(tr("DialerScreen")), (int)ServerWidgetSettings::DialerScreen);
    QTreeWidgetItem* secondaryDisplay =
            new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(tr("SecondaryDisplay")), (int)ServerWidgetSettings::SecondaryDisplay);

    ui.treeWidget->addTopLevelItem( server );
    ui.treeWidget->addTopLevelItem( browser );
    ui.treeWidget->addTopLevelItem( messageBox );
    ui.treeWidget->addTopLevelItem( dialerScreen );
    ui.treeWidget->addTopLevelItem( secondaryDisplay );

    //TODO: the interface options are hardcoded
    QTreeWidgetItem* slideIn = new QTreeWidgetItem( messageBox, QStringList() << tr("SlideIn"), -1 );
    slideIn->setCheckState( 0, Qt::Unchecked );
    slideIn->setData( 0, Qt::UserRole, "SlideIn" );
    QTreeWidgetItem* wheel = new QTreeWidgetItem( browser, QStringList() << tr("Wheel"), -1 );
    wheel->setCheckState( 0, Qt::Unchecked );
    wheel->setData( 0, Qt::UserRole, "Wheel" );
    QTreeWidgetItem* e2si = new QTreeWidgetItem( server, QStringList() << tr("Example"), -1 );
    e2si->setCheckState( 0, Qt::Unchecked );
    e2si->setData( 0, Qt::UserRole, "E2" );
    QTreeWidgetItem* e2browser = new QTreeWidgetItem( browser, QStringList() << tr("Example"), -1 );
    e2browser->setCheckState( 0, Qt::Unchecked );
    e2browser->setData( 0, Qt::UserRole, "E2" );

    QSettings cfg( "Trolltech", "ServerWidgets" );
    cfg.beginGroup( "Mapping" );

    ui.uiSelector->setCurrentIndex( 0 ); //default Qtopia
    if ( cfg.childKeys().isEmpty() )
        return;

    if ( cfg.contains( "Default" ) )
        if ( cfg.value("Default").toString() == "E2" )
            ui.uiSelector->setCurrentIndex( 1 );

    QString tmp = cfg.value("BrowserScreen").toString();
    if ( !tmp.isEmpty() ) {
        if ( tmp == "Wheel" )
            wheel->setCheckState( 0, Qt::Checked );
        else if ( tmp == "E2" )
            e2browser->setCheckState( 0 , Qt::Checked );
    }

    tmp = cfg.value("ServerInterface").toString();
    if ( !tmp.isEmpty() ) {
        if ( tmp == "E2" )
            e2si->setCheckState( 0, Qt::Checked );
    }

    tmp = cfg.value("MessageBox").toString();
    if ( !tmp.isEmpty() ) {
        if ( tmp == "SlideIn" )
            slideIn->setCheckState( 0, Qt::Checked );
    }
}

bool ServerWidgetSettings::eventFilter( QObject* o, QEvent* e )
{
    if ( o == ui.treeWidget ) {
        if ( e->type() == QEvent::FocusIn ) {
            ui.treeWidget->setEditFocus( true );
            if ( !ui.treeWidget->currentItem() ) {
                QTreeWidgetItem* item = ui.treeWidget->topLevelItem( 0 );
                if ( item )
                    ui.treeWidget->setCurrentItem( item );
            }
        } else if ( e->type() == QEvent::KeyPress ) {
            /*QKeyEvent* ke = (QKeyEvent*) e;

            QTreeWidgetItem* current = ui.treeWidget->currentItem();
            if ( ke->key() == Qt::Key_Up && ui.treeWidget->indexOfTopLevelItem( current ) == 0 ) {
                ui.uiSelector->setFocus();
                return true;
            } else if ( ke->key() == Qt::Key_Down &&
                    ui.treeWidget->indexOfTopLevelItem( current ) == ui.treeWidget->topLevelItemCount()-1 ) {
                ui.uiSelector->setFocus();
                return true;
            }*/
        }
    }

    return false;
}


void ServerWidgetSettings::shutdownClicked()
{
    saveSettings();
    {
        QtopiaIpcEnvelope env( "QPE/System", "restart()" );
    }
    QtopiaApplication::quit();
}

void ServerWidgetSettings::saveSettings()
{
    QSettings cfg( "Trolltech", "ServerWidgets" );
    cfg.beginGroup( "Mapping" );
    cfg.remove(""); //delete all entries in current grp

    switch( ui.uiSelector->currentIndex() ) {
        default:
        case 0:
            //standard Qtopia -> nothing to do
            break;
        case 1:
            cfg.setValue("Default", "E2");
            break;
    }

    //custom
    for( int i = 0; i<ui.treeWidget->topLevelItemCount(); i++ ) {
        QTreeWidgetItem* top = ui.treeWidget->topLevelItem( i );
        for ( int j = 0; j < top->childCount(); j++ ) {
            QTreeWidgetItem* child = top->child( j );
            if ( !child || child->checkState(0) != Qt::Checked )
                continue;
            switch ( top->type() ) {
                case ServerInterface:
                    cfg.setValue( "ServerInterface", child->data( 0, Qt::UserRole ).toString() );
                    break;
                case SecondaryDisplay:
                    cfg.setValue( "SecondaryDisplay", child->data( 0, Qt::UserRole ).toString() );
                    break;
                case DialerScreen:
                    cfg.setValue( "DialerScreen", child->data( 0, Qt::UserRole ).toString() );
                    break;
                case MessageBox:
                    cfg.setValue( "MessageBox", child->data(0, Qt::UserRole ).toString() );
                    break;
                case BrowserScreen:
                    cfg.setValue( "BrowserScreen", child->data( 0, Qt::UserRole ).toString() );
                    break;
                case CallScreen:
                    cfg.setValue( "CallScreen", child->data( 0, Qt::UserRole ).toString() );
                    break;
                default:
                    qWarning() << "unknown type of server widget" << top->type();
            }
        }
    }
    cfg.sync();
}

void ServerWidgetSettings::accept()
{
    saveSettings();
    QDialog::accept();
}

void ServerWidgetSettings::newWidgetSelected( QTreeWidgetItem* item, int /*column*/ )
{
    if ( !item || item->type() != -1 )
        return;

    static bool inChange = false;
    if (inChange)
        return;
    inChange = true;

    //prevent double selection
    QTreeWidgetItem* parent = item->parent();
    if ( !parent )
        return;
    int activatedItem = parent->indexOfChild( item );
    for( int i=0; i< parent->childCount(); i++ ) {
        if ( i != activatedItem ) {
            parent->child( i )->setCheckState( 0, Qt::Unchecked );
        }
    }
    inChange = false;
}
