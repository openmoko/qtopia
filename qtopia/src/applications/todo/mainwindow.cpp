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

#define QTOPIA_INTERNAL_FD
#define QTOPIA_INTERNAL_FILEOPERATIONS

#include "mainwindow.h"
#include "todoentryimpl.h"
#include "todotable.h"

#include <qtopia/applnk.h>
#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif
#include <qtopia/qpeapplication.h>
#include <qtopia/qcopenvelope_qws.h>
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
#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
# include <qtopia/categorydialog.h>
#endif

#include <qaction.h>
#include <qarray.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qlabel.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <unistd.h>
#endif

#include <stdlib.h>

static bool constructorDone = FALSE;

TodoSettings::TodoSettings(QWidget *parent, const char *name, bool modal, WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setCaption(tr("Configure columns to display"));

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

//===========================================================================

TodoView::TodoView( QWidget *parent, const char *name)
    : QTextBrowser(parent, name) 
{ 
    setFrameStyle(NoFrame); 
#ifdef QTOPIA_DATA_LINKING
     mNotesQC = new QDLClient( this, "qdlNotes" );
#endif
}
void TodoView::init( const PimTask &task )
{
    QString txt = task.toRichText();
#ifdef QTOPIA_DATA_LINKING
    QDL::loadLinks( task.customField( QDL::DATA_KEY ), QDL::clients( this ) );
    txt = QDL::lidsToAnchors( txt, mNotesQC );
#endif
    setText(txt);
}

void TodoView::setSource( const QString &name )
{
#ifdef QTOPIA_DATA_LINKING
    QDL::activateLink( name, QDL::clients( this ) );
#else
    QTextBrowser::setSource( name );
#endif
}

//===========================================================================

TodoWindow::TodoWindow( QWidget *parent, const char *name, WFlags f) :
    QMainWindow( parent, name, f )
{
#ifdef Q_WS_QWS
    beamfile = Global::tempDir() + "obex";
    QDir d;
    d.mkdir(beamfile);
    beamfile += "/todo.vcs";
#endif

    setCaption( tr("Tasks") );
    setBackgroundMode( PaletteButton );

    tView = 0;

    listView = new QWidget(this);

    table = new TodoTable(listView );
#ifdef QTOPIA_PHONE
    table->setMargin(0);
    table->setSelectionMode( TodoTable::NoSelection );
#else
    table->setSelectionMode( TodoTable::Extended );
#endif
    QWhatsThis::add( table, tr("List of tasks matching the completion and category filters.") );
    
    setCentralWidget( listView );

    createUI();
    
    if ( table->hasCurrentEntry() )
        currentEntryChanged( );
    setCentralWidget( listView );
    
    connect( table, SIGNAL( clicked() ),
	    this, SLOT( showDetailView() ) );

    // Wah? why this strange turn-around
    connect( table, SIGNAL( updateTask(const PimTask&) ),
	     table, SLOT( updateEntry(const PimTask&) ) );
    connect( table, SIGNAL( currentChanged() ),
             this, SLOT( currentEntryChanged() ) );

    connect(qApp, SIGNAL( appMessage(const QCString&,const QByteArray&) ), 
	    this, SLOT( appMessage(const QCString&,const QByteArray&) ) );
    connect(qApp, SIGNAL(reload()), this, SLOT(reload()));
    connect(qApp, SIGNAL(flush()), this, SLOT(flush()));
    connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(catChanged()) );
    
    constructorDone = TRUE;

    
    // This call is to ensure that all records contains only valid categories 
    catChanged();
}

TodoWindow::~TodoWindow()
{
    table->saveData();
}

void TodoWindow::createUI()
{
    QGridLayout *grid = new QGridLayout(listView);
    grid->addMultiCellWidget( table, 0, 0, 0, 1);

    newAction = new QAction( tr( "New" ), Resource::loadIconSet( "new" ),
			      QString::null, 0, this, 0 );
    connect( newAction, SIGNAL( activated() ),
             this, SLOT( createNewEntry() ) );
    newAction->setWhatsThis( tr("Create a new task.") );

    editAction = new QAction( tr( "Edit" ), Resource::loadIconSet( "edit" ),
		     QString::null, 0, this, 0 );
    connect( editAction, SIGNAL( activated() ),
             this, SLOT( editCurrentEntry() ) );
    editAction->setWhatsThis( tr("Edit the highlighted task.") );
    editAction->setEnabled( FALSE );

    deleteAction = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ),
		     QString::null, 0, this, 0 );
    connect( deleteAction, SIGNAL( activated() ),
             this, SLOT( deleteCurrentEntry() ) );
    deleteAction->setWhatsThis( tr("Delete the highlighted task.") );
    deleteAction->setEnabled( FALSE );

    findAction = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ),
		     QString::null, 0, this, 0 );
    findAction->setToggleAction( TRUE );
    findAction->setWhatsThis( tr("Search for a task.") );
    connect( findAction, SIGNAL( toggled(bool) ),
	     this, SLOT( showFindWidget(bool) ) );

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    if ( Ir::supported() ) {
	beamAction = new QAction( tr( "Beam" ), Resource::loadIconSet( "beam" ),
			 QString::null, 0, this, 0 );
	connect( beamAction, SIGNAL( activated() ),
		 this, SLOT( beamCurrentEntry() ) );
	beamAction->setWhatsThis( tr("Beam the highlighted task to another device.") );
	beamAction->setEnabled(FALSE);
    } else {
	beamAction = 0;
    }
