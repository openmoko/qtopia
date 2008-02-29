/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/qpeglobal.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/resource.h>
#include <qtopia/applnk.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/mimetype.h>
#include <qtopia/categories.h>
#include <qtopia/services.h>
#include <qtopia/custom.h>
#include <qtopia/password.h>

#include <qdir.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <qtimer.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <qframe.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qpopupmenu.h>

#include "startmenu.h"
#include "taskbar.h"

#include "serverinterface.h"
#include "launcherview.h"
#include "launcher.h"
#include "server.h"
#include <qtopia/docproperties.h>
#include <stdlib.h>
#include <assert.h>

#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <unistd.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

#ifdef Q_WS_QWS
#include <qkeyboard_qws.h>
#endif


//===========================================================================

LauncherTabWidget::LauncherTabWidget( Launcher* parent ) :
    QVBox( parent )
{
    docLoadingWidget = 0;
    launcher = parent;
    categoryBar = new LauncherTabBar( this );
    QPalette pal = categoryBar->palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );
    stack = new QWidgetStack(this);
    connect( categoryBar, SIGNAL(selected(int)), this, SLOT(raiseTabWidget()) );
    categoryBar->show();
    stack->show();

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel *channel = new QCopChannel( "QPE/Launcher", this );
    connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
             this, SLOT(launcherMessage(const QCString&,const QByteArray&)) );
    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
	     this, SLOT(appMessage(const QCString&,const QByteArray&)));
#endif

    docLoadingWidget = new LoadingWidget( stack );
    Config cfg("Launcher");
    cfg.setGroup( "Tab Documents" ); // No tr
    setTabViewAppearance( docLoadingWidget, cfg );

    stack->addWidget( docLoadingWidget, 0 );
}


void LauncherTabWidget::initLayout()
{
    layout()->activate();
    docView()->setFocus();
    categoryBar->showTab("Documents");
}

void LauncherTabWidget::appMessage(const QCString& message, const QByteArray&)
{
    if ( message == "nextView()" )
	categoryBar->nextTab();
}

void LauncherTabWidget::raiseTabWidget()
{
    if ( categoryBar->currentView() == docView()
	 && docLoadingWidget->enabled() ) {
	stack->raiseWidget( docLoadingWidget );
	docLoadingWidget->updateGeometry();
    } else {
	stack->raiseWidget( categoryBar->currentView() );
    }
}

void LauncherTabWidget::tabProperties()
{
    LauncherView *view = categoryBar->currentView();
    QPopupMenu *m = new QPopupMenu( this );
    m->insertItem( tr("Icon View"), LauncherView::Icon );
    m->insertItem( tr("List View"), LauncherView::List );
    m->setItemChecked( (int)view->viewMode(), TRUE );
    int rv = m->exec( QCursor::pos() );
    if ( rv >= 0 && rv != view->viewMode() ) {
	view->setViewMode( (LauncherView::ViewMode)rv );
    }

    delete m;
}

void LauncherTabWidget::deleteView( const QString& id )
{
    LauncherTab *t = categoryBar->launcherTab(id);
    if ( t ) {
	stack->removeWidget( t->view );
	delete t->view;
	categoryBar->removeTab( t );
    }
}

LauncherView* LauncherTabWidget::newView( const QString& id, const QPixmap& pm, const QString& label )
{
    LauncherView* view = new LauncherView( stack );
    connect( view, SIGNAL(clicked(const AppLnk*)),
	    this, SIGNAL(clicked(const AppLnk*)));
    connect( view, SIGNAL(rightPressed(const AppLnk*)),
	    this, SIGNAL(rightPressed(const AppLnk*)));

    int n = categoryBar->count();
    stack->addWidget( view, n );

    LauncherTab *tab = new LauncherTab( id, view, pm, label );
    categoryBar->insertTab( tab, n-1 );

    if ( id == "Documents" )
	docview = view;

    qDebug("inserting %s at %d", id.latin1(), n-1 );

    Config cfg("Launcher");
    setTabAppearance( tab, cfg );
    return view;
}

