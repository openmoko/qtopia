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

#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H

#ifdef QTOPIA_PHONE
#include "taskphonedetail.h"
#else
#include "taskdetail.h"
#endif

#include <qtopia/pim/task.h>

#ifdef QTOPIA_DESKTOP
#include <qtopia/categoryselect.h>
#endif // QTOPIA_DESKTOP

#include <qdatetime.h>
#include <qpalette.h>
#include <qdialog.h>

class QLabel;
class QTimer;
class DateBookMonth;
class QMultiLineEdit;
class QScrollView;


#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QPushButton;
class QTabWidget;

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    TaskDialog( const PimTask &task, QWidget *parent = 0, const char* name = 0,
		   bool modal = FALSE, WFlags fl = 0 );
    TaskDialog( int id, QWidget* parent = 0, const char* name = 0,
		   bool modal = FALSE, WFlags fl = 0 );
    ~TaskDialog();

    PimTask todoEntry();
    void setCurrentCategory(int);

    void show();

#ifdef QTOPIA_DESKTOP
    void updateCategories();
    CategorySelect *categorySelect();

#endif
 signals:
    void categoriesChanged();

protected slots:
    void dueDateChanged( const QDate& );
    void startDateChanged( const QDate& );
    void endDateChanged( const QDate& );
    
    void dueButtonToggled();
    void statusChanged();

protected:
    void closeEvent(QCloseEvent *);
    //void resizeEvent(QResizeEvent *);
    virtual void accept();
    virtual void reject();

private:
    void init();
    PimTask todo;
    bool buttonclose;

    QScrollView *sv;

    NewTaskDetail *taskdetail;
    QMultiLineEdit *inputNotes;
};

#endif // NEWTASKDIALOG_H