#endif

    backAction = new QAction( tr("Back"), 
	    Resource::loadIconSet("addressbook/back"),
	    QString::null, 0, this, 0);
    backAction->setEnabled(FALSE);
    connect( backAction, SIGNAL(activated()), this, SLOT(showListView()) );

#ifndef QTOPIA_PHONE
    setToolBarsMovable( FALSE );

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mb = new QPEMenuBar( bar );

    QPopupMenu *edit = new QPopupMenu( this );

    bar = new QPEToolBar( this );

    newAction->addTo( bar );
    newAction->addTo( edit );
    editAction->addTo( bar );
    editAction->addTo( edit );
    deleteAction->addTo( bar );
    deleteAction->addTo( edit );
    findAction->addTo( bar );
    findAction->addTo( edit );
    if (beamAction)
	beamAction->addTo( edit );
    bar->addSeparator();

    QPopupMenu *view = new QPopupMenu(this);
    backAction->addTo( bar );

    view->insertSeparator();
    view->insertItem( tr("Select All"), this, SLOT( selectAll() ) );
    view->insertSeparator();
    view->insertItem( tr("Configure columns"), this, SLOT( configure() ) );

    mb->insertItem( tr( "Task" ), edit );
    mb->insertItem( tr( "View" ), view );
#else
    contextMenu = new ContextMenu(this);
    newAction->addTo(contextMenu);
    editAction->addTo(contextMenu);
    deleteAction->addTo(contextMenu);
    if (beamAction)
	beamAction->addTo(contextMenu);

    actionCategory = new QAction(tr("View Category..."), Resource::loadIconSet("viewcategory"), QString::null,
	    0, this, 0 );
    connect( actionCategory, SIGNAL(activated()), this, SLOT(selectCategory()));
    actionCategory->addTo(contextMenu);

    categoryLbl = new QLabel(listView);
    categoryLbl->hide();
    grid->addMultiCellWidget(categoryLbl, 1, 1, 0, 1);

    categoryDlg = 0;
#endif

#ifndef QTOPIA_PHONE
    // Search bar
    searchBar = new QPEToolBar(this);
    addToolBar( searchBar,  tr("Search"), QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL(textChanged(const QString&)),
	    this, SLOT(search()) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    QAction *a = new QAction( tr( "Find Next" ), Resource::loadIconSet( "next" ), QString::null, 0, this, 0 );
    a->setWhatsThis( tr("Find the next matching task.") );
    a->addTo( searchBar );

    searchBar->hide();

    connect( a, SIGNAL(activated()), table, SLOT(findNext()) );
    // can create this item now
    view->insertItem( tr("Fit to width"), table, SLOT(fitHeadersToWidth()) );

    Config config( "todo" );
    config.setGroup( "View" );
    int compFilt = config.readNumEntry( "ShowComplete", 1 );
    setShowCompleted( compFilt );

    // Filter bar
    QComboBox *cb = new QComboBox( listView );
    cb->insertItem( tr("Pending Tasks") );
    cb->insertItem( tr("All Tasks") );
    cb->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred) );
    cb->setCurrentItem( table->completedFilter() ? 0 : 1 );
    connect( cb, SIGNAL(activated(int)), this, SLOT(setShowCompleted(int)) );
    QWhatsThis::add( cb, tr("Show tasks with this completion status.") );

    QArray<int> vl( 0 );
    catSelect = new CategorySelect( listView );
    catSelect->setRemoveCategoryEdit( TRUE );
    catSelect->setCategories( vl, "Todo List", // No tr
	    tr("Tasks") );
    catSelect->setAllCategories( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(catSelected(int)) );
    QWhatsThis::add( catSelect, tr("Show tasks in this category.") );

    grid->addWidget( cb, 1, 0);
    grid->addWidget( catSelect, 1, 1);

