/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_FD

#include "mainwindow.h"
#include "todoentryimpl.h"
#include "todotable.h"

#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/ir.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpemessagebox.h>
#include <qtopia/resource.h>
#include <qtopia/pim/task.h>
#include <qtopia/pim/private/taskio_p.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/categoryselect.h>
#include <qtopia/categories.h>
#include <qtopia/pim/contact.h>

#include <qaction.h>
#include <qarray.h>
#include <qdatastream.h>
#include <qdatetime.h>
#if defined (Q_OS_WIN32)
#include <qdir.h>
#endif
#include <qfile.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qlayout.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>

TodoSettings::TodoSettings(QWidget *parent, const char *name, bool modal, WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setCaption("Configure columns to display");

    QGridLayout *grid = new QGridLayout(this);
    map = new FieldMap(this);
    grid->addWidget(map, 0, 0);
}

void TodoSettings::setCurrentFields(const QValueList<int> &f)
{
    QMap<int,QString> m = PimTask::trFieldsMap();
    m.remove( PimRecord::Categories );
    map->setFields(m, f);
}

TodoWindow::TodoWindow( QWidget *parent, const char *name, WFlags f) :
    QMainWindow( parent, name, f ), tasks(TaskIO::ReadWrite)
{
//     QTime t;
//     t.start();
    
    setCaption( tr("Todo") );
    setBackgroundMode( PaletteButton );

    vb = new QVBox( this );
    
    Config config( "todo" );
    config.setGroup( "View" );
    bool complete = config.readBoolEntry( "ShowComplete", true );
    tasks.setCompletedFilter( !complete );

    tView = 0;
    table = new TodoTable(tasks.sortedTasks(), vb );
    table->setSelectionMode( TodoTable::Extended );
    QWhatsThis::add( table, tr("List of tasks matching the completion and category filters.") );

    setCentralWidget( vb );
    setToolBarsMovable( FALSE );

//     qDebug("after load: t=%d", t.elapsed() );
    
    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mb = new QPEMenuBar( bar );

    QPopupMenu *edit = new QPopupMenu( this );

    bar = new QPEToolBar( this );

    QAction *a = new QAction( tr( "New Task" ), Resource::loadIconSet( "new" ),
			      QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotNew() ) );
    a->setWhatsThis( tr("Create a new task.") );
    a->addTo( bar );
    a->addTo( edit );
    a = new QAction( tr( "Edit" ), Resource::loadIconSet( "edit" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotEdit() ) );
    a->setWhatsThis( tr("Edit the highlighted task.") );
    a->addTo( bar );
    a->addTo( edit );
    a->setEnabled( FALSE );
    editAction = a;
    a = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotDelete() ) );
    a->setWhatsThis( tr("Delete the highlighted task.") );
    a->addTo( bar );
    a->addTo( edit );
    a->setEnabled( FALSE );
    deleteAction = a;

    if ( Ir::supported() ) {
	a = new QAction( tr( "Beam" ), Resource::loadIconSet( "beam" ),
			 QString::null, 0, this, 0 );
	connect( a, SIGNAL( activated() ),
		 this, SLOT( slotBeam() ) );
	a->setWhatsThis( tr("Beam the highlighted task to another device.") );
	a->addTo( edit );
	a->addTo( bar );
	beamAction = a;
    } else {
	beamAction = NULL;
    }

    a = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ),
		     QString::null, 0, this, 0 );
    a->setToggleAction( TRUE );
    a->setWhatsThis( tr("Search for a task.") );
    connect( a, SIGNAL( toggled(bool) ),
	     this, SLOT( slotFind(bool) ) );
    a->addTo( bar );
    a->addTo( edit );
    findAction = a;

    QPopupMenu *view = new QPopupMenu(this);
    view->insertItem( tr("Select All"), this, SLOT( selectAll() ) );
    view->insertSeparator();
    view->insertItem( tr("Configure headers"), this, SLOT( configure() ) );

    mb->insertItem( tr( "Task" ), edit );
    mb->insertItem( tr( "View" ), view );
    
    // Search bar
    searchBar = new QPEToolBar(this);
    addToolBar( searchBar,  "Search", QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL(textChanged(const QString &)),
	    this, SLOT(search()) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    a = new QAction( tr( "Find Next" ), Resource::loadIconSet( "next" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(search()) );
    a->setWhatsThis( tr("Find the next matching task.") );
    a->addTo( searchBar );

    searchBar->hide();

    // Filter bar
    QHBox *hb = new QHBox( vb );

    QComboBox *cb = new QComboBox( hb );
    cb->insertItem( tr("Pending Tasks") );
    cb->insertItem( tr("All Tasks") );
    cb->setCurrentItem( tasks.completedFilter() ? 0 : 1 );
    connect( cb, SIGNAL(activated(int)), this, SLOT(setShowCompleted(int)) );
    QWhatsThis::add( cb, tr("Show tasks with this completion status.") );

    QArray<int> vl( 0 );
    catSelect = new CategorySelect( hb );
    catSelect->setRemoveCategoryEdit( TRUE );
    catSelect->setCategories( vl, "Todo List", tr("Todo List") );
    catSelect->setAllCategories( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(catSelected(int)) );
    QWhatsThis::add( catSelect, tr("Show tasks in this category.") );

    resize( 200, 300 );
    if ( table->hasCurrentEntry() )
        currentEntryChanged( 0, 0 );
    connect( table, SIGNAL( clicked() ),
	    this, SLOT( slotDetailView() ) );

//     qDebug("mainwindow #3: t=%d", t.elapsed() );
    connect( table, SIGNAL(findWrapAround()), this, SLOT(findWrapped()) );
    connect( table, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( table, SIGNAL(findFound()), this, SLOT(findFound()) );

    int currCat = config.readNumEntry( "Category", -2 );
    catSelect->setCurrentCategory( currCat );
    catSelected( currCat );

    connect( table, SIGNAL( updateTask(const PimTask &) ),
	     this, SLOT( updateEntry( const PimTask &) ) );
    connect( table, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( currentEntryChanged( int, int ) ) );

//     qDebug("done: t=%d", t.elapsed() );
    connect(qApp, SIGNAL( appMessage(const QCString &, const QByteArray &) ), 
	    this, SLOT( appMessage(const QCString &, const QByteArray &) ) );
    connect(qApp, SIGNAL(reload()), this, SLOT(reload()));
    connect(qApp, SIGNAL(flush()), this, SLOT(flush()));
}

TodoWindow::~TodoWindow()
{
}

void TodoWindow::addEntry( const PimTask &todo )
{
    tasks.addTask( todo );
    table->reload(tasks.sortedTasks());
}

void TodoWindow::removeEntry(const PimTask &todo )
{
    tasks.removeTask( todo );
    table->reload(tasks.sortedTasks());
}

void TodoWindow::updateEntry(const PimTask &todo )
{
    tasks.updateTask( todo );
    table->reload(tasks.sortedTasks());
}

void TodoWindow::appMessage(const QCString &msg, const QByteArray &data)
{
    bool needShow = FALSE;
    if ( msg == "newTask()" ) {
	slotNew();
    } else if ( msg == "addTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	addEntry(c);
    } else if ( msg == "removeTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	removeEntry(c);
    } else if ( msg == "updateTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	updateEntry(c);
    } else if ( msg == "showTask(QUuid)" ) {
	// If we're in edit mode, we need to stay there (which we currently do)
	QDataStream stream(data,IO_ReadOnly);
	QUuid u;
	stream >> u;

	//
	// Searching for the right category given just the uid may be
	// expensive.  Default to the "All" category for now.
	//
	catSelect->setCurrentCategory(-2);
	catSelected(catSelect->currentCategory());
	table->setCurrentEntry(u);
	needShow = TRUE;
    } else if ( msg == "receiveData(QString,QString)" ) {
	QDataStream stream(data,IO_ReadOnly);
	QString f,t;
	stream >> f >> t;
	if ( t.lower() == "text/x-vcalendar" )
	    if ( receiveFile(f) )
		needShow = TRUE;
	QFile::remove(f);
    } 
    
    if ( needShow ) {
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
//	showMaximized();
#else
//	show();
#endif
//	raise();
	QPEApplication::setKeepRunning();
//	setActiveWindow();
    }
}

void TodoWindow::slotListView()
{
    if ( !table->isVisible() ) {
	todoView()->hide();
	setCentralWidget( vb );
	table->show();
	table->setFocus();
	setCaption( tr("Todo") );
    }
}

void TodoWindow::slotDetailView()
{
    todoView()->init( table->currentEntry() );
    showView();
}

TodoView* TodoWindow::todoView()
{
    if ( !tView )
	tView = new TodoView(this);

    return tView;
}

void TodoWindow::showView()
{
    if ( table->isVisible() ) {
	table->hide();
	setCentralWidget( todoView() );
	tView->show();
	tView->setFocus();  //To avoid events being passed to QTable
	setCaption( tr("Task Details") );
    }
}

void TodoWindow::slotNew()
{
    int id;
    id = -1;
    QArray<int> ids;
    ids = table->currentEntry().categories();
    if ( ids.count() )
	id = ids[0];
    NewTaskDialog e( id, this, 0, TRUE );
    e.setCurrentCategory(catSelect->currentCategory());

    PimTask todo;

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
        todo = e.todoEntry();
        addEntry( todo );
	findAction->setEnabled( TRUE );
        currentEntryChanged( 0, 0 );
	slotListView();
    }
}

void TodoWindow::slotDelete()
{
    if ( table->currentRow() == -1 )
        return;

    QString strName = table->currentEntry().description().left(30);

    if ( table->selectionMode() == TodoTable::Extended ) {
	QValueList<QUuid> t = table->selectedTasks();
	
	if ( !t.count() ) return;
	
	QString str;
	if ( t.count() > 1 )
	    str = QString("Are you sure you want to delete the %1 selected tasks?").arg( t.count() );
	else
	    str = QString("Are you sure you want to delete:\n%1?").arg( table->currentEntry().description().left(30)  );

	switch( QMessageBox::warning( this, tr("Todo"), tr(str), tr("Yes"), tr("No"), 0, 0, 1 ) ) {
	    case 0:
	    {
		deleteTasks(t);
		currentEntryChanged(0, 0);
		slotListView();
	    }
	    break;
	    case 1: break;
	}
    } else if ( QPEMessageBox::confirmDelete( this, tr( "Todo" ),
	strName.simplifyWhiteSpace() ) ) {
	return;
	
	removeEntry( table->currentEntry() );
	currentEntryChanged(0, 0);
	slotListView();
    }
}

void TodoWindow::deleteTasks(const QValueList<QUuid> &t)
{
    for (QValueList<QUuid>::ConstIterator it = t.begin(); it != t.end(); ++it) {
	PrTask t;
	t.setUid( *it );
	tasks.removeTask(t);
    }
    
    table->reload( tasks.sortedTasks() );
}

void TodoWindow::slotEdit()
{
    PimTask todo = table->currentEntry();

    NewTaskDialog e( todo, this, 0, TRUE );
    e.setCaption( tr( "Edit Task" ) );

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
        todo = e.todoEntry();
	updateEntry( todo );
	slotListView();
    }
    
}

