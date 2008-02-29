/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#include "packageview.h"
#include "packagemodel.h"
#include "serveredit.h"

#include <QBoxLayout>
#include <QTextEdit>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>
#include <QtopiaApplication>
#include <QKeyEvent>
#include <QWaitWidget>
#include <qsoftmenubar.h>
#include <qtopialog.h>

#ifndef QT_NO_SXE
#include "domaininfo.h"
#endif

#include "packagemanagerservice.h"
/*! \internal
    Case sensitive less than operation for two QActions.
    Intended to be used with qSort on a list of QActions. 
*/
bool actionLessThan(QAction *a, QAction *b)
{
    return a->text() < b->text();
}

PackageDetails::PackageDetails(QWidget *parent, Type type, bool modal) : QDialog(parent)
{
    setupUi(this);
    setModal(modal);

    QMenu *contextMenu = new QMenu( this );
    if ( type == PackageDetails::Install || type == PackageDetails::Uninstall ) 
    {
        QAction *confirmAction = new QAction( tr("Confirm"), this );
        connect( confirmAction, SIGNAL(triggered()), this, SLOT(accept()) );
        contextMenu->addAction( confirmAction );
    }
    else { //otherwise assume package info
        description->setText("<font color=\"#000000\"> INFORMATION</font>");
    }

    QAction *cancelInstallAction = new QAction( tr("Cancel"), this );
    connect( cancelInstallAction, SIGNAL(triggered()), this, SLOT(reject()) );

    contextMenu->addAction( cancelInstallAction );
    QSoftMenuBar::addMenuTo( this->description, contextMenu );
}


const int PackageView::InstalledIndex = 0;
const int PackageView::DownloadIndex = 1;