//     qDebug("mainwindow #3: t=%d", t.elapsed() );
    connect( table, SIGNAL(findWrapAround()), this, SLOT(findWrapped()) );
    connect( table, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( table, SIGNAL(findFound()), this, SLOT(findFound()) );

    int currCat = config.readNumEntry( "Category", -2 );
    catSelect->setCurrentCategory( currCat );
    table->setFilter( currCat );
    setCaption( tr("Tasks") + " - " + table->categoryLabel( currCat ) );
#else
    setCaption(tr("Tasks"));
#endif
}

void TodoWindow::appMessage(const QCString &msg, const QByteArray &data)
{
    /*@ \service Tasks */
    bool needShow = FALSE;
    if ( msg == "newTask()" ) {
        /*@ \message
            Create a new task
        */
	createNewEntry();
    } else if ( msg == "addTask(PimTask)" ) {
        /*@ \message
            \arguments task Add task, \e task
        */
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->addEntry(c);
    } else if ( msg == "removeTask(PimTask)" ) {
        /*@ \message
            \arguments task Remove task, \e task
        */
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->removeEntry(c);
    } else if ( msg == "updateTask(PimTask)" ) {
        /*@ \message
            \arguments task Update task, \e task
        */
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->updateEntry(c);
	if ( centralWidget() == todoView() ) {
	    todoView()->init( table->currentEntry() );
	    tView->setFocus();  //To avoid events being passed to QTable
	}
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
	catSelected(-2);
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
#ifdef QTOPIA_DATA_LINKING
    } else if( msg == "QDLRequestLink(QString,QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
        QString	clientID, hint;
	stream >> clientID >> hint;
	QDLHeartBeat hb( clientID );

	QDialog *diag = new QDialog(this, 0, TRUE);
	diag->setCaption( tr("Tasks") );
	QHBoxLayout *hl = new QHBoxLayout(diag);
	TodoTable *tmpTable = new TodoTable(diag, 0, 0, TRUE);
	connect(tmpTable, SIGNAL(clicked()), diag, SLOT(accept()));
	hl->addWidget(tmpTable);
#ifndef QTOPIA_DESKTOP
	if( QPEApplication::execDialog( diag ) == QDialog::Accepted )
#else
	if( diag->exec() == QDialog::Accepted )
#endif
	{
	    QCopEnvelope e( QDL::CLIENT_CHANNEL, "QDLProvideLink(QString,int,...)" );
	    e << clientID;
	    e << 1;
	    QByteArray dataRef;
	    QDataStream refStream( dataRef, IO_WriteOnly );
	    refStream << tmpTable->currentEntry().uid().toString();
	    QDLLink newLink("todolist",  dataRef, tmpTable->currentEntry().description(),  "todolist/TodoList");
	    e << newLink;

	}
	delete diag;
    } else if( msg == "QDLActivateLink(QByteArray)" ) {
	needShow = TRUE;
	QDataStream stream( data, IO_ReadOnly );
	QByteArray dataRef;
	stream >> dataRef;
	QDataStream refStream( dataRef, IO_ReadOnly );
	QString dataRefStr;
	refStream >> dataRefStr;
	QUuid uid( dataRefStr  );
	table->setCurrentEntry( uid );
	showListView();
#endif
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

void TodoWindow::showListView()
{
    if ( centralWidget() != listView ) {
	backAction->setEnabled(FALSE);
	todoView()->hide();
	setCentralWidget( listView );
	listView->show();
	table->setFocus();
	setCaption( tr("Tasks") );
#ifdef QTOPIA_PHONE
	newAction->setEnabled(TRUE);
	actionCategory->setEnabled(TRUE);
	editAction->setEnabled(FALSE);
	deleteAction->setEnabled(FALSE);
	if (beamAction) beamAction->setEnabled(FALSE);
#endif
    }
}

void TodoWindow::showDetailView()
{
    if (centralWidget() != todoView()) {
	backAction->setEnabled(TRUE);
	todoView()->init( table->currentEntry() );
	listView->hide();
	setCentralWidget( todoView() );
	todoView()->show();
	todoView()->setFocus();  //To avoid events being passed to QTable
	setCaption( tr("Task Details") );
#ifdef QTOPIA_PHONE
	newAction->setEnabled(FALSE);
	actionCategory->setEnabled(FALSE);
	editAction->setEnabled(TRUE);
	deleteAction->setEnabled(TRUE);
	if (beamAction) beamAction->setEnabled(TRUE);
#endif
    }
}

TodoView* TodoWindow::todoView()
{
    if ( !tView ) {
	tView = new TodoView(this);
	tView->setMargin(0);
	connect( tView, SIGNAL(done()), this, SLOT(showListView()) );
	connect( tView, SIGNAL(previous()), this, SLOT(viewPrevious()) );
	connect( tView, SIGNAL(next()), this, SLOT(viewNext()) );
#ifdef QTOPIA_PHONE
	ContextBar::setLabel(tView, Key_Select, ContextBar::NoLabel);
#endif
    }

    return tView;
}

void TodoWindow::createNewEntry()
{
#ifdef QTOPIA_PHONE
    TaskDialog e( -2, this, "edit-screen", TRUE );
#else
    TaskDialog e( catSelect->currentCategory(), this, "edit-screen", TRUE );
#endif
    e.setCaption( tr( "New Task" ) );

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
	PimTask todo = e.todoEntry();
	if ( todo.description().isEmpty() && !todo.notes().isEmpty() ) {
	    // Don't want to loose an entry that has some useful data
	    QString desc = todo.notes();
#ifdef QTOPIA_DATA_LINKING
	    // filter out any qdl fields
	    desc = QDL::removeLids(desc);
#endif
	    if ( desc.length() > 30 ) {
		// keep description length sensible
		desc.truncate(27);
		desc.append( "..." ); // no tr
	    }
	    todo.setDescription( desc );
	}
	if ( !todo.description().isEmpty() ) {
	    table->addEntry( todo );
	    findAction->setEnabled( TRUE );
	    currentEntryChanged( );
	    showListView();
	}
    }
}

void TodoWindow::deleteCurrentEntry()
{
    if ( table->currentRow() == -1 )
        return;

    QString strName = table->currentEntry().description().left(30);

    if ( table->selectionMode() == TodoTable::Extended ) {
	QValueList<int> t = table->selectedTasks();
	
	if ( !t.count() ) return;
	
	QString str;
	if ( t.count() > 1 )
	    str = tr("<qt>Are you sure you want to delete the %1 selected tasks?</qt>").arg( t.count() );
	else
	    str = tr("<qt>Are you sure you want to delete:<p><b>%1</b>?</qt>").arg( table->currentEntry().description().left(30)  );

	switch( QMessageBox::warning( this, tr("Tasks"), str, tr("Yes"), tr("No"), 0, 0, 1 ) ) {
	    case 0:
	    {
		table->removeList(t);
		currentEntryChanged();
		showListView();
	    }
	    break;
	    case 1: break;
	}
    } else if ( QPEMessageBox::confirmDelete( this, tr( "Tasks" ),
	strName.simplifyWhiteSpace() ) ) {
	table->removeEntry( table->currentEntry() );
	currentEntryChanged();
	showListView();
    }
    if ( !table->numRows() && findAction->isOn()) {
	findAction->setOn(FALSE);
    }
}

void TodoWindow::editCurrentEntry()
{
    PimTask todo = table->currentEntry();

    TaskDialog e( todo, this, "edit-screen", TRUE );
    e.setCaption( tr( "Edit Task" ) );

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
        todo = e.todoEntry();
	table->updateEntry( todo );
	if (table->currentEntry().uid() != todo.uid())
	    showListView();
	if ( centralWidget() == todoView() ) {
	    todoView()->init( table->currentEntry() );
	    tView->setFocus();  //To avoid events being passed to QTable
	}
    }
    
}

