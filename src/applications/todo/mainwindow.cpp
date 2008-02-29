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

#include "mainwindow.h"
#include "todoentryimpl.h"
#include "todotable.h"
#include "todocategoryselector.h"

#include <qcontent.h>
#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>

#include <qtopiasendvia.h>
#include <qtopianamespace.h>
#include <qtopia/pim/qtask.h>
#include <qtopia/pim/qtaskview.h>
#include <QCategoryFilter>
#include <qcategoryselector.h>
#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

#include <QDL>
#include <QDLBrowserClient>
#include <QDSActionRequest>
#include <QDSData>

#include <QAction>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QComboBox>
#include <QLineEdit>
#include <QWhatsThis>
#include <QLayout>
#include <QLabel>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#include <stdlib.h>

#include <QToolBar>
#include <QMenuBar>
#include <QCloseEvent>
#include <QStackedWidget>

static bool constructorDone = false;
//===========================================================================

TodoView::TodoView( QWidget *parent)
:   QDLBrowserClient( parent, "qdlnotes" )
{
    setFrameStyle(NoFrame);
}

void TodoView::init( const QTask &task )
{
    mTask = task;
    QString txt = task.toRichText();
    loadLinks( task.customField( QDL::CLIENT_DATA_KEY ) );
    setHtml(txt);
    verifyLinks();

    // setEditFocus so that TodoView::keyPressEvent() will be called when
    // viewing a sequence of linked tasks
    setEditFocus( true );
}

void TodoView::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
#ifdef QTOPIA_PHONE
        case Qt::Key_Back:
            emit done();
            return;
#endif
        default:
            QDLBrowserClient::keyPressEvent( e );
            break;
    }
}

//===========================================================================

TodoWindow::TodoWindow( QWidget *parent, Qt::WFlags f) :
    QMainWindow( parent, f ),
    prevTasks()
{
    QtopiaApplication::loadTranslations("libqtopiapim");
    beamfile = Qtopia::tempDir() + "obex";
    QDir d;
    d.mkdir(beamfile);
    beamfile += "/todo.vcs";

    setWindowTitle( tr("Tasks") );

    tView = 0;

    centralView = new QStackedWidget;
    listView = new QWidget;
    listView->setFocusPolicy(Qt::NoFocus);
    centralView->addWidget(listView);

    model = new QTaskModel(this);
    table = new TodoTable;
    table->setModel(model);
    closeAfterDetailView = false;
#ifdef QTOPIA_PHONE
   // table->setContentsMargins(0,0,0,0);
    table->installEventFilter(this);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
#endif
    table->setWhatsThis( tr("List of tasks matching the completion and category filters.") );

    setCentralWidget( centralView );

    createUI();

    if ( table->currentIndex().isValid() )
        currentEntryChanged( );

    connect( table, SIGNAL( taskActivated(const QTask &) ),
            this, SLOT( showDetailView(const QTask &) ) );
    connect( table, SIGNAL( currentItemChanged(const QModelIndex &) ),
            this, SLOT( currentEntryChanged() ) );

    connect( model, SIGNAL(modelReset()), this, SLOT( taskModelReset()));
    connect(qApp, SIGNAL( appMessage(const QString&,const QByteArray&) ),
            this, SLOT( appMessage(const QString&,const QByteArray&) ) );
    connect(qApp, SIGNAL(reload()), this, SLOT(reload()));
    connect(qApp, SIGNAL(flush()), this, SLOT(flush()));


    new TasksService(this);

    constructorDone = true;

    taskModelReset();
}

TodoWindow::~TodoWindow()
{
}

