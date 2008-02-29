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

#include "todoentryimpl.h"
#include "nulldb.h"

#include <qtopia/categoryselect.h>
#include <qtopia/datebookmonth.h>
#include <qtopia/global.h>
#include <qtopia/imageedit.h>
#include <qtopia/pim/task.h>
#include <qtopia/timestring.h>
#include <qtopia/datepicker.h>

#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qscrollview.h>
#include <qlayout.h>

/* 
 *  Constructs a NewTaskDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
NewTaskDialogBase::NewTaskDialogBase( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "NewTaskDialogBase" );
    resize( 273, 300 ); 
    setCaption( tr( "New Task" ) );
    NewTaskDialogBaseLayout = new QGridLayout( this ); 
    NewTaskDialogBaseLayout->setSpacing( 3 );
    NewTaskDialogBaseLayout->setMargin( 0 );

    TabWidget = new QTabWidget( this, "TabWidget" );

    tab = new QWidget( TabWidget, "tab" );
    tabLayout = new QGridLayout( tab ); 
    tabLayout->setSpacing( 3 );
    tabLayout->setMargin( 3 );
    TabWidget->insertTab( tab, tr( "Task" ) );

    NewTaskDialogBaseLayout->addMultiCellWidget( TabWidget, 0, 0, 0, 2 );

    //
    // QtopiaDesktop uses cancel and ok buttons for this dialog.
    //
#ifdef QTOPIA_DESKTOP
    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setText( tr( "Cancel" ) );

    NewTaskDialogBaseLayout->addWidget( buttonCancel, 1, 2 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setText( tr( "OK" ) );

    NewTaskDialogBaseLayout->addWidget( buttonOk, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    NewTaskDialogBaseLayout->addItem( spacer, 1, 0 );
#endif // QTOPIA_DESKTOP

    // signals and slots connections
}

/*  
 *  Destroys the object and frees any allocated resources
 */
NewTaskDialogBase::~NewTaskDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void NewTaskDialogBase::dateChanged( const QString & )
{
    qWarning( "NewTaskDialogBase::dateChanged( const QString & ): Not implemented yet!" );
}

void NewTaskDialogBase::dateChanged( int, int, int )
{
    qWarning( "NewTaskDialogBase::dateChanged( int, int, int ): Not implemented yet!" );
}

NewTaskDialog::NewTaskDialog( const PimTask& task, QWidget *parent,
			      const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ),
      todo( task )
{
    init();

    todo.setCategories( task.categories() );

    taskdetail->inputDescription->setText( task.description() );
    taskdetail->comboPriority->setCurrentItem( task.priority() - 1 );
    if ( task.isCompleted() )
	taskdetail->comboStatus->setCurrentItem( 2 );
    else
	taskdetail->comboStatus->setCurrentItem( task.status() );

    taskdetail->spinComplete->setValue( task.percentCompleted() );

    taskdetail->checkDue->setChecked( task.hasDueDate() );
    QDate date = task.dueDate();
    if ( task.hasDueDate() )
	taskdetail->buttonDue->setDate( date );

    date = task.startedDate();
    if ( !date.isNull() )
	taskdetail->buttonStart->setDate( date );

    date = task.completedDate();
    if ( !date.isNull() )
	taskdetail->buttonEnd->setDate( date );

    inputNotes->setText( task.notes() );

    // set up enabled/disabled logic
    dueButtonToggled();
    statusChanged();
}