void TodoWindow::viewPrevious()
{
    int cr = table->currentRow();
    if ( --cr >= 0 ) {
	table->setCurrentCell( cr, table->currentColumn() );
	todoView()->init( table->currentEntry() );
    }
}

void TodoWindow::viewNext()
{
    int cr = table->currentRow();
    if ( ++cr < table->numRows() ) {
	table->setCurrentCell( cr, table->currentColumn() );
	todoView()->init( table->currentEntry() );
    }
}

void TodoWindow::setShowCompleted( int s )
{
    if ( !table->isUpdatesEnabled() )
	return;
    table->setCompletedFilter( s != 1 );
    table->reload();
}

void TodoWindow::currentEntryChanged( )
{
    bool entrySelected = table->hasCurrentEntry();

#ifndef QTOPIA_PHONE
    editAction->setEnabled(entrySelected);
    deleteAction->setEnabled(entrySelected);
    if (beamAction) beamAction->setEnabled(entrySelected);
#endif
    findAction->setEnabled(entrySelected);
}

void TodoWindow::reload()
{
    table->loadData();
#ifndef QTOPIA_PHONE
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Todo List", // No tr
	tr("Tasks") );
    catSelect->setAllCategories( TRUE );
    
    catSelected( catSelect->currentCategory() );