void TodoWindow::createUI()
{
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(0);
    grid->setMargin(0);
    grid->addWidget( table, 0, 0, 1, 2);

    newAction = new QAction( QIcon( ":icon/new" ), tr( "New" ), this );
    connect( newAction, SIGNAL(triggered()),
             this, SLOT( createNewEntry() ) );
    newAction->setWhatsThis( tr("Create a new task.") );

    editAction = new QAction( QIcon( ":icon/edit" ), tr( "Edit" ), this );
    connect( editAction, SIGNAL(triggered()),
             this, SLOT( editCurrentEntry() ) );
    editAction->setWhatsThis( tr("Edit the highlighted task.") );
    editAction->setVisible( false );

    deleteAction = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    connect( deleteAction, SIGNAL(triggered()),
             this, SLOT( deleteCurrentEntry() ) );
    deleteAction->setWhatsThis( tr("Delete the highlighted task.") );
    deleteAction->setVisible( false );

    findAction = new QAction( QIcon( ":icon/find" ), tr( "Find" ), this );
    findAction->setCheckable( true );
    findAction->setWhatsThis( tr("Search for a task.") );
    connect( findAction, SIGNAL( toggled(bool) ),
             this, SLOT( showFindWidget(bool) ) );

    markDoneAction = new QAction( QIcon( ":icon/ok" ), tr( "Mark task complete" ), this );
    markDoneAction->setWhatsThis( tr("Mark the current task as completed.") );
    markDoneAction->setVisible(false);
    connect( markDoneAction, SIGNAL(triggered()),
             this, SLOT( markTaskDone() ) );

    markNotDoneAction = new QAction( QIcon( ":icon/phone/reject" ), tr( "Mark task incomplete" ), this );
    markNotDoneAction->setWhatsThis( tr("Mark the current task as not completed.") );
    markNotDoneAction->setVisible(false);
    connect( markNotDoneAction, SIGNAL(triggered()),
             this, SLOT( markTaskNotDone() ) );

    if ( QtopiaSendVia::isDataSupported("text/x-vcalendar")) {
        beamAction = new QAction( QIcon( ":icon/beam" ), tr( "Send" ), this );
        connect( beamAction, SIGNAL(triggered()),
                 this, SLOT( beamCurrentEntry() ) );
        beamAction->setWhatsThis( tr("Send the highlighted task to another device.") );
        beamAction->setVisible(false);
    } else {
        beamAction = 0;
    }

    backAction = new QAction(
            QIcon(":icon/i18n/back"), tr("Back"), this);
    backAction->setEnabled(false);
    connect( backAction, SIGNAL(triggered()), this, SLOT(showListView()) );

#ifndef QTOPIA_PHONE
    QToolBar *bar = new QToolBar( this );
    bar->setMovable(false);

    QMenuBar *mb = menuBar(); // new QMenuBar( bar );
    addToolBar(bar);

    QMenu *edit = mb->addMenu( tr( "Task" ) );

    bar->addAction(newAction);
    bar->addAction(editAction);
    bar->addAction(deleteAction);
    bar->addAction(findAction);

    edit->addAction(newAction);
    edit->addAction(editAction);
    edit->addAction(deleteAction);
    edit->addAction(findAction);

    if (beamAction)
        edit->addAction(beamAction);
    bar->addSeparator();

    QMenu *view = mb->addMenu( tr( "View" ) );
    bar->addAction(backAction);

    view->addSeparator();
    view->addAction( tr("Select All"), this, SLOT( selectAll() ) );
    view->addSeparator();
    //view->insertItem( tr("Configure columns"), this, SLOT( configure() ) );

#else // else phone
    contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(newAction);
    contextMenu->addAction(markDoneAction);
    contextMenu->addAction(markNotDoneAction);
    contextMenu->addAction(editAction);
    contextMenu->addAction(deleteAction);


    if (beamAction)
        contextMenu->addAction(beamAction);

    actionCategory = new QAction(QIcon(":icon/viewcategory"), tr("View Category..."), this );
    connect( actionCategory, SIGNAL(triggered()), this, SLOT(selectCategory()));
    contextMenu->addAction(actionCategory);


    categoryLbl = new QLabel;
    categoryLbl->hide();
    grid->addWidget(categoryLbl, 1, 0, 1, 2);

    categoryDlg = 0;
#endif

#ifndef QTOPIA_PHONE
    // Search bar
    searchBar = new QToolBar(this);
    addToolBar( searchBar );
    searchBar->setMovable(false);

    searchEdit = new QLineEdit( searchBar );
    connect( searchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(startNewSearch(const QString &)) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(nextSearchItem()) );

    QAction *a = new QAction( QIcon( ":icon/i18n/next" ),
            tr( "Find Next" ), this );
    a->setWhatsThis( tr("Find the next matching task.") );
    searchBar->addAction(a);

    searchBar->hide();

    connect( a, SIGNAL(triggered()), this, SLOT(findNext()) );

    QSettings config("Trolltech","todo");
    config.beginGroup( "View" );
    int compFilt = config.value( "ShowComplete", 1 ).toInt();
    setShowCompleted( compFilt );

    // Filter bar
    QComboBox *cb = new QComboBox( listView );
    cb->addItem( tr("Pending Tasks") );
    cb->addItem( tr("All Tasks") );
    cb->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred) );
    cb->setCurrentIndex( model->filterCompleted() ? 0 : 1 );
    connect( cb, SIGNAL(activated(int)), this, SLOT(setShowCompleted(int)) );
    cb->setWhatsThis( tr("Show tasks with this completion status.") );

    QList<int> vl;
    catSelect = new QCategorySelector( TodoCategoryScope, QCategorySelector::Filter );
    connect( catSelect, SIGNAL(filterSelected(const QCategoryFilter &)), this, SLOT(catSelected(const QCategoryFilter &)) );
    catSelect->setWhatsThis( tr("Show tasks in this category.") );

    grid->addWidget( cb, 1, 0);
    grid->addWidget( catSelect, 1, 1);

    QCategoryFilter f;
    f.readConfig(config, "Category");
    catSelect->selectFilter(f);
    if (f.acceptAll())
        setWindowTitle(tr("Tasks"));
    else
        setWindowTitle(tr("Tasks") + " - " + f.label());
