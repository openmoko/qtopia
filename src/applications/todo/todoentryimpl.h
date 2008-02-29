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

#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H

#include "todoentry.h"

#include <qtopia/pim/task.h>

#include <qdatetime.h>
#include <qpalette.h>
#include <qdialog.h>

class QLabel;
class QTimer;
class DateBookMonth;

class NewTaskDialog : public QDialog
{
    Q_OBJECT

public:
    NewTaskDialog( const PimTask &task, QWidget *parent = 0, const char* name = 0,
		   bool modal = FALSE, WFlags fl = 0 );
    NewTaskDialog( int id, QWidget* parent = 0, const char* name = 0,
		   bool modal = FALSE, WFlags fl = 0 );
    ~NewTaskDialog();

    PimTask todoEntry();
    void setCurrentCategory(int);

protected slots:
    void dueDateChanged( const QDate& );
    void startDateChanged( const QDate& );
    void endDateChanged( const QDate& );
    
    void dueButtonToggled();
    void statusChanged();

protected:
    virtual void accept();
    virtual void reject();

private:
    void init();
    PimTask todo;

    NewTaskDialogBase *s;
};

#endif // NEWTASKDIALOG_H