#else
    catSelected( -2 );
#endif
    table->reload();
    if ( centralWidget() == tView )
	showDetailView();
}

void TodoWindow::flush()
{
    table->saveData();
}

void TodoWindow::catSelected( int c )
{
    table->setFilter( c ); // WAH? Why?
    table->reload(); // WAH? Why?
#ifndef QTOPIA_PHONE
    setCaption( tr("Tasks") + " - " + table->categoryLabel( c ) );
#else
    if (c == -2) {
	categoryLbl->hide();
    } else {
	categoryLbl->setText(tr("Category: %1").arg(table->categoryLabel(c)));
	categoryLbl->show();
    }
#endif
}

// Loop through and validate the categories.  If any records' category list was
// modified we need to update
void TodoWindow::catChanged()
{
    Categories c;
    c.load( categoryFileName() );
    QArray<int> cats = c.ids("Todo List", c.labels("Todo List", TRUE) ); // No tr
    if (table->categoriesChanged(cats))
	reload();
}

void TodoWindow::closeEvent( QCloseEvent *e )
{
    e->accept();
    // repeat for categories...
    // if writing configs fail, it will emit an
    // error, but I feel that it is "ok" for us to exit
    // espically since we aren't told if the write succeeded...
    Config config( "todo" );
    config.setGroup( "View" );
    config.writeEntry( "ShowComplete", !table->completedFilter() );
    config.writeEntry( "Category", table->filter() );
}

#ifdef QTOPIA_PHONE
void TodoWindow::keyPressEvent(QKeyEvent *e)
{
    QMainWindow::keyPressEvent(e);
    if ( !e->isAccepted() && (e->key() == Key_Back || e->key() == Key_No)) {
	e->accept();
	if ( centralWidget() == tView )
	    showListView();
	else
	    close();
    }
}
#endif

void TodoWindow::showFindWidget( bool s )
{
    showListView();
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
    table->find( searchEdit->text() );
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

    if (tl.count() < 1) {
	// should spit out more appropriate message.
	QMessageBox::information(this, tr("New Tasks"),
		    tr("<p>Received empty task list.  No tasks added"),
		    QMessageBox::Ok);
	return FALSE;
    }
    QString msg = tr("<P>%1 new tasks.<p>Do you want to add them to your Tasks?").
	arg(tl.count());

    if ( QMessageBox::information(this, tr("New Tasks"),
	    msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
	for( QValueList<PimTask>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
	    table->addEntry( *it );
	}
	return TRUE;
    }
    return FALSE;
}


void TodoWindow::beamCurrentEntry()
{
    if ( !table->hasCurrentEntry() ) {
	qWarning("todo::beamCurrentEntry called with nothing to beam");
	return;
    }
    
    QString description;

    ::unlink( beamfile.local8Bit().data() ); // delete if exists
    PimTask c = table->currentEntry();
    if ( table->selectionMode() == TodoTable::Extended ) {
	QValueList<PimTask> l = table->selected();
	PimTask::writeVCalendar( beamfile, l );
	
	if ( l.count() > 1 )
	    description = tr("the %1 selected tasks").arg( l.count() );
	else
	    description = c.description();
    } else {
	PimTask::writeVCalendar( beamfile, c );
	description = c.description();
    }
    
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done(Ir*) ), this, SLOT( beamDone(Ir*) ) );
    ir->send( beamfile, description, "text/x-vCalendar" );
#endif
}

void TodoWindow::beamDone( Ir *ir )
{
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    delete ir;
    ::unlink( beamfile.local8Bit().data() );
#endif
}

void TodoWindow::configure()
{
    TodoSettings settings(this, "", TRUE);
    settings.setCurrentFields( table->fields() );
    if ( QPEApplication::execDialog(&settings) == QDialog::Accepted ) {
	table->setFields( settings.fields() );
	table->reload(); // WAH? Why?
    }
}

void TodoWindow::selectAll()
{
    table->selectAll();
}

void TodoWindow::selectCategory()
{
#ifdef QTOPIA_PHONE
    if (!categoryDlg) {
	categoryDlg = new CategorySelectDialog("Todo List", this, 0, TRUE);
	categoryDlg->setAllCategories(TRUE);
	connect(categoryDlg, SIGNAL(selected(int)), this, SLOT(catSelected(int)));
    }

    QPEApplication::execDialog(categoryDlg);
#endif
}