#else
    setWindowTitle(tr("Tasks"));
    QSettings config("Trolltech","todo");
    config.beginGroup( "View" );
    QCategoryFilter f;
    f.readConfig(config, "Category");
    catSelected(f);
#endif
    listView->setLayout(grid);
}

void TodoWindow::appMessage(const QString &msg, const QByteArray &data)
{
    /*@ \service Tasks */
    bool needShow = false;
    if ( msg == "receiveData(QString,QString)" ) {
        QDataStream stream(data);
        QString f,t;
        stream >> f >> t;
        if ( t.toLower() == "text/x-vcalendar" )
            if ( receiveFile(f) )
                needShow = true;
        QFile::remove(f);
    }

    if ( needShow ) {
//TODO: Something needs to happen here in the absence of the setKeepRunning() behaviour.
#if defined(Q_WS_QWS)
//      showMaximized();
#else
//      show();
#endif
//      raise();
//      setActiveWindow();
    }
}

void TodoWindow::showListView()
{
    if ( centralView->currentIndex() != 0 ) {
        backAction->setEnabled(false);
        centralView->setCurrentIndex(0);
        table->setEditFocus(true);
        setWindowTitle( tr("Tasks") );
#ifdef QTOPIA_PHONE
        newAction->setVisible(true);
        actionCategory->setVisible(true);
        editAction->setVisible(false);
        deleteAction->setVisible(false);
        if (beamAction) beamAction->setVisible(false);
#endif
    }
}

void TodoWindow::showDetailView(const QTask &t)
{
    if( isHidden() ) // only close after view if hidden on first activation
    {
        closeAfterDetailView = true;
    }

    todoView()->init( t );
    if (centralView->currentIndex() != 1) {
        backAction->setEnabled(true);
        centralView->setCurrentIndex(1);
        // ?? todoView()->setFocus();  //To avoid events being passed to QTable
        setWindowTitle( tr("Task Details") );

        /* make this the current list item too */
        QModelIndex newSel = model->index(t.uid());
        if ( newSel.isValid() ) {
            table->setCurrentIndex(newSel);
            table->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }

#ifdef QTOPIA_PHONE
        newAction->setVisible(false);
        actionCategory->setVisible(false);
        editAction->setVisible(true);
        deleteAction->setVisible(true);
        if (beamAction) beamAction->setVisible(true);
#endif
    }
}