LauncherView *LauncherTabWidget::view( const QString &id )
{
    LauncherTab *t = categoryBar->launcherTab(id);
    if ( !t )
	return 0;
    return t->view;
}

LauncherView *LauncherTabWidget::docView()
{
    return docview;
}

void LauncherTabWidget::setLoadingWidgetEnabled( bool v )
{
    if ( docLoadingWidget && v != docLoadingWidget->enabled() ) {
	docLoadingWidget->setEnabled( v );
	raiseTabWidget();
    }
}

void LauncherTabWidget::setLoadingProgress( int percent )
{
    docLoadingWidget->setProgress( (percent / 4) * 4 );
}

// ### this function could more to LauncherView
void LauncherTabWidget::setTabViewAppearance( LauncherView *v, Config &cfg )
{
    // View
    QString view = cfg.readEntry( "View", "Icon" );
    if ( view == "List" ) // No tr
	v->setViewMode( LauncherView::List );
    QString bgType = cfg.readEntry( "BackgroundType" );
    if ( bgType == "Image" ) { // No tr
	QString pm = cfg.readEntry( "BackgroundImage" );
	v->setBackgroundType( LauncherView::Image, pm );
    } else if ( bgType == "SolidColor" ) {
	QString c = cfg.readEntry( "BackgroundColor" );
	v->setBackgroundType( LauncherView::SolidColor, c );
    } else {
	v->setBackgroundType( LauncherView::Ruled, QString::null );
    }
    QString textCol = cfg.readEntry( "TextColor" );
    if ( textCol.isEmpty() )
	v->setTextColor( QColor() );
    else
	v->setTextColor( QColor(textCol) );
    bool customFont = cfg.readBoolEntry( "CustomFont", FALSE );
    if ( customFont ) {
	QStringList font = cfg.readListEntry( "Font", ',' );
	if ( font.count() == 4 )
	    v->setViewFont( QFont(font[0], font[1].toInt(), font[2].toInt(), font[3].toInt()!=0) );
    } else {
	v->clearViewFont();
    }
}

// ### Could move to LauncherTab
void LauncherTabWidget::setTabAppearance( LauncherTab *tab, Config &cfg )
{
    cfg.setGroup( QString( "Tab %1" ).arg(tab->type) ); // No tr

    setTabViewAppearance( tab->view, cfg );

    // Tabs
    QString tabCol = cfg.readEntry( "TabColor" );
    if ( tabCol.isEmpty() )
	tab->bgColor = QColor();
    else
	tab->bgColor = QColor(tabCol);
    QString tabTextCol = cfg.readEntry( "TabTextColor" );
    if ( tabTextCol.isEmpty() )
	tab->fgColor = QColor();
    else
	tab->fgColor = QColor(tabTextCol);
}

void LauncherTabWidget::paletteChange( const QPalette &p )
{
    QVBox::paletteChange( p );
    QPalette pal = palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );
    categoryBar->update();
}

void LauncherTabWidget::styleChange( QStyle & )
{
    QTimer::singleShot( 0, this, SLOT(setProgressStyle()) );
}

void LauncherTabWidget::setProgressStyle()
{
    if (docLoadingWidget) 
	docLoadingWidget->setProgressStyle();
}

void LauncherTabWidget::setBusy(bool on)
{
    if ( on )
	currentView()->setBusy(TRUE);
    else {
	for ( int i = 0; i < categoryBar->count(); i++ ) {
	    LauncherView *view = ((LauncherTab *)categoryBar->tab(i))->view;
	    view->setBusy( FALSE );
	}
    }
}

LauncherView *LauncherTabWidget::currentView(void)
{
    return (LauncherView*)stack->visibleWidget();
}

