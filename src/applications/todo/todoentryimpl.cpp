/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "todoentryimpl.h"

#include <qtopiaapplication.h>

#include <qtopia/pim/qtask.h>
#include <qtimestring.h>
#include <qcalendarwidget.h>
#include <QDL>
#include <QDLEditClient>
#if !defined(QTOPIA_PHONE)
#include <pixmapdisplay.h>
#endif

#include <qmessagebox.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qsoftmenubar.h>
#include <QScrollArea>

TaskDialog::TaskDialog( const QTask& task, QWidget *parent,
        Qt::WFlags fl )
    : QDialog( parent, fl
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    ),
      todo( task ), sv(0)
{
    init();

    todo.setCategories( task.categories() );

    inputDescription->setText( task.description() );
    comboPriority->setCurrentIndex( task.priority() - 1 );
    if ( task.isCompleted() )
        comboStatus->setCurrentIndex( 2 );
    else
        comboStatus->setCurrentIndex( task.status() );

    spinComplete->setValue( task.percentCompleted() );

    dueCheck->setChecked( task.hasDueDate() );
    if ( task.hasDueDate() )
        dueEdit->setDate( task.dueDate() );

    QDate date = task.startedDate();
    if ( !date.isNull() ) {
        startedCheck->setChecked(true);
        startedEdit->setDate( date );
    } else {
        startedCheck->setChecked(false);
    }

    date = task.completedDate();
    if ( !date.isNull() ) {
        completedCheck->setChecked(true);
        completedEdit->setDate( date );
    } else {
        completedCheck->setChecked(false);
    }

    inputNotes->setHtml( task.notes() );
    QDL::loadLinks( task.customField( QDL::CLIENT_DATA_KEY ),
                    QDL::clients( inputNotes ) );
    inputNotesQC->verifyLinks();

    // set up enabled/disabled logic
    updateFromTask();
}

/*
 *  Constructs a TaskDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
TaskDialog::TaskDialog( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    )
{
    init();

    // set up enabled/disabled logic
    updateFromTask();
}

void TaskDialog::init()
{
    buttonclose = false;

    //resize( 273, 300 );
    setWindowTitle( tr( "New Task" ) );
    QGridLayout *gl = new QGridLayout( this );
    gl->setSpacing( 3 );
    gl->setMargin( 0 );

    QTabWidget *tw = new QTabWidget( this );
    gl->addWidget(tw, 0, 0, 0, 2);
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
    inputNotes = new QTextEdit( noteTab );
    int rowCount = 0;
    inputNotesQC = new QDLEditClient( inputNotes, "qdlnotes" );
#ifdef QTOPIA_PHONE
    inputNotesQC->setupStandardContextMenu();
#else
    PixmapDisplay *linkButton = new PixmapDisplay( noteTab );
    linkButton->setPixmap(
        QIcon( ":icon/qdllink" ).pixmap(
            QStyle::PixelMetric(QStyle::PM_SmallIconSize), true ) );
    connect( linkButton, SIGNAL(clicked()), inputNotesWC, SLOT(requestLink()) );
    noteLayout->addWidget( linkButton, rowCount++, 0, Qt::AlignRight );
    linkButton->setFocusPolicy( Qt::NoFocus );
#endif
    noteLayout->addWidget( inputNotes, rowCount++, 0 );

#ifndef QTOPIA_DESKTOP
    sv = new QScrollArea(this);
    sv->setWidgetResizable(true);
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sv->setFocusPolicy(Qt::NoFocus);
    sv->setFrameStyle(QFrame::NoFrame);
    QWidget* taskDetail = new QWidget(sv);

    Ui::NewTaskDetail::setupUi(taskDetail);
    sv->setWidget(taskDetail);
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sv->setFrameStyle(QFrame::NoFrame);
    //taskdetail = new Ui::NewTaskDetail(sv->viewport());
    //sv->addChild(taskdetail);

    tw->addTab(sv, tr("Task"));
#else
    QWidget *tv = new QWidget(tw);
    Ui::NewTaskDetail::setupUi(tv);
    tw->addTab( tv, tr("Task") );
#endif
    tw->addTab( noteTab, tr("Notes"));

#ifdef QTOPIA_DESKTOP
    setTabOrder(comboCategory, buttonOk);
    setTabOrder(buttonOk, buttonCancel);
    buttonOk->setDefault(true);

    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
#endif // QTOPIA_DESKTOP

    connect( startedEdit, SIGNAL( dateChanged(const QDate&) ),
             this, SLOT( startDateChanged(const QDate&) ) );
    connect( completedEdit, SIGNAL( dateChanged(const QDate&) ),
             this, SLOT( endDateChanged(const QDate&) ) );

    QDate current = QDate::currentDate();

    dueEdit->setDate( current );
    startedEdit->setDate( QDate() );
    completedEdit->setDate( current );

    comboCategory->selectCategories(todo.categories());

    connect( comboStatus, SIGNAL( activated(int) ), this, SLOT( statusChanged(int) ) );

    connect( startedCheck, SIGNAL( toggled(bool) ), this, SLOT( startDateChecked() ) );
    connect( completedCheck, SIGNAL( toggled(bool) ), this, SLOT( endDateChecked() ) );
    connect( spinComplete, SIGNAL( valueChanged(int) ), this, SLOT( percentChanged(int) ) );

    inputDescription->setFocus();

    resize( 300, 300 );
#ifdef QTOPIA_DESKTOP
    setMaximumSize( sizeHint()*2 );
#endif
}

void TaskDialog::startDateChecked()
{
    if (startedCheck->isChecked())
        todo.setStartedDate(startedEdit->date());
    else
        todo.setStartedDate(QDate());
    updateFromTask();
}

void TaskDialog::endDateChecked()
{
    if (completedCheck->isChecked())
        todo.setCompletedDate(completedEdit->date());
    else
        todo.setCompletedDate(QDate());
    updateFromTask();
}

void TaskDialog::percentChanged(int percent)
{
    todo.setPercentCompleted(percent);
    updateFromTask();
}

void TaskDialog::statusChanged(int status)
{
    todo.setStatus(status);
    updateFromTask();
}

void TaskDialog::updateFromTask()
{
    int p = todo.percentCompleted();
    int s = todo.status();
    QDate sDate = todo.startedDate();
    QDate cDate = todo.completedDate();

    spinComplete->blockSignals(true);
    startedCheck->blockSignals(true);
    completedCheck->blockSignals(true);
    startedEdit->blockSignals(true);
    completedEdit->blockSignals(true);
    comboStatus->blockSignals(true);

    spinComplete->setValue(p);
    comboStatus->setCurrentIndex(s);
    startedCheck->setChecked(sDate.isValid());
    completedCheck->setChecked(cDate.isValid());
    if (sDate.isValid())
        startedEdit->setDate(sDate);
    if (cDate.isValid())
        completedEdit->setDate(cDate);

    spinComplete->blockSignals(false);
    startedCheck->blockSignals(false);
    completedCheck->blockSignals(false);
    startedEdit->blockSignals(false);
    completedEdit->blockSignals(false);
    comboStatus->blockSignals(false);

}

/*
 *  Destroys the object and frees any allocated resources
 */