TodoView* TodoWindow::todoView()
{
    if ( !tView ) {
        tView = new TodoView(this);
        connect( tView, SIGNAL(done()), this, SLOT(doneDetailView()) );
        connect( tView, SIGNAL(previous()), this, SLOT(viewPrevious()) );
        connect( tView, SIGNAL(next()), this, SLOT(viewNext()) );
#ifdef QTOPIA_PHONE
        QSoftMenuBar::setLabel(tView, Qt::Key_Select, QSoftMenuBar::NoLabel);
#endif
        centralView->addWidget(tView);
    }

    return tView;
}

void TodoWindow::doneDetailView()
{
    if (closeAfterDetailView) {
        closeAfterDetailView = false;
        close();
    }
    else {
        if ( prevTasks.count() == 0 ) {
            showListView();
        } else {
            showDetailView( model->task( prevTasks.top() ) );
            prevTasks.pop();
        }
    }
}

void TodoWindow::createNewEntry()
{
    TaskDialog* edit = new TaskDialog( parentWidget() );
    edit->setObjectName("edit-screen");
    edit->setModal(true);
    edit->setWindowTitle( tr( "New Task" ) );

    int ret = QtopiaApplication::execDialog( edit );

    if ( ret == QDialog::Accepted ) {
        QTask todo = edit->todoEntry();
        if ( todo.description().isEmpty() && !todo.notes().isEmpty()) {
            todo.setDescription( tr("Note") );
        }
        if ( !todo.description().isEmpty() ) {
            todo.setUid(model->addTask( todo ));
            findAction->setVisible( true );
            showDetailView(todo);
            currentEntryChanged( );
        }
    }

    delete edit;
}

void TodoWindow::deleteCurrentEntry()
{
    /*
       XXX

       This hackery is here as a fix for bug 132549.  When a task is updated,
       the selection model no longer includes the "current task".  As such,
       we deduce that if the current index is null, but the details view is
       being shown, just use that instead.

       We should really fix the model so that updates don't break the selection
       model.

       [this may no longer be necessary with the taskModelReset/currentEntryChanged code]
     */

    QTask task;
    bool viewSelected = false;
    if(centralView->currentIndex() == 1) {
        task = tView->task();
    } else if(table->currentIndex().isValid()) {
        task = table->currentTask();
        viewSelected = true;
    } else {
        return;
    }

    QString strName = Qt::escape(task.description().left(30));

    /* NOTE: we're currently doing single selection only.. */
    if ( viewSelected &&
         table->selectionMode() == TodoTable::ExtendedSelection ) {
        QList<QUniqueId> t = table->selectedTaskIds();

        if ( !t.count() ) return;

        QString str;
        if ( t.count() > 1 )
            str = tr("<qt>Are you sure you want to delete the %1 selected tasks?</qt>").arg( t.count() );
        else
            str = tr("<qt>Are you sure you want to delete:<p><b>%1</b>?</qt>").arg( strName  );

        switch( QMessageBox::warning( this, tr("Tasks"), str, tr("Yes"), tr("No"), 0, 0, 1 ) ) {
            case 0:
            {
                removeTasksQDLLink( t );
                model->removeList(t);
                showListView();
                currentEntryChanged();
            }
            break;
            case 1: break;
        }
    } else if ( Qtopia::confirmDelete( this, tr( "Tasks" ),
        strName.simplified() ) ) {
        removeTaskQDLLink( task );
        model->removeTask( task );
        showListView();
        currentEntryChanged();
    }
    if ( !model->count() && findAction->isChecked())
        findAction->setChecked(false);
}

void TodoWindow::editCurrentEntry()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    TaskDialog* edit = new TaskDialog( todo, parentWidget() );
    edit->setObjectName("edit-screen");
    edit->setModal( true );
    edit->setWindowTitle( tr( "Edit Task" ) );

    int ret = QtopiaApplication::execDialog( edit );

    if ( ret == QDialog::Accepted ) {
        todo = edit->todoEntry();
        model->updateTask( todo );
        todo = model->task(todo.uid());
        showDetailView(todo);
        currentEntryChanged();
    }

    delete edit;
}

void TodoWindow::setShowCompleted( int s )
{
    model->setFilterCompleted( s != 1 );
}

void TodoWindow::viewPrevious()
{
}

void TodoWindow::viewNext()
{
}

