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

#include <qtopiaglobal.h>
#include <qtopiaipcenvelope.h>
#include <qcontent.h>

#include <qtopiaapplication.h>
#include <qmimetype.h>
#include <qcategorymanager.h>
#include <qtopiaservices.h>
#include <qdocumentproperties.h>
#include <custom.h>
#include <qpassworddialog.h>
#include <qtopialog.h>

#include <qdir.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <QTimer>
#include <QComboBox>
#include <QStyle>
#include <QPushButton>
#include <QTabBar>
#include <QLayout>
#include <QRegExp>
#include <QMessageBox>
#include <QFrame>
#include <QPainter>
#include <QLabel>
#include <QTextStream>
#include <QSettings>

#include <QStackedWidget>
#include <QTabBar>
#include <QMenu>
#include <QDesktopWidget>
#include <QDebug>

#include "startmenu.h"
#include "taskbar.h"

#include "windowmanagement.h"
#include "launcherview.h"
#include "launcher.h"
#include "launchertab.h"
#include "server.h"
#include <stdlib.h>
#include <assert.h>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

#include "servertask.h"
QTOPIA_TASK(PDAQtopiaUI, Launcher);

// define LauncherTabWidget
LauncherTabWidget::LauncherTabWidget( Launcher* parent ) :
    QWidget( parent )
{
    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->setMargin(0);
    vbl->setSpacing(0);
    launcher = parent;
    categoryBar = new LauncherTabBar( this );
    vbl->addWidget(categoryBar);
    QPalette pal = categoryBar->palette();
    pal.setColor( QPalette::Light, pal.color(QPalette::Active,QPalette::Shadow) );
    categoryBar->setPalette( pal );
    stack = new QStackedWidget(this);
    vbl->addWidget(stack);
    connect( categoryBar, SIGNAL(currentChanged(int)), this, SLOT(raiseTabWidget()) );

    QtopiaChannel *channel = new QtopiaChannel( "QPE/LauncherSettings", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(launcherMessage(const QString&,const QByteArray&)) );
    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
             this, SLOT(appMessage(const QString&,const QByteArray&)));
}

void LauncherTabWidget::initLayout()
{
    layout()->activate();
    docView()->setFocus();
    showTab("Applications");
}

void LauncherTabWidget::appMessage(const QString& message, const QByteArray&)
{
    // TODO: This message is no longer sent on QPE/System, it is now a service
    if ( message == "nextView()" ) {
        int n = categoryBar->count();
        int tab = categoryBar->currentIndex();
        if ( tab >= 0 )
            categoryBar->setCurrentIndex( (tab - 1 + n)%n );
    }
}

void LauncherTabWidget::raiseTabWidget()
{
    LauncherView * view = categoryBar->currentView();
    if(view)
        stack->setCurrentWidget(view);
}

void LauncherTabWidget::tabProperties()
{
    LauncherView *view = static_cast<LauncherView *>(stack->currentWidget());
    QMenu *m = new QMenu( this );
    QAction *iconAction = m->addAction(tr("Icon View"));
    if (view->viewMode() == LauncherView::Icon)
        iconAction->setChecked(true);
    QAction *listAction = m->addAction(tr("List View"));
    if (view->viewMode() == LauncherView::List)
        listAction->setChecked(true);
    QAction *result = m->exec( QCursor::pos() );
    if (result == iconAction)
        view->setViewMode(LauncherView::Icon);
    else
        view->setViewMode(LauncherView::List);

    delete m;
}

void LauncherTabWidget::deleteView( const QString& id )
{
    LauncherTab * t = categoryBar->lookup(id);
    if(t) {
        stack->removeWidget(t->view);
        delete t->view;
        categoryBar->removeTab(id);
    }
}

LauncherView* LauncherTabWidget::newView( const QString& id, const QIcon &icon, const QString &label )
{
    LauncherView* view = new ApplicationLauncherView( id, stack );

    stack->addWidget(view);
    connect( view, SIGNAL(clicked(QContent)),
            this, SIGNAL(clicked(QContent)));
    connect( view, SIGNAL(rightPressed(QContent)),
            this, SIGNAL(rightPressed(QContent)));

    LauncherTab * tab;
    if ( id == "Documents" ) {
        // Documents tab should always be at the end
        tab = categoryBar->addTab(id, label, icon, true);
        docview = view;
    } else {
        tab = categoryBar->addTab(id, label, icon);
    }
    tab->view = view;

    qLog(UI) << "inserting " << id.toAscii() << " into launcher tab";

    QSettings cfg("Trolltech","Launcher");
    setTabAppearance( tab, id, cfg );

    return view;
}

