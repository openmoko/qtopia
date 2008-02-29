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

#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H

#ifdef QTOPIA_PHONE
#include "ui_taskphonedetail.h"
#else
#include "ui_taskdetail.h"
#endif

#include <qtopia/pim/qtask.h>

#ifdef QTOPIA_DESKTOP
#include <categoryselect.h>
#endif // QTOPIA_DESKTOP

#include <qdatetime.h>
#include <qpalette.h>
#include <qdialog.h>
#include <QTextEdit>

class QLabel;
class QTimer;
class QTextEdit;
class QScrollView;
class VScrollView;


#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QTabWidget;
class QScrollArea;
class QDLEditClient;

class TaskDialog : public QDialog, private Ui::NewTaskDetail
{
    Q_OBJECT

public:
    TaskDialog( const QTask &task, QWidget *parent = 0, Qt::WFlags fl = 0 );
    TaskDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~TaskDialog();

    const QTask &todoEntry() const;

    void show();

#ifdef QTOPIA_DESKTOP
    void updateCategories();
    QCategorySelector *categorySelect();

#endif
 signals:
    void categoriesChanged();

protected slots:
    void startDateChanged( const QDate& );
    void endDateChanged( const QDate& );

    void startDateChecked();
    void endDateChecked();
    void percentChanged(int);
    void statusChanged(int);

    void updateFromTask();

protected:
    void closeEvent(QCloseEvent *);
    //void resizeEvent(QResizeEvent *);
    virtual void accept();
    virtual void reject();

private:
    void init();
    mutable QTask todo;
    bool buttonclose;

    QScrollArea *sv;

    QTextEdit *inputNotes;
    QDLEditClient *inputNotesQC;
};

#endif // NEWTASKDIALOG_H
