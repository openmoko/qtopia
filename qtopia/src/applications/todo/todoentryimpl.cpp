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

#include "todoentryimpl.h"
#include "nulldb.h"

#include <qtopia/vscrollview.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/categoryselect.h>
#include <qtopia/datebookmonth.h>
#include <qtopia/global.h>
#include <qtopia/pim/task.h>
#include <qtopia/timestring.h>
#include <qtopia/datepicker.h>
#include <qtopia/qpestyle.h>
#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif
#if !defined(QTOPIA_PHONE) && defined(QTOPIA_DATA_LINKING)
#include <qtopia/pixmapdisplay.h>
#include <qtopia/resource.h>
#endif

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

TaskDialog::TaskDialog( const PimTask& task, QWidget *parent,
			      const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    ),
      todo( task ), sv(0)
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
#ifdef QTOPIA_DATA_LINKING
    QDL::loadLinks( task.customField( QDL::DATA_KEY ), QDL::clients( inputNotes ) );
#endif

    // set up enabled/disabled logic
    dueButtonToggled();
    statusChanged();
}

/*
 *  Constructs a TaskDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TaskDialog::TaskDialog( int id, QWidget* parent,  const char* name, bool modal,
			      WFlags fl )
    : QDialog( parent, name, modal, fl
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    )
{
    if ( id != -1 && id != -2 ) {
	QArray<int> ids( 1 );
	ids[0] = id;
	todo.setCategories( ids );
    }

    init();

    // set up enabled/disabled logic
    dueButtonToggled();
    statusChanged();
}

void TaskDialog::init()
{
    buttonclose = FALSE;

    //resize( 273, 300 ); 
    setCaption( tr( "New Task" ) );
    QGridLayout *gl = new QGridLayout( this ); 
    gl->setSpacing( 3 );
    gl->setMargin( 0 );

    QTabWidget *tw = new QTabWidget( this, "TabWidget" );
    gl->addMultiCellWidget(tw, 0, 0, 0, 2);
    //
    // QtopiaDesktop uses cancel and ok buttons for this dialog.
    //
#ifdef QTOPIA_DESKTOP
    QPushButton *buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setText( tr( "Cancel" ) );

    gl->addWidget( buttonCancel, 1, 2 );

    QPushButton *buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setText( tr( "OK" ) );

    gl->addWidget( buttonOk, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    gl->addItem( spacer, 1, 0 );
#endif // QTOPIA_DESKTOP

    QWidget *noteTab = new QWidget( this );
    QGridLayout *noteLayout = new QGridLayout( noteTab );
    inputNotes = new QMultiLineEdit( noteTab );
    inputNotes->setWordWrap(QMultiLineEdit::WidgetWidth);
    int rowCount = 0;
#ifdef QTOPIA_DATA_LINKING
    QDLWidgetClient *inputNotesWC = new QDLWidgetClient( inputNotes, "qdlNotes" );
#ifdef QTOPIA_PHONE
    inputNotesWC->setupStandardContextMenu();
#else
    PixmapDisplay *linkButton = new PixmapDisplay( noteTab );
    linkButton->setPixmap( Resource::loadIconSet( "qdllink" )
					    .pixmap( QIconSet::Small, TRUE ) );
    connect( linkButton, SIGNAL(clicked()), inputNotesWC, SLOT(requestLink()) );
    noteLayout->addWidget( linkButton, rowCount++, 0, Qt::AlignRight );
    linkButton->setFocusPolicy( NoFocus );
#endif
#endif
    noteLayout->addWidget( inputNotes, rowCount++, 0 );

#ifndef QTOPIA_DESKTOP
    sv = new VScrollView(this);
    taskdetail = new NewTaskDetail(sv->viewport());
    sv->addChild(taskdetail);

    tw->addTab(sv, tr("Task"));
#else
    taskdetail = new NewTaskDetail(tw);
    tw->addTab( taskdetail, tr("Task") );
#endif
    tw->addTab( noteTab, tr("Notes"));

    setTabOrder(taskdetail->inputDescription, taskdetail->comboPriority);
    setTabOrder(taskdetail->comboPriority, taskdetail->comboStatus);
    setTabOrder(taskdetail->comboStatus, taskdetail->spinComplete);
    setTabOrder(taskdetail->spinComplete, taskdetail->checkDue);
    setTabOrder(taskdetail->checkDue, taskdetail->buttonDue);
    setTabOrder(taskdetail->buttonDue, taskdetail->buttonStart);
    setTabOrder(taskdetail->buttonStart, taskdetail->buttonEnd);
    setTabOrder(taskdetail->buttonEnd, taskdetail->comboCategory);

#ifdef QTOPIA_DESKTOP
    setTabOrder(taskdetail->comboCategory, buttonOk);
    setTabOrder(buttonOk, buttonCancel);
    buttonOk->setDefault(TRUE);

    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
#endif // QTOPIA_DESKTOP

    connect( taskdetail->buttonDue, SIGNAL( valueChanged(const QDate&) ),
             this, SLOT( dueDateChanged(const QDate&) ) );
    connect( taskdetail->buttonStart, SIGNAL( valueChanged(const QDate&) ),
             this, SLOT( startDateChanged(const QDate&) ) );
    connect( taskdetail->buttonEnd, SIGNAL( valueChanged(const QDate&) ),
             this, SLOT( endDateChanged(const QDate&) ) );

    QDate current = QDate::currentDate();

    taskdetail->buttonDue->setDate( current );
    taskdetail->buttonStart->setDate( QDate() );
    taskdetail->buttonEnd->setDate( current );

    taskdetail->comboCategory->setCategories( todo.categories(), "Todo List", // No tr
	tr("Tasks") );

    connect( taskdetail->checkDue, SIGNAL( clicked() ), this, SLOT( dueButtonToggled() ) );
    connect( taskdetail->comboStatus, SIGNAL( activated(int) ), this, SLOT( statusChanged() ) );
    taskdetail->inputDescription->setFocus();

    resize( 300, 300 );
#ifdef QTOPIA_DESKTOP
    setMaximumSize( sizeHint()*2 );
#endif
}

//void TaskDialog::resizeEvent(QResizeEvent *e)
//{
    //taskdetail->setFixedSize(-2*style().defaultFrameWidth() + e->size().width() - style().scrollBarExtent().width(), taskdetail->sizeHint().height());
//}

void TaskDialog::dueButtonToggled()
{
    taskdetail->buttonDue->setEnabled( taskdetail->checkDue->isChecked() );
}

void TaskDialog::statusChanged()
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
TaskDialog::~TaskDialog()
{
    // no need to delete child widgets, Qt does it all for us
}
void TaskDialog::dueDateChanged( const QDate& /* date */ )
{
}