PackageView::PackageView( QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    setWindowTitle( tr( "Package Manager" ));
    model = new PackageModel( this );

    connect(model, SIGNAL(targetsUpdated(QStringList)),
            this, SLOT(targetsChanged(QStringList)));
    connect(model, SIGNAL(serversUpdated(QStringList)),
            this, SLOT(serversChanged(QStringList)));
    connect(this, SIGNAL(targetChoiceChanged(QString)),
            model, SLOT(userTargetChoice(QString)));
    connect(model, SIGNAL(serverStatus(QString)),
            this, SLOT(postServerStatus(QString)));
    connect(model,  SIGNAL(newlyInstalled(QModelIndex)),
            this, SLOT(selectNewlyInstalled(QModelIndex)));

    //setup view for installed packages
    installedView = new QTreeView( this );
    installedView->setModel( model  );
    installedView->setRootIndex( model->index(0,0,QModelIndex()) );
    installedView->setRootIsDecorated( false );
    connect( installedView, SIGNAL(activated(QModelIndex)),
            this, SLOT(activateItem(QModelIndex)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            installedView, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            installedView, SLOT(rowsRemoved(QModelIndex,int,int)));

    //setup page for installed packages
    QWidget *installedPage = new QWidget;
    QVBoxLayout *vbInstalledPage = new QVBoxLayout( installedPage );
    vbInstalledPage->setSpacing( 2 );
    vbInstalledPage->setMargin( 2 );
    vbInstalledPage->addWidget( installedView );

    //setup view for downloadable packages
    downloadView = new QTreeView( this );
    downloadView->setModel( model );
    downloadView->setRootIndex( model->index(1,0,QModelIndex()) );
    downloadView->setRootIsDecorated( false );
    connect( downloadView, SIGNAL(activated(QModelIndex)),
            this, SLOT(activateItem(QModelIndex)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            downloadView, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            downloadView, SLOT(rowsRemoved(QModelIndex,int,int)));

    //setup page for downloadable packages
    QWidget *downloadPage = new QWidget(this);
    QVBoxLayout *vbDownloadPage = new QVBoxLayout( downloadPage );
    vbDownloadPage->setSpacing( 2 );
    vbDownloadPage->setMargin( 2 );
    vbDownloadPage->addWidget( downloadView );
    statusLabel = new QLabel( "No Server Chosen", this );
    statusLabel->setWordWrap( true );
    vbDownloadPage->addWidget( statusLabel );

    installedView->hideColumn( 1 );
    installedView->header()->hide();
    downloadView->hideColumn( 1 );
    downloadView->header()->hide();
//TODO: install to media card
     menuTarget = new QMenu( tr( "Install to" ), this );
    new PackageManagerService( this );

    tabWidget = new QTabWidget( this );
    tabWidget->addTab( installedPage, tr( "Installed" ) );
    tabWidget->addTab( downloadPage, tr( "Downloads" ) );
    setCentralWidget( tabWidget );
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
    //setup context menu for installed packages
    QWidget * installedPage = tabWidget->widget( 0 );
    QMenu *installedContext = QSoftMenuBar::menuFor( installedPage );
    detailsAction = new QAction( tr("Details"), this);
    detailsAction->setVisible( false );
    connect( detailsAction, SIGNAL(triggered()), this, SLOT(displayDetails()) );

    uninstallAction = new QAction( tr("Uninstall"), installedPage );
    uninstallAction->setVisible( false );
    connect( uninstallAction, SIGNAL(triggered()), this, SLOT(startUninstall()) );

    reenableAction = new QAction( tr("Re-enable"), installedPage );
    reenableAction->setVisible( false );
    connect( reenableAction, SIGNAL(triggered()), this, SLOT(confirmReenable()) );

    installedContext->addAction( detailsAction );
    installedContext->addAction( uninstallAction );
    installedContext->addAction( reenableAction );
    connect( installedContext, SIGNAL(aboutToShow()),
                this,   SLOT(contextMenuShow()) );

    //setup context menu for downloadable packages
    QWidget * downloadPage = tabWidget->widget( 1 );
    QMenu *downloadContext = QSoftMenuBar::menuFor( tabWidget->widget(1) );
    installAction = new QAction( tr("Install"), downloadPage );
    installAction->setVisible( false );
    connect( installAction, SIGNAL(triggered()), this, SLOT(startInstall()) );

    menuServers = new QMenu( tr( "Connect" ), this );

    QAction* editServersAction = new QAction( tr( "Edit servers" ), this );
    connect( editServersAction, SIGNAL(triggered()), this, SLOT(editServers()) );

    downloadContext->addMenu( menuServers );
    downloadContext->addAction( detailsAction );
    downloadContext->addAction( installAction );
    downloadContext->addAction( editServersAction );

    connect( downloadContext, SIGNAL(aboutToShow()),
                this,   SLOT(contextMenuShow()) );

    model->populateServers();
    QStringList servers = model->getServers();
    qSort( servers );
    QAction *sa;
    serversActionGroup = new QActionGroup( this );
    serversActionGroup->setExclusive( true );
    connect( serversActionGroup, SIGNAL(triggered(QAction*)),
             this, SLOT(serverChoice(QAction*)) );
    for ( int i = 0; i < servers.count(); i++ )
    {
        sa = new QAction( servers[i], serversActionGroup );
        serversActionGroup->addAction( sa );
    }
    menuServers->addActions( serversActionGroup->actions() );

    targetActionGroup = new QActionGroup( this );
    targetActionGroup->setExclusive( true );
    connect( targetActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(targetChoice(QAction*)) );

     waitWidget = new QWaitWidget( this );
}

void PackageView::activateItem( const QModelIndex &item )
{
    if ( model->rowCount( item ) > 0 ) // has children
    {
        if ( installedView->isExpanded( item ) )
            installedView->collapse( item );
        else
            installedView->expand( item );
    }

    QModelIndex ix = item.parent();
    if ( ix.isValid() ) // is a child node
        showDetails( item, PackageDetails::Info );
}

void PackageView::serverChoice( QAction *a )
{
    Q_UNUSED( a );
    QString server = a->text();
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

/*! \internal
    Updates list of available servers to connect to under the
    "Connect" option of the context menu.
*/
void PackageView::serversChanged( const QStringList &servers )
{
    QList<QAction*> actionList = serversActionGroup->actions();
    QStringList serverList = servers;
    qSort( serverList );
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
        sa = new QAction( serverList[i], serversActionGroup );

    actionList = serversActionGroup->actions();
    qSort( actionList.begin(), actionList.end(), actionLessThan );

    menuServers->clear();
    menuServers->addActions( actionList );
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
        sa = new QAction( targetList[i], targetActionGroup );

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

/*!
    \internal
    \fn void PackageView::targetChoiceChanged( const QString & )
    Emitted when the user choice of installation target changes, eg
    from "Internal Storage" to "CF Card"
*/


/* \a type must be either be Install or Info */
void PackageView::showDetails( const QModelIndex &item , PackageDetails::Type type )
{
    if( !item.isValid() || !item.parent().isValid()
        || !model->hasIndex(item.row(), item.column(), item.parent()) )
        return;

    if ( type != PackageDetails::Install && type != PackageDetails::Info )
        return;

    QString name = model->data( item, Qt::DisplayRole ).toString(); //package name

    PackageDetails *pd = new PackageDetails( this, type, true );
    QSoftMenuBar::setLabel(pd->description, Qt::Key_Select, QSoftMenuBar::NoLabel); //shouldn't be need once
    QSoftMenuBar::setLabel(pd->description, Qt::Key_Back, QSoftMenuBar::Back);      //softkey helpers are in place
    QtopiaApplication::setMenuLike( pd, true );

    pd->setWindowTitle( name );

    QString details;
    if ( type == PackageDetails::Install )
    {
#ifndef QT_NO_SXE
        if( model->hasSensitiveDomains(model->data(item,AbstractPackageController::Domains).toString()) )
        {
            details = QLatin1String( "<font color=\"#0000FF\">");
            details += tr( "The package <font color=\"#0000FF\">%1</font> <b>cannot be installed</b> as it utilizes protected resources" ).arg( name );
            details += QLatin1String("</font>");
            pd->description->setHtml( details );
            pd->showMaximized();
            return;
        }
        else
        {
#endif

#ifndef QT_NO_SXE
            details = QString("<font color=\"#000000\">") + "<center><b><u>" + tr( "Security Alert" ) + "</u></b></center><p>"
                    + QString("%1")
                    .arg( DomainInfo::explain( model->data( item, AbstractPackageController::Domains ).toString(), name ));
        }
#else
            details = QString("<font color=\"#000000\">") + "<center><b><u>" + tr( "Confirm Install") + " </u></b></center><p>"
                        + tr("About to install <font color=\"#0000CC\"><b> %1 </b></font>", "%1 = package name")
                            .arg( model->data( item, Qt::DisplayRole ).toString() );
#endif
            details += QString("<br>") + tr( "Confirm Install?" ) + QString("</font>");
            pd->description->setHtml( details );
            QtopiaApplication::setMenuLike( pd, true );
    }
    else //must be requesting package information
    {
        pd->description->setHtml( QString("<font color=\"#000000\">") + model->data(item, Qt::WhatsThisRole).toString() +"</font>");
        QtopiaApplication::setMenuLike( pd, false );
    }

    qLog(Package) << "show details" << ( name.isNull() ? "no valid name" : name );

    pd->showMaximized();

    int result = pd->exec();
    
    if ( type == PackageDetails::Install && result == QDialog::Accepted )
    {
            model->activateItem( item );
    }

    delete pd;
}

void PackageView::startInstall()
{
    const char *dummyStr = QT_TRANSLATE_NOOP( "PackageView", "Installing..." );
    Q_UNUSED( dummyStr );

    showDetails( downloadView->currentIndex(), PackageDetails::Install );

}

void PackageView::startUninstall()
{
    if ( QMessageBox::warning(this, tr("Confirm Uninstall?"), tr("Are you sure you wish to uninstall %1?","%1=package name")
                .arg( model->data( installedView->currentIndex(), Qt::DisplayRole ).toString()) + QLatin1String("<BR>")
                + tr("All running instances will be terminated."), QMessageBox::Yes | QMessageBox::No )
         == QMessageBox::Yes)
    {
            waitWidget->show();
            waitWidget->setText(tr("Uninstalling..."));
            model->activateItem( installedView->currentIndex() );
            installedView->setCurrentIndex(QModelIndex());
            waitWidget->hide();
    }
}

void PackageView::displayDetails()
{
    if ( tabWidget->currentIndex() == InstalledIndex )
        showDetails( installedView->currentIndex(), PackageDetails::Info );
    else
        showDetails( downloadView->currentIndex(), PackageDetails::Info );
}

void PackageView::confirmReenable()
{
    if ( QMessageBox::warning(this, tr("Confirm Re-enable?"), tr("Are you sure you wish to re-enable %1?","%1=package name")
            .arg( model->data( installedView->currentIndex(), Qt::DisplayRole ).toString()), QMessageBox::Yes | QMessageBox::No )
         == QMessageBox::Yes)
    {
        model->reenableItem( installedView->currentIndex() );
    }
}

/*!
    \internal
    Slot to be called whenever the context menu is shown.
    Sets the appropriate options available in the context menu.
*/
void PackageView::contextMenuShow()
{
    QModelIndex currIndex = ( tabWidget->currentIndex() == InstalledIndex ) ? (installedView->currentIndex())
                                                               : (downloadView->currentIndex());
    if( !currIndex.isValid() || !currIndex.parent().isValid()
        || !model->hasIndex(currIndex.row(), currIndex.column(), currIndex.parent()) )
    {
        detailsAction->setVisible( false );

        if ( tabWidget->currentIndex() == InstalledIndex )
        {
            uninstallAction->setVisible( false );
            reenableAction->setVisible( false );
        }
        else //otherwise dealing with Downloads tab
        {
            installAction->setVisible( false );
        }
    }
    else
    {
        detailsAction->setVisible( true );
        if ( tabWidget->currentIndex() == InstalledIndex )
        {
            uninstallAction->setVisible( true );
            if ( model->data( currIndex, Qt::StatusTipRole ).toBool() ) //if package is enabled
                reenableAction->setVisible( false );
            else
                reenableAction->setVisible( true );
        }
        else //Downloads tab
        {
            installAction->setVisible( true );
        }
    }
}

/*!
  \internal
  Displays the status of the server
*/
void PackageView::postServerStatus( const QString &status )
{
    statusLabel->setText( status );
}

/*!
  \internal
  Show a newly installed package as the currently selected package.
*/
void PackageView::selectNewlyInstalled( const QModelIndex &index)
{
    tabWidget->setCurrentIndex( PackageView::InstalledIndex );
    installedView->setCurrentIndex( index );
    QMessageBox::information( this, tr("Successful Installation"), tr("Package successfully installed" ) );
}


