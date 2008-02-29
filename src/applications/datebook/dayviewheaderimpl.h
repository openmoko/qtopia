/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef DATEBOOKDAYHEADER_H
#define DATEBOOKDAYHEADER_H
#include <qwidget.h>
#include <qdatetime.h>

class QToolButton;
class QDateEdit;
class QFrame;
class QButtonGroup;

class DayViewHeader : public QWidget
{
    Q_OBJECT

public:
    DayViewHeader( bool bUseMonday, QWidget* parent = 0 );
    ~DayViewHeader();
    void setStartOfWeek( bool onMonday );

public slots:
    void goBack();
    void goForward();
    void setDate( int, int, int );
    void setDay( int );

signals:
    void dateChanged( const QDate & );

private slots:
    void init();
    void setupNames();

private:
    QDate currDate;
    bool bUseMonday;

    QToolButton *back;
    QToolButton *forward;
    QFrame *fraDays;
    QButtonGroup *grpDays;
    QToolButton *cmdDay1, *cmdDay2, *cmdDay3, *cmdDay4, *cmdDay5, *cmdDay6, *cmdDay7;
    QDateEdit *dButton;
};

#endif // DATEBOOKDAYHEADER_H