LauncherView *LauncherTabWidget::view( const QString &id )
{
    LauncherTab * t = categoryBar->lookup(id);
    if(t)
        return t->view;
    else
        return 0; // Not found
}

LauncherView *LauncherTabWidget::docView()
{
    return docview;
}

void LauncherTabWidget::layoutTabs()
{
    categoryBar->layoutTabs();
}

// ### this function could more to LauncherView
void LauncherTabWidget::setTabViewAppearance( LauncherView *v, QSettings &cfg )
{
    // View
    QString view = cfg.value( "View", "Icon" ).toString();
    if ( view == "List" ) // No tr
        v->setViewMode( LauncherView::List );

    QString bgType = cfg.value( "BackgroundType" ).toString();

    if ( bgType == "Image" ) { // No tr

        QString pm = cfg.value( "BackgroundImage" ).toString();
        v->setBackgroundType( LauncherView::Image, pm );

    } else if ( bgType == "SolidColor" ) {
        QString c = cfg.value( "BackgroundColor" ).toString();
        v->setBackgroundType( LauncherView::SolidColor, c );

    } else {
        v->setBackgroundType( LauncherView::Ruled, QString() );
    }

    QString textCol = cfg.value( "TextColor" ).toString();

    if ( textCol.isEmpty() )
        v->setTextColor( QColor() );

    else
        v->setTextColor( QColor(textCol) );
    bool customFont = cfg.value( "CustomFont", false ).toBool();

    if ( customFont ) {
        QStringList font = cfg.value( "Font").toString().split( ',' );

    if ( font.count() == 4 )
            v->setViewFont( QFont(font[0], font[1].toInt(), font[2].toInt(), font[3].toInt()!=0) );
    } else {

        v->clearViewFont();
    }
}

void LauncherTabWidget::setTabAppearance( LauncherTab *tab,
                                          const QString &type,
                                          QSettings &cfg )
{
    cfg.beginGroup( QString( "Tab %1" ).arg(type) ); // No tr

    setTabViewAppearance( tab->view, cfg );

    // Tabs
    QString tabCol = cfg.value( "TabColor" ).toString();
    if ( tabCol.isEmpty() )
        tab->bgColor = QColor();
    else
        tab->bgColor = QColor(tabCol);
    QString tabTextCol = cfg.value( "TabTextColor" ).toString();
    if ( tabTextCol.isEmpty() )
        tab->fgColor = QColor();
    else
        tab->fgColor = QColor(tabTextCol);

    cfg.endGroup();
}

void LauncherTabWidget::showTab(const QString& id)
{
    categoryBar->setCurrentTab(id);
}

void LauncherTabWidget::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QPalette pal = palette();
        pal.setColor( QPalette::Light, pal.color(QPalette::Active,QPalette::Shadow) );
        categoryBar->setPalette( pal );
    }

    QWidget::changeEvent(e);
}

void LauncherTabWidget::setBusy(bool on)
{
    if ( on )
        currentView()->setBusy(true);
    else {
        for(int ii = 0; ii < stack->count(); ii++)
            static_cast<LauncherView *>(stack->widget(ii))->setBusy(false);
    }
}

LauncherView *LauncherTabWidget::currentView(void)
{
    return (LauncherView*)stack->currentWidget();
}

void LauncherTabWidget::launcherMessage( const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );
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
    } else if ( msg == "setTextColor(QString,QString)" ) {
        QString id;
        stream >> id;
        QString color;
        stream >> color;
        if ( view(id) )
            view(id)->setTextColor( QColor(color) );
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
            } else {
                view(id)->clearViewFont();
            }
        }
    }
}



//---------------------------------------------------------------------------

/*!
  \class Launcher
  \internal
  */