void TodoWindow::taskModelReset()
{
    /* Basically, we know that the current index will never be valid, so base our decisions
       on the number of tasks in the model */
    if ( model->count() <= 0 ) {
        QSoftMenuBar::setLabel( table, Qt::Key_Select, "new", tr("New") );
    } else {
        QSoftMenuBar::setLabel( table, Qt::Key_Select, QSoftMenuBar::View);
    }
}

void TodoWindow::currentEntryChanged( )
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    bool entrySelected = table->currentIndex().isValid();

#ifndef QTOPIA_PHONE
    editAction->setVisible(entrySelected);
    deleteAction->setVisible(entrySelected);
    if (beamAction) beamAction->setVisible(entrySelected);
#endif

    if (todo != QTask()) {
        if (todo.status() == QTask::Completed) {
            markDoneAction->setVisible(false);
            markNotDoneAction->setVisible(true);
        } else {
            markDoneAction->setVisible(true);
            markNotDoneAction->setVisible(false);
        }
    } else {
        markDoneAction->setVisible(false);
        markNotDoneAction->setVisible(false);
    }

    findAction->setVisible(entrySelected);
}

void TodoWindow::markTaskDone()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    if (todo != QTask()) {
        todo.setStatus(QTask::Completed);
        model->updateTask(todo);
        todo = model->task(todo.uid());
        if (centralView->currentIndex() == 1)
            showDetailView(todo);
        currentEntryChanged();
    }
}

void TodoWindow::markTaskNotDone()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    if (todo != QTask() && todo.status() == QTask::Completed) {
        todo.setStatus(QTask::InProgress);
        model->updateTask(todo);
        todo = model->task(todo.uid());
        if (centralView->currentIndex() == 1)
            showDetailView(todo);
        currentEntryChanged();
    }
}

void TodoWindow::reload()
{
    model->refresh();
}

void TodoWindow::flush()
{
    model->flush();
}

void TodoWindow::catSelected( const QCategoryFilter &c )
{
    model->setCategoryFilter( c );
#ifndef QTOPIA_PHONE
    if (!c.acceptAll())
        setWindowTitle( tr("Tasks") + " - " + c.label() );
    else
        setWindowTitle( tr("Tasks") );
#else
    if (c.acceptAll()) {
        categoryLbl->hide();
    } else {
        categoryLbl->setText(tr("Category: %1").arg(c.label(TodoCategoryScope)));
        categoryLbl->show();
    }
#endif
}

void TodoWindow::closeEvent( QCloseEvent *e )
{
    e->accept();
    QSettings config("Trolltech","todo");
    config.beginGroup( "View" );
    config.setValue( "ShowComplete", !model->filterCompleted() );
    model->categoryFilter().writeConfig(config, "Category");
}

#ifdef QTOPIA_PHONE
void TodoWindow::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Back || e->key() == Qt::Key_No ) {
        e->accept();
        if ( centralView->currentIndex() == 1 )
            showListView();
        else
            close();
    } else
        QMainWindow::keyPressEvent(e);
}

bool TodoWindow::eventFilter( QObject *o, QEvent *e )
{
    if(o == table)
    {
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            if ( ke->key() == Qt::Key_Select) {
                if(table->currentIndex().isValid())
                    return false;
                else {
                    createNewEntry();
                    return true;
                }
            }
        }
    }
    return false;
}
#endif

void TodoWindow::showFindWidget( bool s )
{
    Q_UNUSED(s);
#if 0
    showListView();
    if ( s ) {
        searchBar->show();
        searchEdit->setFocus();
    } else {
        searchBar->hide();
        model->clearSearch();
        table->clearSelection();
    }
#endif
}

void TodoWindow::startNewSearch(const QString &s)
{
    Q_UNUSED(s);
#if 0
    QModelIndex index = model->startSearch(s);
    if (index.isValid()) {
        table->setCurrentIndex(index);
        table->scrollTo(index);
    }
#endif
}

void TodoWindow::nextSearchItem()
{
#if 0
    QModelIndex index = model->nextSearchItem();
    if (index.isValid()) {
        table->setCurrentIndex(index);
        table->scrollTo(index);
    }
#endif
}

void TodoWindow::findNotFound()
{
    Qtopia::statusMessage( tr("Find: not found") );
}