TaskDialog::~TaskDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void TaskDialog::startDateChanged( const QDate& date )
{
    if ( date > completedEdit->date() )
        completedEdit->setDate( date );
}

void TaskDialog::endDateChanged( const QDate& date )
{
    if ( date < startedEdit->date() )
        startedEdit->setDate( date );
}

/*!
*/

const QTask &TaskDialog::todoEntry() const
{
    todo.setDescription( inputDescription->text() );
    todo.setPriority( (QTask::Priority) (comboPriority->currentIndex() + 1) );
    // percent completed and status are kept up to date, don't need to read
    // here.

    if (dueCheck->isChecked())
        todo.setDueDate( dueEdit->date() );
    else
        todo.clearDueDate();

    if (startedCheck->isChecked())
        todo.setStartedDate( startedEdit->date() );
    else
        todo.setStartedDate( QDate() );

    if (completedCheck->isChecked())
        todo.setCompletedDate( completedEdit->date() );
    else
        todo.setCompletedDate( QDate() );

    todo.setCategories( comboCategory->selectedCategories() );

    /* changing to plain text until we have a more efficient import export format */
    if (inputNotes->toPlainText().simplified().isEmpty())
        todo.setNotes(QString());
    else
        todo.setNotes( inputNotes->toHtml() );
    // XXX should load links here
    QString links;
    QDL::saveLinks( links, QDL::clients( inputNotes ) );
    todo.setCustomField( QDL::CLIENT_DATA_KEY, links );

    return todo;
}

void TaskDialog::show()
{
    buttonclose = false;
    QDialog::show();
}

/*!

*/
void TaskDialog::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_DESKTOP
    QTask old(todo);
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
    buttonclose = true;
    QDialog::reject();
}

void TaskDialog::accept()
{
    buttonclose = true;
    QDialog::accept();
}


#ifdef QTOPIA_DESKTOP

QCategorySelector *TaskDialog::categorySelect()
{
    return comboCategory;
}

void TaskDialog::updateCategories()
{
    if ( !comboCategory )
        return;

    connect( this, SIGNAL( categoriesChanged() ),
             comboCategory, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
                comboCategory, SLOT( categoriesChanged() ) );
}
#endif