void LauncherTabWidget::launcherMessage( const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "setTabView(QString,int)" ) {
	QString id;
	stream >> id;
	int mode;
	stream >> mode;
	if ( view(id) )
	    view(id)->setViewMode( (LauncherView::ViewMode)mode );
    } else if ( msg == "setTabBackground(QString,int,QString)" ) {
	QString id;
	stream >> id;
	int mode;
	stream >> mode;
	QString pixmapOrColor;
	stream >> pixmapOrColor;
	if ( view(id) )
	    view(id)->setBackgroundType( (LauncherView::BackgroundType)mode, pixmapOrColor );
	if ( id == "Documents" )
	    docLoadingWidget->setBackgroundType( (LauncherView::BackgroundType)mode, pixmapOrColor );
    } else if ( msg == "setTextColor(QString,QString)" ) {
	QString id;
	stream >> id;
	QString color;
	stream >> color;
	if ( view(id) )
	    view(id)->setTextColor( QColor(color) );
	if ( id == "Documents" )
	    docLoadingWidget->setTextColor( QColor(color) );
    } else if ( msg == "setFont(QString,QString,int,int,int)" ) {
	QString id;
	stream >> id;
	QString fam;
	stream >> fam;
	int size;
	stream >> size;
	int weight;
	stream >> weight;
	int italic;
	stream >> italic;
	if ( view(id) ) {
	    if ( !fam.isEmpty() ) {
		view(id)->setViewFont( QFont(fam, size, weight, italic!=0) );
		qDebug( "setFont: %s, %d, %d, %d", fam.latin1(), size, weight, italic );
	    } else {
		view(id)->clearViewFont();
	    }
	}
    }
}



//---------------------------------------------------------------------------

Launcher::Launcher()
    : QMainWindow( 0, "PDA User Interface", QWidget::WStyle_Customize | QWidget::WGroupLeader )
{
    tabs = 0;
    tb = 0;
    delayedAppLnk = 0;
    tid_today = startTimer(3600*2*1000);
    last_today_show = QDate::currentDate();
}

void Launcher::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == tid_today ) {
	QDate today = QDate::currentDate();
	if ( today != last_today_show ) {
	    last_today_show = today;
	    Config cfg("today");
	    cfg.setGroup("Start");
#ifndef QPE_DEFAULT_TODAY_MODE
#define QPE_DEFAULT_TODAY_MODE "Never"
#endif
	    if ( cfg.readEntry("Mode",QPE_DEFAULT_TODAY_MODE) == "Daily" ) {
		QCopEnvelope env(Service::channel("today"),"raise()");
	    }
	}
    }
}

void Launcher::createGUI()
{
    setCaption( tr("Launcher") );

    // we have a pretty good idea how big we'll be
    setGeometry( 0, 0, qApp->desktop()->width(), qApp->desktop()->height() );

    tb = new TaskBar;
    tabs = new LauncherTabWidget( this );
    setCentralWidget( tabs );

    ServerInterface::dockWidget( tb, ServerInterface::Bottom );

    qApp->installEventFilter( this );

    connect( qApp, SIGNAL(authenticate(bool)), this, SLOT(askForPin(bool)) );

    connect( tb, SIGNAL(tabSelected(const QString&)),
	this, SLOT(showTab(const QString&)) );
    connect( tabs, SIGNAL(selected(const QString&)),
	this, SLOT(viewSelected(const QString&)) );
    connect( tabs, SIGNAL(clicked(const AppLnk*)),
	this, SLOT(select(const AppLnk*)));
    connect( tabs, SIGNAL(rightPressed(const AppLnk*)),
	this, SLOT(properties(const AppLnk*)));

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel* sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString&,const QByteArray&)),
             this, SLOT(systemMessage(const QCString&,const QByteArray&)) );
#endif

    // all documents
    QImage img( Resource::loadImage( "DocsIcon" ) );
    QPixmap pm;
    pm = img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() );
    // It could add this itself if it handles docs
    tabs->newView("Documents", pm, tr("Documents") )->setToolsEnabled( TRUE );
    QTimer::singleShot( 0, tabs, SLOT( initLayout() ) );

    qApp->setMainWidget( this );
}

void Launcher::showGUI()
{
    tb->show();
    QTimer::singleShot( 0, this, SLOT( makeVisible() ) );
}

Launcher::~Launcher()
{
    if ( tb )
	destroyGUI();
}