void TodoWindow::findFound()
{
    Qtopia::statusMessage( "" );
}

void TodoWindow::findWrapped()
{
    Qtopia::statusMessage( tr("Find: reached end") );
}

void TodoWindow::setDocument( const QString &filename )
{
    QContent doc(filename);
    if ( doc.isValid() )
        receiveFile(doc.file());
    else
        receiveFile(filename);
}

bool TodoWindow::receiveFile( const QString &filename )
{
    QList<QTask> tl = QTask::readVCalendar( filename );

    if (tl.count() < 1) {
        // should spit out more appropriate message.
        QMessageBox::information(this, tr("New Tasks"),
                    tr("<p>Received empty task list.  No tasks added"),
                    QMessageBox::Ok);
        return false;
    }
    QString msg = tr("<P>%1 new tasks.<p>Do you want to add them to your Tasks?").
        arg(tl.count());

    if ( QMessageBox::information(this, tr("New Tasks"),
            msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
        for( QList<QTask>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
            model->addTask( *it );
        }
        return true;
    }
    return false;
}


void TodoWindow::beamCurrentEntry()
{
    if ( !table->currentIndex().isValid() ) {
        qWarning("todo::beamCurrentEntry called with nothing to beam");
        return;
    }

    QString description;

    ::unlink( beamfile.toLocal8Bit() ); // delete if exists
    QTask c = table->currentTask();
    if ( table->selectionMode() == TodoTable::ExtendedSelection ) {
        QList<QTask> l = table->selectedTasks();
        QTask::writeVCalendar( beamfile, l );

        if ( l.count() > 1 )
            description = tr("the %1 selected tasks").arg( l.count() );
        else
            description = c.description();
    } else {
        QTask::writeVCalendar( beamfile, c );
        description = c.description();
    }

    QFile f(beamfile);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data = f.readAll();
        QtopiaSendVia::sendData(this, data, "text/x-vcalendar");
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
        categoryDlg = new QCategoryDialog(TodoCategoryScope, QCategoryDialog::Filter, this);
        categoryDlg->setText( tr("Only tasks which have <i>all</i> the selected categories will be shown...") );
        categoryDlg->setObjectName("Todo List");
    }
    categoryDlg->selectFilter(model->categoryFilter());

    if (QtopiaApplication::execDialog(categoryDlg) == QDialog::Accepted)
        catSelected(categoryDlg->selectedFilter());
#endif
}

void TodoWindow::qdlActivateLink( const QDSActionRequest& request )
{
    // Check if we're already showing a task, if so push it onto the previous tasks
    // stack
    if ( table->currentIndex().isValid() ) {
        prevTasks.push( todoView()->task().uid() );
    }

    // Grab the link from the request and check that is one of ours
    QDLLink link( request.requestData() );
    if ( link.service() != "Tasks" ) {
        QDSActionRequest( request ).respond( tr( "Link doesn't belong to Tasks" ) );
        return;
    }
    const QByteArray dataRef = link.data();
    QDataStream refStream( dataRef );
    QUniqueId uid;
    refStream >> uid;
    if ( model->exists( uid ) ) {
        showDetailView(model->task(uid));
        showMaximized();
        QDSActionRequest( request ).respond();
    }
    else {
        QMessageBox::warning(
            this,
            tr("Tasks"),
            "<qt>" + tr("The selected task no longer exists.") + "</qt" );
        QDSActionRequest( request ).respond( "Task doesn't exist" );
    }
}

void TodoWindow::qdlRequestLinks( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    if ( model->count() == 0 ) {
        QMessageBox::warning(
            this,
            tr( "Tasks" ),
            "<qt>" + tr( "No tasks available." ) + "</qt>",
            QMessageBox::Ok );

        processingRequest.respond( "No tasks available." );

        return;
    }

    QTaskSelector *s = new QTaskSelector( false, ( isVisible() ? this : 0 ) );
    s->setModal( true );
    s->setModel(model);
    s->showMaximized();

    if( (s->exec() == QDialog::Accepted) && (s->taskSelected()) )
    {
        QTask task = s->selectedTask();
        QList<QDSData> links;
        links.push_back( taskQDLLink( task ) );

        QByteArray array;
        {
            QDataStream ds( &array, QIODevice::WriteOnly );
            ds << links;
        }

        processingRequest.respond( QDSData( array, QDLLink::listMimeType() ) );
    } else {
        processingRequest.respond( tr( "Task selection cancelled" ) );
    }
    delete s;
}

