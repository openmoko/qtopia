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
#ifndef DATEBOOKWEEK
#define DATEBOOKWEEK

#include "periodview.h"
#include "layoutmanager.h"

#include <qlist.h>
#include <qlist.h>
#include <qvector.h>
#include <qstring.h>
#include <QViewport>

class QLabel;
class QResizeEvent;
class QSpinBox;
class QTimer;
class QHeaderView;

class WeekViewHeader;
class WeekViewContents;

class WeekView : public PeriodView
{
    Q_OBJECT

public:
    WeekView( QOccurrenceModel *, bool weekOnMonday, QWidget *parent = 0 );

    bool hasSelection() const { return false; }
    QAppointment currentAppointment() const { return QAppointment(); }
    bool calcWeek( const QDate &d, int &week, int &year ) const;

    QDate weekDate() const;

public slots:
    void selectDate( const QDate & );

    void setStartsOnMonday( bool );
    void setDayStarts( int h );

    void databaseUpdated();

    void redraw();

protected slots:
    void keyPressAppointment(QKeyAppointment *);

private slots:
    void dayActivated( int day );

    void setWeek( int y, int w );
    void setYear( int );
    void showAppointmentsLabel( QList<QOccurrence> & );
    void hideAppointmentsLabel();

private:
    void setTotalWeeks( int totalWeeks );
    int totalWeeks() const;

    void getAppointments();
    int year;
    int _week;
    WeekViewHeader *header;
    WeekViewContents *contents;
    QLabel *lblDesc;
    QTimer *tHide;
};

class WeekViewContents : public QViewport
{
    Q_OBJECT
public:
    WeekViewContents( WeekView *parent = 0 );

    void showAppointments( QList<QOccurrence> &ev , const QDate &startDate );
    void moveToHour( int h );
    void setStartOfWeek( bool bOnMonday );

    void alterDay( int );

signals:
    void activateWeekDay( int d );
    void appointmentsSelected( QList<QOccurrence> & );
    void selectionCleared();

protected slots:
    void keyPressAppointment(QKeyAppointment *);
    void timeStringChanged();

private:
    void positionItem( LayoutItem *i );
    LayoutItem *intersects( const LayoutItem * );
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void drawContents( QPainter *p);
    void contentsMousePressAppointment( QMouseAppointment * );
    void contentsMouseReleaseAppointment( QMouseAppointment * );
    void mousePressAppointment( QMouseAppointment * );
    void mouseReleaseAppointment( QMouseAppointment * );
    void resizeAppointment( QResizeEvent * );
    void updateWeekNames(bool wsom);

private:
    int posOfHour(int h) const;
    int hourAtPos(int p) const;
    QHeaderView *header;
    QList<LayoutManager*> items;
    QList<LayoutItem*> dayItems;
    int rowHeight;
    bool showingAppointment;
    WeekView *wv;
};


#endif