/*
 *  Constructs a NewTaskDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
NewTaskDialog::NewTaskDialog( int id, QWidget* parent,  const char* name, bool modal,
			      WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( id != -1 ) {
	QArray<int> ids( 1 );
	ids[0] = id;
	todo.setCategories( ids );
    }

    init();

    // set up enabled/disabled logic
    dueButtonToggled();
    statusChanged();
}

void NewTaskDialog::init()
{
    buttonclose = FALSE;

    s = new NewTaskDialogBase(this);

    s->TabWidget->setCurrentPage(s->TabWidget->currentPageIndex());
    while (s->TabWidget->currentPage()) {
	s->TabWidget->removePage(s->TabWidget->currentPage());
    }

    taskdetail = new NewTaskDetail(this);
    inputNotes = new QMultiLineEdit(this);

    QScrollView *sv = new QScrollView(this);
    sv->setHScrollBarMode(QScrollView::AlwaysOff);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->addChild(taskdetail);

    s->TabWidget->addTab(sv, tr("Task"));
    s->TabWidget->addTab(inputNotes, tr("Notes"));

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(s);

    setTabOrder(taskdetail->inputDescription, taskdetail->comboPriority);
    setTabOrder(taskdetail->comboPriority, taskdetail->comboStatus);
    setTabOrder(taskdetail->comboStatus, taskdetail->spinComplete);
    setTabOrder(taskdetail->spinComplete, taskdetail->checkDue);
    setTabOrder(taskdetail->checkDue, taskdetail->buttonDue);
    setTabOrder(taskdetail->buttonDue, taskdetail->buttonStart);
    setTabOrder(taskdetail->buttonStart, taskdetail->buttonEnd);
    setTabOrder(taskdetail->buttonEnd, taskdetail->comboCategory);

#ifdef QTOPIA_DESKTOP
    setTabOrder(taskdetail->comboCategory, s->buttonOk);
    setTabOrder(s->buttonOk, s->buttonCancel);
    s->buttonOk->setDefault(TRUE);

    connect( s->buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( s->buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
#endif // QTOPIA_DESKTOP

    connect( taskdetail->buttonDue, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( dueDateChanged( const QDate& ) ) );
    connect( taskdetail->buttonStart, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( startDateChanged( const QDate& ) ) );
    connect( taskdetail->buttonEnd, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( endDateChanged( const QDate& ) ) );

    QDate current = QDate::currentDate();

    taskdetail->buttonDue->setDate( current );
    taskdetail->buttonStart->setDate( QDate() );
    taskdetail->buttonEnd->setDate( current );

    taskdetail->comboCategory->setCategories( todo.categories(), "Todo List", // No tr
	tr("Todo List") );

    connect( taskdetail->checkDue, SIGNAL( clicked() ), this, SLOT( dueButtonToggled() ) );
    connect( taskdetail->comboStatus, SIGNAL( activated(int) ), this, SLOT( statusChanged() ) );
    taskdetail->inputDescription->setFocus();

    resize( 300, 300 );
}

void NewTaskDialog::dueButtonToggled()
{
    taskdetail->buttonDue->setEnabled( taskdetail->checkDue->isChecked() );
}

void NewTaskDialog::statusChanged()
{
    PimTask::TaskStatus t = (PimTask::TaskStatus)taskdetail->comboStatus->currentItem();

    taskdetail->buttonStart->setEnabled( t != PimTask::NotStarted );
    taskdetail->buttonEnd->setEnabled( t == PimTask::Completed );

    // status change may lead to percent complete change. Work it out.
    taskdetail->spinComplete->blockSignals(TRUE);
    if (t == PimTask::NotStarted) {
	taskdetail->spinComplete->setValue(0);
	taskdetail->spinComplete->setEnabled( FALSE );
    } else if (t == PimTask::Completed) {
	taskdetail->spinComplete->setValue(100);
	taskdetail->spinComplete->setEnabled( FALSE );
    } else  {
	if (taskdetail->spinComplete->value() >= 100)
	    taskdetail->spinComplete->setValue(99);
	taskdetail->spinComplete->setEnabled( TRUE );
    }
    taskdetail->spinComplete->blockSignals(FALSE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
NewTaskDialog::~NewTaskDialog()
{
    // no need to delete child widgets, Qt does it all for us
}
void NewTaskDialog::dueDateChanged( const QDate& /* date */ )
{
}

void NewTaskDialog::startDateChanged( const QDate& date )
{
    if ( date > taskdetail->buttonEnd->date() )
	taskdetail->buttonEnd->setDate( date );
}

void NewTaskDialog::endDateChanged( const QDate& date )
{
    if ( date < taskdetail->buttonStart->date() )
	taskdetail->buttonStart->setDate( date );
}

/*!
*/

void NewTaskDialog::setCurrentCategory(int i)
{
    taskdetail->comboCategory->setCurrentCategory(i);
}

PimTask NewTaskDialog::todoEntry()
{
    todo.setDescription( taskdetail->inputDescription->text() );
    todo.setPriority( (PimTask::PriorityValue) (taskdetail->comboPriority->currentItem() + 1) );
    if ( taskdetail->comboStatus->currentItem() == 2 ) {
	todo.setCompleted( TRUE );
	todo.setPercentCompleted( 0 );
    } else {
	todo.setCompleted( FALSE );
	int percent = taskdetail->spinComplete->value();
	if ( percent >= 100 ) {
	    todo.setStatus( PimTask::Completed );
	} else  {
	    todo.setStatus( (PimTask::TaskStatus) taskdetail->comboStatus->currentItem() );
	}
	todo.setPercentCompleted( percent );
    }

    if (taskdetail->checkDue->isChecked())
	todo.setDueDate( taskdetail->buttonDue->date() );
    else
	todo.clearDueDate();

    todo.setStartedDate( taskdetail->buttonStart->date() );
    todo.setCompletedDate( taskdetail->buttonEnd->date() );

    todo.setCategories( taskdetail->comboCategory->currentCategories() );

    todo.setNotes( inputNotes->text() );

    return todo;
}

void NewTaskDialog::show()
{
    buttonclose = FALSE;
    QDialog::show();
}

/*!

*/
void NewTaskDialog::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_DESKTOP
    PimTask old(todo);
    if ( !buttonclose && old.toRichText() != todoEntry().toRichText() ) {
	QString message = tr("Discard changes?");
	switch( QMessageBox::warning(this, tr("Todo List"), message,
		QMessageBox::Yes, QMessageBox::No) ) {

	    case QMessageBox::Yes:
		QDialog::closeEvent(e);
	    break;
	    case QMessageBox::No:
		e->ignore();
		todo = old;
	    break;
	}
    } else
	QDialog::closeEvent(e);
#else
    QDialog::closeEvent(e);
#endif
}

void NewTaskDialog::reject()
{
    buttonclose = TRUE;
    QDialog::reject();
}

void NewTaskDialog::accept()
{
    buttonclose = TRUE;
    QDialog::accept();
}


#ifdef QTOPIA_DESKTOP

CategorySelect *NewTaskDialog::categorySelect()
{
    return taskdetail->comboCategory;
}

void NewTaskDialog::updateCategories()
{
    if ( !taskdetail->comboCategory )
	return;

    connect( this, SIGNAL( categoriesChanged() ),
	     taskdetail->comboCategory, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
		taskdetail->comboCategory, SLOT( categoriesChanged() ) );
}
#endif
