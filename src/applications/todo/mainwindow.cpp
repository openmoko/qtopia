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

#include <qpe/applnk.h>
#include <qpe/qpeapplication.h>
#include <qpe/config.h>
#include <qpe/finddialog.h>
#include <qpe/global.h>
#include <qpe/ir.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpemessagebox.h>
#include <qpe/resource.h>
#include <qpe/pim/task.h>
#include <qpe/qpetoolbar.h>
#include <qpe/categoryselect.h>
#include <qpe/categories.h>

#include <qaction.h>
#include <qarray.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qwhatsthis.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>

TodoWindow::TodoWindow( QWidget *parent, const char *name, WFlags f = 0 ) :
    QMainWindow( parent, name, f )
{
//     QTime t;
//     t.start();
    
    setCaption( tr("Todo") );
    setBackgroundMode( PaletteButton );

    QVBox *vb = new QVBox( this );
    
    QString str;
    table = new TodoTable( vb );
    table->setColumnWidth( 2, 10 );
    QWhatsThis::add( table, tr("List of tasks matching the completion and category filters.") );

    setCentralWidget( vb );
    setToolBarsMovable( FALSE );

//     qDebug("after load: t=%d", t.elapsed() );
    
    Config config( "todo" );
    config.setGroup( "View" );
    bool complete = config.readBoolEntry( "ShowComplete", true );
    table->setShowCompleted( complete );

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mb = new QPEMenuBar( bar );

    QPopupMenu *edit = new QPopupMenu( this );
    contextMenu = new QPopupMenu( this );

    bar = new QPEToolBar( this );

    QAction *a = new QAction( tr( "New Task" ), Resource::loadPixmap( "new" ),
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
    a->addTo( contextMenu );
    a->setEnabled( FALSE );
    editAction = a;
    a = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotDelete() ) );
    a->setWhatsThis( tr("Delete the highlighted task.") );
    a->addTo( bar );
    a->addTo( edit );
    a->addTo( contextMenu );
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

    mb->insertItem( tr( "Task" ), edit );

    // Search bar
    searchBar = new QPEToolBar(this);
    addToolBar( searchBar,  "Search", QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL(textChanged(const QString &)),
	    this, SLOT(search()) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    a = new QAction( tr( "Find Next" ), Resource::loadPixmap( "next" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(search()) );
    a->setWhatsThis( tr("Find the next matching task.") );
    a->addTo( searchBar );

    searchBar->hide();

    // Filter bar
    QHBox *hb = new QHBox( vb );

    QComboBox *cb = new QComboBox( hb );
    cb->insertItem( tr("Pending Tasks") );
    cb->insertItem( tr("All Tasks") );
    cb->setCurrentItem( table->showCompleted() ? 1 : 0 );
    connect( cb, SIGNAL(activated(int)), this, SLOT(setShowCompleted(int)) );
    QWhatsThis::add( cb, tr("Show tasks with this completion status.") );

    Categories c;
    c.load(categoryFileName());
    QArray<int> vl( 0 );
    catSelect = new CategorySelect( hb );
    catSelect->setRemoveCategoryEdit( TRUE );
    catSelect->setCategories( vl, "Todo List", tr("Todo List") );
    catSelect->setAllCategories( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(catSelected(int)) );
    QWhatsThis::add( catSelect, tr("Show tasks in this category.") );

    resize( 200, 300 );
    if ( table->numRows() > 0 )
        currentEntryChanged( 0, 0 );
    connect( table, SIGNAL( signalEdit() ),
             this, SLOT( slotEdit() ) );
    connect( table, SIGNAL(signalShowMenu(const QPoint &)),
	     this, SLOT( slotShowPopup(const QPoint &)) );

//     qDebug("mainwindow #3: t=%d", t.elapsed() );
    connect( table, SIGNAL(findWrapAround()), this, SLOT(findWrapped()) );
    connect( table, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( table, SIGNAL(findFound()), this, SLOT(findFound()) );

    int currCat = config.readNumEntry( "Category", -2 );
    catSelect->setCurrentCategory( currCat );
    catSelected( currCat );

    connect( table, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( currentEntryChanged( int, int ) ) );

//     qDebug("done: t=%d", t.elapsed() );
}

TodoWindow::~TodoWindow()
{
}

void TodoWindow::appMessage(const QCString &msg, const QByteArray &data)
{
    bool needShow = FALSE;
    if ( msg == "addTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->addEntry(c);
    } else if ( msg == "removeTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->removeEntry(c);
    } else if ( msg == "updateTask(PimTask)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimTask c;
	stream >> c;
	table->updateEntry(c);
    } else if ( msg == "showTask(QUuid)" ) {
	// If we're in edit mode, we need to stay there (which we currently do)
	QDataStream stream(data,IO_ReadOnly);
	QUuid u;
	stream >> u;
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

void TodoWindow::slotNew()
{
    int id;
    id = -1;
    QArray<int> ids;
    ids = table->currentEntry().categories();
    if ( ids.count() )
	id = ids[0];
    NewTaskDialog e( id, this, 0, TRUE );

    PimTask todo;

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
        todo = e.todoEntry();
        table->addEntry( todo );
	findAction->setEnabled( TRUE );
        currentEntryChanged( 0, 0 );
    }
}

void TodoWindow::slotDelete()
{
    if ( table->currentRow() == -1 )
        return;

    QString strName = table->currentEntry().description().left(30);

    if ( !QPEMessageBox::confirmDelete( this, tr( "Todo" ),
	strName.simplifyWhiteSpace() ) ) {
	return;
    }

    table->removeCurrentEntry();

    currentEntryChanged(0, 0);
}

void TodoWindow::slotEdit()
{
    PimTask todo = table->currentEntry();

    NewTaskDialog e( todo, this, 0, TRUE );
    e.setCaption( tr( "Edit Task" ) );

    int ret = QPEApplication::execDialog(&e);

    if ( ret == QDialog::Accepted ) {
        todo = e.todoEntry();
	table->updateEntry( todo );
    }

}

void TodoWindow::slotShowPopup( const QPoint &p )
{
    contextMenu->popup( p );
}

void TodoWindow::setShowCompleted( int s )
{
    if ( !table->isUpdatesEnabled() )
	return;
    table->setShowCompleted( s == 1 );
}

void TodoWindow::currentEntryChanged( int r, int )
{
    bool    entrySelected = table->numRows() > 0;

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
    table->reload();
}

void TodoWindow::flush()
{
    table->flush();
}

void TodoWindow::catSelected( int c )
{
    table->setShowCategory( c );
    setCaption( tr("Todo") + " - " + table->categoryLabel( c ) );
    currentEntryChanged(0, 0);
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
    config.writeEntry( "ShowComplete", table->showCompleted() );
    config.writeEntry( "Category", table->showCategory() );
}

void TodoWindow::slotFind( bool s )
{
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
    table->slotDoFind( searchEdit->text(), table->showCategory() );
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
	    table->addEntry( *it );
	}
	return TRUE;
    }
    return FALSE;
}

static const char * beamfile = "/tmp/obex/todo.vcs";

void TodoWindow::slotBeam()
{
    unlink( beamfile ); // delete if exists
    PimTask c = table->currentEntry();
    mkdir("/tmp/obex/", 0755);
    PimTask::writeVCalendar( beamfile, c );
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
    QString description = c.description();
    ir->send( beamfile, description, "text/x-vCalendar" );
}

void TodoWindow::beamDone( Ir *ir )
{
    delete ir;
    unlink( beamfile );
}