void Launcher::makeVisible()
{
    showMaximized();
}

void Launcher::destroyGUI()
{
    delete tb;
    tb = 0;
    delete tabs;
    tabs =0;
}

bool Launcher::eventFilter( QObject*, QEvent *ev )
{
#ifdef QT_QWS_SL5XXX
    if ( ev->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent *)ev;
	if ( ke->key() == Qt::Key_F11 ) { // menu key
	    QWidget *active = qApp->activeWindow();
	    if ( active && active->isPopup() )
		active->close();
	    else {
		Global::terminateBuiltin("calibrate"); // No tr
		tb->launchStartMenu();
	    }
	    return TRUE;
	}
    }
#else
    Q_UNUSED(ev);
#endif
    return FALSE;
}

void Launcher::toggleSymbolInput()
{
    tb->toggleSymbolInput();
}

void Launcher::toggleNumLockState()
{
    tb->toggleNumLockState();
}

void Launcher::toggleCapsLockState()
{
    tb->toggleCapsLockState();
}

void Launcher::askForPin(bool apo)
{
    Password::authenticate(apo);
    ServerApplication::lockScreen(FALSE);
}

static bool isVisibleWindow(int wid)
{
#ifdef Q_WS_QWS
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->winId() == wid )
	    return !w->isFullyObscured();
    }
#endif
    return FALSE;
}

void Launcher::viewSelected(const QString& s)
{
    setCaption( s + tr(" - Launcher") );
}

void Launcher::showTab(const QString& id)
{
    tabs->categoryBar->showTab(id);
    raise();
}

void Launcher::select( const AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
	// Not supported: flat is simpler for the user
    } else {
	if ( appLnk->exec().isNull() ) {
	    if ( ! delayedAppLnk ) {
		delayedAppLnk = new AppLnk(*appLnk);
		QTimer::singleShot( 0, this, SLOT(delayedSelect()) );
	    }
	} else {
	    tabs->setBusy(TRUE);
	    emit executing( appLnk );
	    appLnk->execute();
	}
    }
}

void Launcher::delayedSelect()
{
    if ( !delayedAppLnk )
	return;
    AppLnk *appLnk = delayedAppLnk;
    delayedAppLnk = 0;

    int i = QMessageBox::information(this,tr("No application"),
	    tr("<qt>No application is defined for this document. "
		"Type is %1.</qt>").arg(appLnk->type()),
	    tr("OK"), tr("View as text"), 0, 0, 1);

    if ( ! QFileInfo(appLnk->file()).exists() )
	qDebug( "file no longer exists!" );
    else if ( i == 1 )
	Global::execute(Service::app("Open/text/*"),appLnk->file());

    delete appLnk;
}

void Launcher::properties( const AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
	// Not supported: flat is simpler for the user
    } else {
	if ( ! delayedAppLnk ) {
	    delayedAppLnk = new AppLnk(*appLnk);
	    QTimer::singleShot( 0, this, SLOT(delayedProperties()) );
	}
    }
}

void Launcher::delayedProperties()
{
    if ( !delayedAppLnk )
	return;
    AppLnk *appLnk = delayedAppLnk;
    delayedAppLnk = 0;

    DocPropertiesDialog prop(appLnk,0,
	    appLnk->isDocLnk() ? "document-properties" // No tr
	    : "apps-properties"); // No tr
    prop.showMaximized();
    prop.exec();

    delete appLnk;
}

void Launcher::storageChanged( const QList<FileSystem> &/*fs*/ )
{
    // ### update combo boxes if we had a combo box for the storage type
}

void Launcher::systemMessage( const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "busy()" ) {
	tb->startWait();
    } else if ( msg == "notBusy(QString)" ) {
	QString app;
	stream >> app;
	tabs->setBusy(FALSE);
	tb->stopWait(app);
    } else if (msg == "applyStyle()") {
        tabs->categoryBar->layoutTabs();
    }
}