QDSData TodoWindow::taskQDLLink( QTask& task )
{
    // Check if we need to create the QDLLink
    QString keyString = task.customField( QDL::SOURCE_DATA_KEY );
    if ( keyString.isEmpty() ||
         !QDSData( QUniqueId( keyString ) ).isValid() ) {
        QByteArray dataRef;
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << task.uid();

        QDLLink link( "Tasks",
                      dataRef,
                      task.description(),
                      "pics/todolist/TodoList" );

        QDSData linkData = link.toQDSData();
        QUniqueId key = linkData.store();
        task.setCustomField( QDL::SOURCE_DATA_KEY, key.toString() );
        model->updateTask( task );

        return linkData;
    }

    // Get the link from the QDSDataStore
    return QDSData( QUniqueId( keyString ) );
}

void TodoWindow::removeTasksQDLLink( QList<QUniqueId>& taskIds )
{
    foreach( QUniqueId taskId, taskIds ) {
        QTask task = model->task( taskId );
        removeTaskQDLLink( task );
    }
}

void TodoWindow::removeTaskQDLLink( QTask& task )
{
    if ( task == QTask() )
        return;

    // Release any client QDLLinks
    QString links = task.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the task is a QDLLink source, if so break it
    QString key = task.customField( QDL::SOURCE_DATA_KEY );
    if ( !key.isEmpty() ) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();

        // Finally remove the stored key
        task.removeCustomField( QDL::SOURCE_DATA_KEY );
        model->updateTask( task );
    }
}

/*!
    \service TasksService Tasks
    \brief Provides the Qtopia Tasks service.

    The \i Tasks service enables applications to access features of
    the Tasks application.
*/

/*!
    \internal
*/
TasksService::~TasksService()
{
}

/*!
    Open a dialog to allow the user to create a new task.

    This slot corresponds to the QCop service message
    \c{Tasks::newTask()}.
*/
void TasksService::newTask()
{
    todo->createNewEntry();
}

/*!
    Add a new \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::addTask(QTask)}.
*/
void TasksService::addTask( const QTask& task )
{
    todo->model->addTask(task);
}

/*!
    Update an existing \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::updateTask(QTask)}.
*/
void TasksService::updateTask( const QTask& task )
{
    todo->model->updateTask(task);
}

/*!
    Remove an existing \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::removeTask(QTask)}.
*/
void TasksService::removeTask( const QTask& task )
{
    QTask taskCopy( task );
    todo->removeTaskQDLLink( taskCopy );
    todo->model->removeTask(task);
}

/*!
    Show the task identified by \a uid.

    This slot corresponds to the QCop service message
    \c{Tasks::showTask(QUniqueId)}.
*/
void TasksService::showTask( const QUniqueId& uid )
{
    QCategoryFilter allCat = QCategoryFilter(QCategoryFilter::All);
#ifndef QTOPIA_PHONE
    todo->catSelect->selectFilter(allCat);
#endif
    todo->catSelected(allCat);
    todo->table->setCurrentIndex(todo->model->index(uid));
}

/*!
    Activate the QDL link contained within \a request.

    The slot corresponds to a QDS service with a request data type of
    QDLLink::mimeType() and no response data.

    The slot corresponds to the QCop service message
    \c{Tasks::activateLink(QDSActionRequest)}.
*/
void TasksService::activateLink( const QDSActionRequest& request )
{
    todo->qdlActivateLink( request );
}

/*!
    Request for one or more QDL links using the hint contained within
    \a request.

    The slot corresponds to a QDS service with a request data type of
    "text/x-qstring" and response data type of QDLLink::listMimeType().

    The slot corresponds to the QCop service message
    \c{Tasks::requestLinks(QDSActionRequest)}.

*/
void TasksService::requestLinks( const QDSActionRequest& request )
{
    todo->qdlRequestLinks( request );
}