void TodoWindow::setShowCompleted( int s )
{
    if ( !table->isUpdatesEnabled() )
	return;
    tasks.setCompletedFilter( s != 1 );
    table->reload(tasks.sortedTasks());
}

void TodoWindow::currentEntryChanged( int , int )
{
    bool entrySelected = table->hasCurrentEntry();

    editAction->setEnabled(entrySelected);
    deleteAction->setEnabled(entrySelected);
    findAction->setEnabled(entrySelected);

    if (beamAction) {
	beamAction->setEnabled(entrySelected);
    }
}

void TodoWindow::reload()
{
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Todo List", tr("Todo List") );
    catSelect->setAllCategories( TRUE );
    
    tasks.ensureDataCurrent(TRUE);
    catSelected( catSelect->currentCategory() );
}

void TodoWindow::flush()
{
    tasks.saveData();
}

void TodoWindow::catSelected( int c )
{
    tasks.setFilter( c );
    setCaption( tr("Todo") + " - " + table->categoryLabel( c ) );
    table->reload(tasks.sortedTasks());
    currentEntryChanged(0, 0);
}

void TodoWindow::closeEvent( QCloseEvent *e )
{
    if ( !table->isVisible() ) {
	slotListView();
	e->ignore();
    } else {
	e->accept();
	// repeat for categories...
	// if writing configs fail, it will emit an
	// error, but I feel that it is "ok" for us to exit
	// espically since we aren't told if the write succeeded...
	Config config( "todo" );
	config.setGroup( "View" );
	config.writeEntry( "ShowComplete", !tasks.completedFilter() );
	config.writeEntry( "Category", tasks.filter() );
    }
}