// These are the update functions from the server
void Launcher::typeAdded( const QString& type, const QString& name,
				    const QPixmap& pixmap, const QPixmap& )
{
    tabs->newView( type, pixmap, name );
    ids.append( type );
    tb->refreshStartMenu();

    static bool first = TRUE;
    if ( first ) {
	first = FALSE;
        tabs->categoryBar->showTab(type);
    }

    tabs->view( type )->setUpdatesEnabled( FALSE );
    tabs->view( type )->setSortEnabled( FALSE );
}

void Launcher::typeRemoved( const QString& type )
{
    tabs->view( type )->removeAllItems();
    tabs->deleteView( type );
    ids.remove( type );
    tb->refreshStartMenu();
}

void Launcher::applicationAdded( const QString& type, const AppLnk& app )
{
    if ( app.type() == "Separator" )  // No tr
	return;

    LauncherView *view = tabs->view( type );
    if ( view )
	view->addItem( new AppLnk( app ), FALSE );
    else
	qWarning("addAppLnk: No view for type %s. Can't add app %s!",
				  type.latin1(),app.name().latin1()  );

    MimeType::registerApp( app );
}

void Launcher::applicationRemoved( const QString& type, const AppLnk& app )
{
    LauncherView *view = tabs->view( type );
    if ( view )
	view->removeLink( app.linkFile() );
    else
	qWarning("removeAppLnk: No view for %s!", type.latin1() );
}

void Launcher::allApplicationsRemoved()
{
    MimeType::clear();
    for ( QStringList::ConstIterator it=ids.begin(); it!= ids.end(); ++it)
	tabs->view( (*it) )->removeAllItems();
}

void Launcher::documentAdded( const DocLnk& doc )
{
    tabs->docView()->addItem( new DocLnk( doc ), FALSE );
}

void Launcher::showLoadingDocs()
{
    tabs->docView()->hide();
}

void Launcher::showDocTab()
{
    if ( tabs->categoryBar->currentView() == tabs->docView() )
	tabs->docView()->show();
}

void Launcher::documentRemoved( const DocLnk& doc )
{
    QString file;
    if ( doc.linkFileKnown() )
	file = doc.linkFile();
    else if ( doc.fileKnown() )
	file = doc.file();
    else
	return;
    tabs->docView()->removeLink( file );
}

void Launcher::documentChanged( const DocLnk& oldDoc, const DocLnk& newDoc )
{
    documentRemoved( oldDoc );
    documentAdded( newDoc );
}

void Launcher::allDocumentsRemoved()
{
    tabs->docView()->removeAllItems();
}

void Launcher::applicationStateChanged( const QString& name, ApplicationState state )
{
    tb->setApplicationState( name, state );
}

void Launcher::applicationScanningProgress( int percent )
{
    switch ( percent ) {
        case 0: {
	    for ( QStringList::ConstIterator it=ids.begin(); it!= ids.end(); ++it) {
		tabs->view( (*it) )->setUpdatesEnabled( FALSE );
		tabs->view( (*it) )->setSortEnabled( FALSE );
	    }
	    break;
        }
        case 100: {
	    for ( QStringList::ConstIterator it=ids.begin(); it!= ids.end(); ++it) {
		tabs->view( (*it) )->setSortEnabled( TRUE );
		tabs->view( (*it) )->setUpdatesEnabled( TRUE );
	    }
	    break;
        }
        default:
            break;
    }
}

void Launcher::documentScanningProgress( int percent )
{
    switch ( percent ) {
        case 0: {
	    tabs->setLoadingProgress( 0 );
	    tabs->setLoadingWidgetEnabled( TRUE );
	    tabs->docView()->setUpdatesEnabled( FALSE );
	    tabs->docView()->setSortEnabled( FALSE );
	    break;
        }
        case 100: {
	    tabs->docView()->updateTools();
	    tabs->docView()->setSortEnabled( TRUE );
	    tabs->docView()->setUpdatesEnabled( TRUE );
	    tabs->setLoadingWidgetEnabled( FALSE );
	    break;
        }
        default:
	    tabs->setLoadingProgress( percent );
            break;
    }
}

