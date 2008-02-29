/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#include "packageview.h"
#include "packagemodel.h"
#include "serveredit.h"

#include <QBoxLayout>
#include <QTextEdit>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>
#include <QUrl>
#include <QtopiaApplication>

#ifdef QTOPIA_PHONE
#include <QKeyEvent>

#include <qsoftmenubar.h>
#else
#include <QMenuBar>
#endif

#include <qtopialog.h>
#include <qdebug.h>

#ifndef QT_NO_SXE
#include "domaininfo.h"
#endif

#include "packagemanagerservice.h"

PackageView *PackageView::latestInstance;

PackageView::PackageView( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
    , listsPrepared( false )
{
    setWindowTitle( tr( "Package Manager" ));
    model = new PackageModel( this );

    connect(model, SIGNAL(targetsUpdated(const QStringList&)),
            this, SLOT(targetsChanged(const QStringList&)));
    connect(model, SIGNAL(serversUpdated(const QStringList&)),
            this, SLOT(serversChanged(const QStringList&)));
    connect( this, SIGNAL(targetChoiceChanged(const QString &)),
            model, SLOT(userTargetChoice(const QString &)) );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 2 );
    vb->setMargin( 2 );

    view = new QTreeView( this );
    view->setModel( model );
    connect( view, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(activateItem(const QModelIndex&)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            view, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    connect( model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            view, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    vb->addWidget( view );

    info = new QTextEdit( this );
    info->setReadOnly( true );
    info->setFocusPolicy( Qt::NoFocus );

    connect( model, SIGNAL(infoHtml(const QString&)),
            info, SLOT(setHtml(const QString&)) );
#ifndef QT_NO_SXE
    connect( model, SIGNAL(domainUpdate(const QString&)),
            this, SLOT(postDomainUpdate(const QString&)) );
#endif
    vb->addWidget( info );

    menuServers = new QMenu( tr( "Servers" ), this );
    QAction* actionEditServers = new QAction( QIcon( ":icon/globe" ),
            tr( "Edit servers..." ), this );
    menuServers->addAction( actionEditServers );
    connect( actionEditServers, SIGNAL(triggered()), this, SLOT(editServers()) );

    menuTarget = new QMenu( tr( "Install to" ), this );
#ifdef QTOPIA_PHONE
    QMenu* contextMenu = QSoftMenuBar::menuFor( this );
    contextMenu->addMenu( menuServers );
    //contextMenu->addMenu( menuTarget );

    // under phone, the select key ("OK") activates installSelection()
    QSoftMenuBar::setLabel( view, Qt::Key_Select, QSoftMenuBar::Ok, QSoftMenuBar::EditFocus );
#else
    QMenuBar *mb = new QMenuBar( this );
    vb->setMenuBar( mb );
    mb->addMenu( menuServers );
    //mb->addMenu( menuTarget );
    QPushButton* pbShowDetails = new QPushButton( QIcon( ":icon/details" ),
            tr( "Details" ), this );
    connect( pbShowDetails, SIGNAL(clicked()),
            this, SLOT(showDetails()) );
    QPushButton* pbInstall = new QPushButton( QIcon( ":icon/install" ),
            tr( "Install" ), this );
    connect( pbInstall, SIGNAL(clicked()),
            this, SLOT(installSelection()) );
    QHBoxLayout *hb = new QHBoxLayout();
    hb->addWidget( pbShowDetails );
    hb->addWidget( pbInstall );
    vb->addLayout( hb );
#endif

    new PackageManagerService( this );

    QTimer::singleShot( 0, this, SLOT(init()) );
}

PackageView::~PackageView()
{
}

/**
  Post construction initialization, done after event loop
  displays the gui
*/
void PackageView::init()
{
    latestInstance = this;
    QStringList servers = model->getServers();
    QAction *sa;
    serversActionGroup = new QActionGroup( this );
    serversActionGroup->setExclusive( true );
    connect( serversActionGroup, SIGNAL(triggered(QAction*)),
             this, SLOT(serverChoice(QAction*)) );
    for ( int i = 0; i < servers.count(); i++ )
    {
        sa = new QAction( servers[i], serversActionGroup );
        sa->setCheckable( true );
        if ( i == 0 ) sa->setChecked( true );
        serversActionGroup->addAction( sa );
    }
    menuServers->addActions( serversActionGroup->actions() );

    targetActionGroup = new QActionGroup( this );
    targetActionGroup->setExclusive( true );
    connect( targetActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(targetChoice(QAction*)) );
    view->resizeColumnToContents( 0 );
    view->resizeColumnToContents( 1 );
    view->header()->hide();
    QItemSelectionModel *sel = new QItemSelectionModel( model );
    sel->setCurrentIndex( model->index( 1, 0, QModelIndex()), QItemSelectionModel::Select ); 
    view->setSelectionModel( sel );
    connect( sel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(updateText(const QModelIndex&, const QModelIndex&)) );
}

void PackageView::activateItem( const QModelIndex &item )
{
    if ( model->rowCount( item ) > 0 ) // has children
    {
        if ( view->isExpanded( item ) )
            view->collapse( item );
        else
            view->expand( item );
    }

    QModelIndex ix = item.parent();
    if ( ix.isValid() ) // is a child node
        showDetails( item );
}

void PackageView::postDomainUpdate( const QString &dom )
{
#ifndef QT_NO_SXE
    QModelIndex curIndex = view->currentIndex();
    QVariant packageName = model->data( curIndex, Qt::DisplayRole );

    if( model->hasSensitiveDomains( dom ) )
    {
        info->setHtml( tr( "%1 utilizes protected resources" ).arg( packageName.toString() ) );
    }
    else
    {
        info->setHtml( DomainInfo::explain( dom, packageName.toString() ));
    }
#else
    Q_UNUSED( dom );
#endif
}

void PackageView::serverChoice( QAction *a )
{
    Q_UNUSED( a );

    QString server = serversActionGroup->checkedAction()->text();
    model->setServer( server );
}

void PackageView::targetChoice( QAction * )
{
    if ( targetActionGroup->checkedAction() == 0 )
        return;
    QString newChoice = targetActionGroup->checkedAction()->text();
    if ( !prevTarget.isEmpty() && prevTarget == newChoice )
        return;
    prevTarget = newChoice;
    emit targetChoiceChanged( newChoice );
}

void PackageView::serversChanged( const QStringList &servers )
{
    QList<QAction*> actionList = serversActionGroup->actions();
    QStringList serverList = servers;
    QAction *sa;

    // remove from the list everything thats in the menu, and
    // if something is not in the list, remove it from the menu
    for ( int i = 0; i < actionList.count(); i++ )
    {
        sa = actionList[i];
        if ( serverList.contains( sa->text() ) )
        {
            serverList.removeAll( sa->text() );
        }
        else
        {
            serversActionGroup->removeAction( sa );
            menuServers->removeAction( sa );
            delete sa;
        }
    }

    // now the list contains just what wasnt in the menu so add
    // them if there are any
    for ( int i = 0; i < serverList.count(); i++ )
    {
        sa = new QAction( serverList[i], serversActionGroup );
        sa->setCheckable( true );
    }

    if ( serversActionGroup->checkedAction() == 0 )
    {
        actionList = serversActionGroup->actions();
        if ( actionList.count() > 0 )
            actionList[0]->setChecked( true );
    }
    menuServers->addActions( serversActionGroup->actions() );
}

/**
  Slot to receive disk/storage add or removal messages from the model
  and alter the menus accordingly
*/
void PackageView::targetsChanged( const QStringList &targets )
{
    QList<QAction*> actionList = targetActionGroup->actions();
    QStringList targetList = targets;
    QAction *sa;
    // remove from the list everything thats in the menu, and
    // if something is not in the list, remove it from the menu
    for ( int i = 0; i < actionList.count(); i++ )
    {
        sa = actionList[i];
        if ( targetList.contains( sa->text() ))
        {
            targetList.removeAll( sa->text() );
        }
        else
        {
            targetActionGroup->removeAction( sa );
            menuTarget->removeAction( sa );
            delete sa;
        }
    }
    // now the list contains just what wasnt in the menu so add
    // them if there are any
    for ( int i = 0; i < targetList.count(); i++ )
    {
        sa = new QAction( targetList[i], targetActionGroup );
        sa->setCheckable( true );
    }
    if ( targetActionGroup->checkedAction() == 0 )
    {
        actionList = targetActionGroup->actions();
        actionList[0]->setChecked( true );
    }
    menuTarget->addActions( targetActionGroup->actions() );
}

void PackageView::editServers()
{
    ServerEdit *edit = new ServerEdit( this );

    int result = edit->exec();

    if ( result == QDialog::Accepted && edit->wasModified() )
        model->setServers( edit->serverList() );

    delete edit;
}

/**
    \internal
    \function targetChoiceChanged( const QString & )
    Emitted when the user choice of installation target changes, eg
    from "Internal Storage" to "CF Card"
*/

void PackageView::showDetails( const QModelIndex &item )
{
    if( !item.isValid() || !item.parent().isValid() )
        return;

    QString name = model->data( item, Qt::DisplayRole ).toString(); //package name
    bool enabled = model->data( item, Qt::StatusTipRole ).toBool(); //whether package enabled

    PackageDetails *pd = new PackageDetails( this, true );
    QString op = model->getOperation( item );
    pd->installButton->setText( op );

    QtopiaApplication::setMenuLike( pd, true );

    if ( enabled )
        pd->reenableButton->setVisible( false );
    else
        pd->reenableButton->setVisible( true );

    pd->setWindowTitle( name );
    QString details;
    if ( op == "Install" )
    {
#ifndef QT_NO_SXE
        if( model->hasSensitiveDomains( model->data( item, Qt::WhatsThisRole ).toString() ) )
        {
            details = tr( "The package <font color=\"#0000FF\">%1</font> <b>cannot be installed</b> as it utilizes protected resources" ).arg( name );

            pd->installButton->setVisible( false );
            pd->cancelButton->setText( tr( "OK" ) );
        }
        else
        {
#endif
            details = tr( "<font color=\"#66CC00\"><b>Installing package</b></font> %1 <b>Go ahead?</b>" )
#ifndef QT_NO_SXE
                .arg( DomainInfo::explain( model->data( item, Qt::WhatsThisRole ).toString(), name ));

            pd->installButton->setVisible( true );
            pd->cancelButton->setText( tr( "Cancel" ) );
        }
#else
                .arg( model->data( item, Qt::DisplayRole ).toString() );
#endif
    }
    else if ( op == "Uninstall" )
    {
#ifndef QT_NO_SXE
        pd->installButton->setVisible( true );
        pd->cancelButton->setText( tr( "Cancel" ) );
#endif
        if ( enabled )
        {
            details = tr( "<font color=\"#FF9900\"><b>Uninstalling package</b></font> %1 <b>Go ahead?</b>" )
                      .arg( model->data( item, Qt::WhatsThisRole ).toString() );
        } else
        {
            details = tr( "<font color=\"#FF9900\"><b>Uninstalling/Re-enabling package</b></font> %1" )
                      .arg( model->data( item, Qt::WhatsThisRole ).toString() );

        }
    }

    pd->description->setHtml( details );
    qLog(Package) << "show details" << ( name.isNull() ? "no valid name" : name );
    pd->showMaximized();
    switch ( pd->exec() )
    {
        case ( QDialog::Accepted ):
            model->activateItem( item );
            break;
        case ( PackageDetails::Reenable ):
            model->reenableItem( item );
            break;
    }
}

void PackageView::installSelection()
{
    showDetails( view->currentIndex() );
}

void PackageView::updateText( const QModelIndex& newCurrent, const QModelIndex& )
{
    model->sendUpdatedText( newCurrent );
}

void PackageView::displayMessage( const QString &s )
{
    latestInstance->info->setHtml( s );
}

#ifdef QTOPIA_PHONE
void PackageView::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() )
    {
        case Qt::Key_Right:
            showDetails( view->currentIndex() );
            e->accept();
            return;

        default:
            // fall thru to ignore
            ;
    }

    e->ignore();
}
#endif

void PackageView::showEvent( QShowEvent * )
{
    if( !listsPrepared )
    {
        QTimer::singleShot( 0, model, SLOT(populateLists()) );

        listsPrepared = true;
    }
}