Launcher::Launcher()
: QWidget(0, Qt::FramelessWindowHint), categories(0)
{
    setAttribute(Qt::WA_GroupLeader);
    tabs = 0;
    tb = 0;
    delayedAppLnk = 0;
    tid_today = startTimer(3600*2*1000);
    last_today_show = QDate::currentDate();

    setWindowTitle( tr("Launcher") );

    // we have a pretty good idea how big we'll be
    setGeometry( 0, 0, qApp->desktop()->width(), qApp->desktop()->height() );

    tb = new TaskBar;
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);
    vb->setSpacing(0);
    tabs = new LauncherTabWidget( this );
    vb->addWidget(tabs);

    WindowManagement::instance()->dockWindow( tb, WindowManagement::Bottom );

    qApp->installEventFilter( this );

    connect( qApp, SIGNAL(authenticate(bool)), this, SLOT(askForPin(bool)) );

    connect( tb, SIGNAL(tabSelected(const QString&)),
        this, SLOT(showTab(const QString&)) );
    connect( tabs, SIGNAL(selected(const QString&)),
        this, SLOT(viewSelected(const QString&)) );
    connect( tabs, SIGNAL(clicked(QContent)),
        this, SLOT(select(QContent)));
    connect( tabs, SIGNAL(rightPressed(QContent)),
        this, SLOT(properties(QContent)));

    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(systemMessage(const QString&,const QByteArray&)) );

    // all documents
    QImage img( QImage( ":image/qpe/DocsIcon" ) );
    int smallIconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPixmap pm = QPixmap::fromImage(img.scaled( smallIconSize, smallIconSize ));
    // It could add this itself if it handles docs
    // akennedy
    tabs->newView("Documents", pm, tr("Documents") )->setToolsEnabled( true );
    QTimer::singleShot( 0, tabs, SLOT(initLayout()) );

    // Setup all types
    categories = new QCategoryManager("Applications", this);
    QObject::connect(categories, SIGNAL(categoriesChanged()),
                     this, SLOT(categoriesChanged()));
    categoriesChanged();

}

void Launcher::categoriesChanged()
{
    QStringList cats = categories->categoryIds();
    for(int ii = 0; ii < cats.count(); ++ii) {
        const QString & cat = cats.at(ii);
        if(!categories->isGlobal(cat) && !currentCategories.contains(cat)) {
            addType(cat, categories->label(cat), categories->icon(cat));
        }
    }

    // Inefficient but small set so should not be a problem
    for(QStringList::Iterator iter = currentCategories.begin();
            iter != currentCategories.end();) {
        if(!cats.contains(*iter)) {
            removeType(*iter);
            iter = currentCategories.erase(iter);
        } else {
            ++iter;
        }
    }
}

void Launcher::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == tid_today ) {
        QDate today = QDate::currentDate();
        if ( today != last_today_show ) {
            last_today_show = today;
            QSettings cfg("Trolltech","today");
            cfg.beginGroup("Start");
#ifndef QPE_DEFAULT_TODAY_MODE
#define QPE_DEFAULT_TODAY_MODE "Never"
#endif
            if ( cfg.value("Mode",QPE_DEFAULT_TODAY_MODE).toString() == "Daily" ) {
                QtopiaIpcEnvelope env(QtopiaService::channel("today"),"raise()");
            }
        }
    }
}

void Launcher::showEvent(QShowEvent *e)
{
    tb->show();
    QTimer::singleShot( 0, this, SLOT( makeVisible() ) );
    QWidget::showEvent(e);
}

Launcher::~Launcher()
{
    if ( tb ) {
    delete tb;
    tb = 0;
    }
    if ( tabs ) {
    delete tabs;
        tabs = 0;
    }
}

void Launcher::makeVisible()
{
    showMaximized();
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
            return true;
        }
    }
#else
    Q_UNUSED(ev);
#endif
    return false;
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
    QPasswordDialog::authenticateUser(this, apo);
    ServerApplication::lockScreen(false);
}

static bool isVisibleWindow(int wid)
{
#ifdef Q_WS_QWS
    const QList<QWSWindow*> &list = qwsServer->clientWindows();
    foreach (QWSWindow* w, list) {
        if ( w->winId() == wid )
            return !w->isFullyObscured();
    }
#endif
    return false;
}

void Launcher::viewSelected(const QString& s)
{
    setWindowTitle( s + tr(" - Launcher") );
}

void Launcher::showTab(const QString& id)
{
    tabs->showTab(id);
    raise();
}

void Launcher::select( const QContent *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
        // Not supported: flat is simpler for the user
    } else {
      if ( appLnk->executableName().isNull() ) {
            if ( ! delayedAppLnk ) {
                delayedAppLnk = new QContent(*appLnk);
                QTimer::singleShot( 0, this, SLOT(delayedSelect()) );
            }
        } else {
            tabs->setBusy(true);
            emit executing( appLnk );
            appLnk->execute();
        }
    }
}

void Launcher::delayedSelect()
{
    if ( !delayedAppLnk )
        return;
    QContent *appLnk = delayedAppLnk;
    delayedAppLnk = 0;

    int i = QMessageBox::information(this,tr("No application"),
            tr("<qt>No application is defined for this document. "
                "Type is %1.</qt>").arg(appLnk->type()),
            tr("OK"), tr("View as text"), 0, 0, 1);

    if ( QFileInfo(appLnk->file()).exists() && i == 1 )
        Qtopia::execute(QtopiaService::app("Open/text/*"),appLnk->file());

    delete appLnk;
}