void TodoWindow::slotFind( bool s )
{
    slotListView();
    if ( s ) {
	searchBar->show();
	searchEdit->setFocus();
    } else {
	searchBar->hide();
	if ( table->numSelections() )
	    table->clearSelection();
	table->clearFindRow();
    }
}

void TodoWindow::search()
{
    table->slotDoFind( searchEdit->text(), tasks.filter() );
}

void TodoWindow::findNotFound()
{
    Global::statusMessage( tr("Find: not found") );
}

void TodoWindow::findFound()
{
    Global::statusMessage( "" );
}

void TodoWindow::findWrapped()
{
    Global::statusMessage( tr("Find: reached end") );
}

void TodoWindow::setDocument( const QString &filename )
{
    DocLnk doc(filename);
    if ( doc.isValid() )
	receiveFile(doc.file());
    else
	receiveFile(filename);
}

bool TodoWindow::receiveFile( const QString &filename )
{
    QValueList<PimTask> tl = PimTask::readVCalendar( filename );

    QString msg = tr("<P>%1 new tasks.<p>Do you want to add them to your Todo List?").
	arg(tl.count());

    if ( QMessageBox::information(this, tr("New Tasks"),
	    msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
	for( QValueList<PimTask>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
	    addEntry( *it );
	}
	return TRUE;
    }
    return FALSE;
}

static const char * beamfile = "/tmp/obex/todo.vcs";

void TodoWindow::slotBeam()
{
    if ( !table->hasCurrentEntry() ) {
	qWarning("todo::slotBeam called with nothing to beam");
	return;
    }
    
    QString description;

    unlink( beamfile ); // delete if exists
    PimTask c = table->currentEntry();
#ifndef Q_OS_WIN32
    mkdir("/tmp/obex/", 0755);
#else
    QDir d;
    d.mkdir("/tmp/obex");
#endif
    if ( table->selectionMode() == TodoTable::Extended ) {
	QValueList<PimTask> l = table->selected();
	PimTask::writeVCalendar( beamfile, l );
	
	if ( l.count() > 1 )
	    description = QString("the %1 selected tasks").arg( l.count() );
	else
	    description = c.description();
    } else {
	PimTask::writeVCalendar( beamfile, c );
	description = c.description();
    }
    
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
    ir->send( beamfile, description, "text/x-vCalendar" );
}

void TodoWindow::beamDone( Ir *ir )
{
    delete ir;
    unlink( beamfile );
}

void TodoWindow::configure()
{
    TodoSettings settings(this, "", TRUE);
    settings.setCurrentFields( table->fields() );
    if ( QPEApplication::execDialog(&settings) == QDialog::Accepted ) {
	table->setFields( settings.fields() );
	table->reload( tasks.sortedTasks() );
    }
}

void TodoWindow::selectAll()
{
    table->selectAll();
}

