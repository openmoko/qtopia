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


NewTaskDialog::NewTaskDialog( const PimTask& task, QWidget *parent,
			      const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ),
      todo( task )
{
    init();

    todo.setCategories( task.categories() );

    s->inputDescription->setText( task.description() );
    s->comboPriority->setCurrentItem( task.priority() - 1 );
    if ( task.isCompleted() )
	s->comboStatus->setCurrentItem( 2 );
    else
	s->comboStatus->setCurrentItem( task.status() );

    s->spinComplete->setValue( task.percentCompleted() );

    s->checkDue->setChecked( task.hasDueDate() );
    QDate date = task.dueDate();
    if ( task.hasDueDate() )
	s->buttonDue->setDate( date );

    date = task.startedDate();
    if ( !date.isNull() )
	s->buttonStart->setDate( date );

    date = task.completedDate();
    if ( !date.isNull() )
	s->buttonEnd->setDate( date );

    s->inputNotes->setText( task.notes() );

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
    QScrollView *sv = new QScrollView(this);
    sv->setResizePolicy(QScrollView::AutoOneFit);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(sv, 0, 0);

    s = new NewTaskDialogBase( sv->viewport() );
    sv->addChild( s );

    setTabOrder(s->inputDescription, s->comboPriority);
    setTabOrder(s->comboPriority, s->comboStatus);
    setTabOrder(s->comboStatus, s->spinComplete);
    setTabOrder(s->spinComplete, s->checkDue);
    setTabOrder(s->checkDue, s->buttonDue);
    setTabOrder(s->buttonDue, s->buttonStart);
    setTabOrder(s->buttonStart, s->buttonEnd);
    setTabOrder(s->buttonEnd, s->comboCategory);

#ifdef Q_WS_QWS
    s->buttonCancel->hide();
    s->buttonOk->hide();
#else
    setTabOrder(s->comboCategory, s->buttonOk);
    setTabOrder(s->buttonOk, s->buttonCancel);
    s->buttonOk->setDefault(TRUE);

    connect( s->buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( s->buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
#endif

    connect( s->buttonDue, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( dueDateChanged( const QDate& ) ) );
    connect( s->buttonStart, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( startDateChanged( const QDate& ) ) );
    connect( s->buttonEnd, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( endDateChanged( const QDate& ) ) );

    QDate current = QDate::currentDate();

    s->buttonDue->setDate( current );
    s->buttonStart->setDate( QDate() );
    s->buttonEnd->setDate( current );

    s->comboCategory->setCategories( todo.categories(), "Todo List", // No tr
	tr("Todo List") );

    connect( s->checkDue, SIGNAL( clicked() ), this, SLOT( dueButtonToggled() ) );
    connect( s->comboStatus, SIGNAL( activated(int) ), this, SLOT( statusChanged() ) );
    s->inputDescription->setFocus();

    resize( 300, 300 );
}

void NewTaskDialog::dueButtonToggled()
{
    s->buttonDue->setEnabled( s->checkDue->isChecked() );
}

void NewTaskDialog::statusChanged()
{
    PimTask::TaskStatus t = (PimTask::TaskStatus)s->comboStatus->currentItem();

    s->buttonStart->setEnabled( t != PimTask::NotStarted );
    s->buttonEnd->setEnabled( t == PimTask::Completed );

    // status change may lead to percent complete change. Work it out.
    s->spinComplete->blockSignals(TRUE);
    if (t == PimTask::NotStarted) {
	s->spinComplete->setValue(0);
	s->spinComplete->setEnabled( FALSE );
    } else if (t == PimTask::Completed) {
	s->spinComplete->setValue(100);
	s->spinComplete->setEnabled( FALSE );
    } else  {
	if (s->spinComplete->value() >= 100)
	    s->spinComplete->setValue(99);
	s->spinComplete->setEnabled( TRUE );
    }
    s->spinComplete->blockSignals(FALSE);
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
    if ( date > s->buttonEnd->date() )
	s->buttonEnd->setDate( date );
}

void NewTaskDialog::endDateChanged( const QDate& date )
{
    if ( date < s->buttonStart->date() )
	s->buttonStart->setDate( date );
}

/*!
*/

void NewTaskDialog::setCurrentCategory(int i)
{
    s->comboCategory->setCurrentCategory(i);
}

PimTask NewTaskDialog::todoEntry()
{
    todo.setDescription( s->inputDescription->text() );
    todo.setPriority( (PimTask::PriorityValue) (s->comboPriority->currentItem() + 1) );
    if ( s->comboStatus->currentItem() == 2 ) {
	todo.setCompleted( TRUE );
	todo.setPercentCompleted( 0 );
    } else {
	todo.setCompleted( FALSE );
	int percent = s->spinComplete->value();
	if ( percent >= 100 ) {
	    todo.setStatus( PimTask::Completed );
	} else  {
	    todo.setStatus( (PimTask::TaskStatus) s->comboStatus->currentItem() );
	}
	todo.setPercentCompleted( percent );
    }

    if (s->checkDue->isChecked())
	todo.setDueDate( s->buttonDue->date() );
    else
	todo.clearDueDate();

    todo.setStartedDate( s->buttonStart->date() );
    todo.setCompletedDate( s->buttonEnd->date() );

    todo.setCategories( s->comboCategory->currentCategories() );

    todo.setNotes( s->inputNotes->text() );

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
    return s->comboCategory;
}

void NewTaskDialog::updateCategories()
{
    if ( !s->comboCategory )
	return;

    connect( this, SIGNAL( categoriesChanged() ),
	     s->comboCategory, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
		s->comboCategory, SLOT( categoriesChanged() ) );
}
#endif
