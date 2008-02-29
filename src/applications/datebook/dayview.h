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
#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>

#include <QDate>
#include <QWidget>

#include "timedview.h"
#include "appointmentlist.h"

class QLabel;
class QScrollArea;
class QPushButton;

namespace DuplicatedFromCalendarWidget 
{
    class QCalendarTextNavigator;
};

class DayView : public QWidget
{
    Q_OBJECT

public:
    DayView(QWidget *parent = 0, const QCategoryFilter& c = QCategoryFilter(), QSet<QPimSource> set = QSet<QPimSource>());

    QModelIndex currentIndex() const;
    QAppointment currentAppointment() const;
    QOccurrence currentOccurrence() const;

    QDate currentDate() const;

    bool allDayFolded() const;
    bool allDayFoldingAvailable() const;
    void setAllDayFolded(bool f);

public slots:

    void selectDate( const QDate & );
    void selectDate( int year, int month );

    void setDaySpan( int starthour, int endhour );

    void setVisibleSources(QSet<QPimSource> set);

    void firstTimed();
    void firstAllDay();
    void lastTimed();
    void lastAllDay();
    void nextOccurrence();
    void previousOccurrence();

    void prevDay();
    void nextDay();

    //void setCurrentIndex(const QModelIndex &);
    void setCurrentOccurrence(const QOccurrence &o);
    void setCurrentAppointment(const QAppointment &a);

    void timedSelectionChanged(const QModelIndex &index);
    void allDayOccurrenceChanged(const QModelIndex &index);
    void allDayOccurrenceActivated(const QModelIndex &index);

    void updateHiddenIndicator(int hidden);

    void modelsReset();
    void categorySelected( const QCategoryFilter &c );

private slots:
    void updateView();

signals:
    void removeOccurrence( const QOccurrence& );
    void editOccurrence( const QOccurrence& );
    void removeAppointment( const QAppointment& );
    void editAppointment( const QAppointment& );
    void beamAppointment( const QAppointment& );
    void newAppointment();
    void newAppointment( const QString & );
    void newAppointment( const QDateTime &dstart, const QDateTime &dend );
    void showDetails();
    void dateChanged();
    void selectionChanged();
    void closeView();

protected:
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent( QMouseEvent * event );
    bool eventFilter(QObject *o, QEvent *e);
    void resizeEvent(QResizeEvent *);
    bool event(QEvent *event);

private:
    void updateHeaderText();
    QOccurrenceModel *currentModel() const;

    QPushButton *mNextDay, *mPrevDay;

    QLabel *mWeekdayLabel;
    QLabel *mDateLabel;
    TimedView *mTimedView;
    AppointmentList *mAllDayList;
    QLabel *mHiddenIndicator;

    QOccurrenceModel *timedModel;
    QOccurrenceModel *allDayModel;
    CompressedTimeManager *mTimeManager;
    QScrollArea *mScrollArea;

    QUniqueId lastSelectedTimedId;
    QUniqueId lastSelectedAllDayId;

    QDate targetDate;
    QTimer *cacheDelayTimer;
    DuplicatedFromCalendarWidget::QCalendarTextNavigator *nav;

    bool allDayFocus;
};


#endif
