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

#ifndef TIMEDVIEW_H
#define TIMEDVIEW_H

#include <QRect>
#include <QDateTime>
#include <QWidget>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>

class QItemSelectionModel;
class QAbstractItemDelegate;

class TimeManagerData;
class TimeManager : public QWidget
{
public:
    TimeManager(QWidget *parent = 0);
    virtual ~TimeManager();

    virtual int minimumGapHeight() const;
    int minimumHeight() const;

    virtual void resetMarks();
    virtual void clearMarks();
    virtual void populateMarks();
    virtual void addMark(int minutes);

    virtual int markPosition(int minutes) const;
    virtual int markMinutes(int position, int direction = 0) const;

    QList<int> marks() const;

    QSize minimumSizeHint() const;

    void setDaySpan(int, int);

    int dayStart() const;
    int dayEnd() const;

    void cacheLayout();

protected:
    // to pick up font changes
    void changeEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    void drawFiller(QPainter *, const QRect &);
    void drawItem(QPainter *, const QRect &, int minutes, int markpos);

    TimeManagerData *d;
};

class CompressedTimeManagerData;
class CompressedTimeManager : public TimeManager
{
public:
    CompressedTimeManager(QWidget *parent = 0);
    void populateMarks();
    void addMark(int minutes);
    void clearMarks();

    void setIdealHeight(int ideal);
    int idealHeight() const;

private:
    CompressedTimeManagerData *d;
};

class TimedViewData;
class TimedView : public QWidget
{
    Q_OBJECT

public:
    TimedView(QWidget *parent = 0);
    virtual ~TimedView();

    virtual void setDate(const QDate &d);
    QDate date() const;

    virtual void setDaySpan(int start, int end);

    virtual QDateTime start() const;
    virtual QDateTime end() const;

    void setModel(QOccurrenceModel *);
    void setTimeManager(TimeManager *);

    QOccurrenceModel *model() const;
    TimeManager *timeManager() const;

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex &);

    QModelIndex index(const QPoint& point) const;
    QDateTime timeAtPoint(const QPoint& globalPoint, int direction = 0) const;

    void setItemDelegate(QAbstractItemDelegate *);
    QAbstractItemDelegate *itemDelegate() const;

    QRect occurrenceRect(const QModelIndex &index) const;

signals:
    void selectionChanged(const QModelIndex&);

public slots:
    void reset();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    TimedViewData *d;
};
#endif // __TIMED_VIEW_H__