void Launcher::properties( const QContent *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
        // Not supported: flat is simpler for the user
    } else {
        if ( ! delayedAppLnk ) {
            delayedAppLnk = new QContent(*appLnk);
            QTimer::singleShot( 0, this, SLOT(delayedProperties()) );
        }
    }
}

void Launcher::delayedProperties()
{
    if ( !delayedAppLnk )
        return;
    QContent *appLnk = delayedAppLnk;
    delayedAppLnk = 0;

    QDocumentPropertiesDialog prop(*appLnk);
    prop.showMaximized();
    prop.exec();

    delete appLnk;
}

void Launcher::systemMessage( const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );
    if ( msg == "busy()" ) {
        tb->startWait();
    } else if ( msg == "notBusy(QString)" ) {
        QString app;
        stream >> app;
        tabs->setBusy(false);
        tb->stopWait(app);
    } else if (msg == "applyStyle()") {
        tabs->layoutTabs();
    }
}

// These are the update functions from the server
void Launcher::addType(const QString& type, const QString& name,
                                     const QIcon &iconconst)
{
    TypeView tv;

    tv.view =   tabs->newView( type, iconconst, name );
    ids.append( type );
    tb->refreshStartMenu();

    tv.view->setObjectName(type);
    map[QLatin1String("Folder/")+type] = tv;

    static bool first = true;
    if ( first ) {
        first = false;
        tabs->showTab(type);
    }

    tabs->view( type )->setUpdatesEnabled( true);
    tabs->view( type )->setSortEnabled( false );
}

void Launcher::removeType(const QString& type)
{
    tabs->view( type )->removeAllItems();
    tabs->deleteView( type );
    ids.removeAll( type );
    tb->refreshStartMenu();
}

#ifdef QTOPIA4_TODO
void Launcher::applicationAdded( const QString& type, const QContent& app )
{
    if ( type == "Separator" )  // No tr
        return;

    if (map.contains("Folder/"+type)) {

      LauncherView *view = map["Folder/"+type].view;
      qLog(DocAPI) << "PdaLauncher::applicationAdded" << type << "," << app.name() << "(cid =" << app.id();
      qLog(DocAPI) << "icon path =" << app.iconName();
      view->addItem(new QContent(app));
  }
}

void Launcher::applicationRemoved(  QContentId id )
{

   QMap<QString, TypeView>::Iterator it;
   for (it = map.begin(); it != map.end(); ++it) {
     (*it).view->removeLink(id);
   }
}

void Launcher::allApplicationsRemoved()
{
   QMap<QString, TypeView>::Iterator it;
   for (it = map.begin(); it != map.end(); ++it) {
     (*it).view->removeAllItems();
   }
}

void Launcher::documentAdded( QContentId doc )
{
    if(doc == QContent::InvalidId)
    {
        qWarning() << "trying to add an invalid id to the document view";
        return;
    }
  tabs->docView()->addItem( new QContent( doc ), false );
 /* if (!actionProps->isEnabled())
    actionProps->setEnabled(true);
  if (!actionDelete->isEnabled())
    actionDelete->setEnabled(true);
  if (!actionBeam->isEnabled())
    actionBeam->setEnabled(true);
  separatorAction->setEnabled(true);
 */
  if (tabs->docView()->isVisible()) {
    tabs->docView()->sort();
 //   tabs->docViewNeedsSort = false;
  } else {
 //   tabs->docViewNeedsSort = true;
  }
}

void Launcher::documentRemoved( QContentId doc )
{

  if ( tabs->docView()->removeLink( doc ) ) {
  if ( tabs->docView()->isVisible()) {
  tabs->docView()->sort();
//      docViewNeedsSort = false;
    } else {
   //   docViewNeedsSort = true;
    }
  if (! tabs->docView()->count()) {
//      actionProps->setEnabled(false);
//      actionDelete->setEnabled(false);
//      actionBeam->setEnabled(false);
//      separatorAction->setEnabled(false);
    }
  }

}

void Launcher::allDocumentsRemoved()
{
  /*
  actionProps->setEnabled(false);
  actionDelete->setEnabled(false);
  actionBeam->setEnabled(false);
  separatorAction->setEnabled(false);
  */
}

void Launcher::documentChanged( QContentId id )
{
  documentRemoved(id);
  documentAdded(id);

}

void Launcher::applicationStateChanged( const QString& name, ApplicationState state )
{
    tb->setApplicationState( name, state );
}
#endif

void Launcher::showDocTab()
{
    if ( tabs->currentView() == tabs->docView() )
        tabs->docView()->show();
}

