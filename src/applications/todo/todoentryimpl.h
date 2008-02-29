/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H

#include <qtopia/pim/qtask.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qwidget.h>

#include <qtopia/pim/qappointment.h>

class QTextEdit;
class QScrollArea;
class QDLEditClient;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QDateEdit;
class QComboBox;
class QLabel;
class TodoCategorySelector;
class RecurrenceDetails;
class ReminderPicker;
class QStackedLayout;

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    TaskDialog( const QTask &task, QWidget *parent = 0, Qt::WFlags fl = 0 );
    TaskDialog( QList<QString> categories, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~TaskDialog();

    const QTask &todoEntry() const;
    const QAppointment &todoAppointment() const;

    void accept();

signals:
    void categoriesChanged();
    void taskEditAccepted(const QTask&, const QAppointment&);

protected slots:
    void startDateChanged( const QDate& );
    void endDateChanged( const QDate& );
    void dueDateChanged( const QDate& );

    void startDateChecked(bool);
    void endDateChecked(bool);
    void dueDateChecked(bool);

    void percentChanged(int);
    void statusChanged(int);

    void updateFromTask();

    void initTab(int, QScrollArea *parent);

private:
    void init();
    void initTaskTab(QScrollArea *);
    void initProgressTab(QScrollArea *);
    void initRecurrenceTab(QScrollArea *);
    void initNotesTab(QScrollArea *);

    mutable QTask todo;
    mutable QAppointment todoAppt;

    QTextEdit *inputNotes;
    QDLEditClient *inputNotesQC;

    QTime defaultReminderTime;
    QGroupBox *dueCheck, *startedCheck, *completedCheck;
    QDateEdit *dueEdit, *startedEdit, *completedEdit;
    QLineEdit *inputDescription;
    QComboBox *comboPriority, *comboStatus;
    QSpinBox *spinComplete;
    TodoCategorySelector *comboCategory;
    RecurrenceDetails *recurDetails;
    ReminderPicker *reminderPicker;
    QStackedLayout *recurStack;
    QWidget *recurControls;
    bool newTask;
};

#endif // NEWTASKDIALOG_H
