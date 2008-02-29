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
#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <qtopia/pim/qappointment.h>
//#include <qtopia/qcalendarwidget.h>
#include <qtopia/pim/qappointmentmodel.h>

#include <QDateTime>
#include <QCalendarWidget>
#include <QBasicTimer>

class DayPaintCache
{
public:
    DayPaintCache() : nAllDay(false), rAllDay(false), tAllDay(false) {}

    QList<int> nLine;
    QList<int> rLine;
    QList<int> tLine;

    bool nAllDay;
    bool rAllDay;
    bool tAllDay;
};

class MonthView : public QCalendarWidget
{
    Q_OBJECT

public:
    MonthView(QWidget *parent = 0);
    ~MonthView();

public slots:
    void rebuildCache();

    void updateModelRange(int year, int month);
    void categorySelected( const QCategoryFilter &c );

protected:
    void paintCell(QPainter *p, const QRect &cr, const QDate &cDay) const;

private:
    void resizeAppointment(QResizeEvent *e);
    QList<QOccurrence> daysAppointments;

    QOccurrenceModel *model;

    //QList<DayPaintCache> paintCache;
    QMap<QDate, DayPaintCache*> paintCache;
    int line_height;
};

#endif