void TaskDialog::startDateChanged( const QDate& date )
{
    if ( date > taskdetail->buttonEnd->date() )
	taskdetail->buttonEnd->setDate( date );
}

void TaskDialog::endDateChanged( const QDate& date )
{
    if ( date < taskdetail->buttonStart->date() )
	taskdetail->buttonStart->setDate( date );
}

/*!
*/

void TaskDialog::setCurrentCategory(int i)
{
    taskdetail->comboCategory->setCurrentCategory(i);
}

PimTask TaskDialog::todoEntry()
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
#ifdef QTOPIA_DATA_LINKING
    // XXX should load links here
    QString links;
    QDL::saveLinks( links, QDL::clients( inputNotes ) );
    todo.setCustomField( QDL::DATA_KEY, links );
#endif

    return todo;
}

void TaskDialog::show()
{
    buttonclose = FALSE;
    QDialog::show();
}

/*!

*/
void TaskDialog::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_DESKTOP
    PimTask old(todo);
    if ( !buttonclose && old.toRichText() != todoEntry().toRichText() ) {
	QString message = tr("Discard changes?");
	switch( QMessageBox::warning(this, tr("Tasks"), message,
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

void TaskDialog::reject()
{
    buttonclose = TRUE;
    QDialog::reject();
}

void TaskDialog::accept()
{
    buttonclose = TRUE;
    QDialog::accept();
}


#ifdef QTOPIA_DESKTOP

CategorySelect *TaskDialog::categorySelect()
{
    return taskdetail->comboCategory;
}

void TaskDialog::updateCategories()
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
