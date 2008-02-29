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
#include "monthview.h"

#include <QSettings>
#include <QPainter>
#include <QResizeEvent>
#include <QTimeString>

#include <QDebug>

static QColor repeatBgColor(0,0,0);
static QColor normalBgColor(0,0,0);
static QColor repeatColor(0,0,0);
static QColor normalColor(0,0,0);
static bool foundRColor = false;
static bool foundNColor = false;

// estimate cell height.
void MonthView::resizeAppointment( QResizeEvent *e )
{
    int ch = e->size().height() / 7; // 6 cells, one more for header.
    if (ch > 0)
        line_height = qMax(5, ch / 6);
    else
        line_height = 5;
    QCalendarWidget::resizeEvent(e);
}

void MonthView::paintCell(QPainter *p, const QRect &cr, const QDate &cDay) const
{
    // one color for repeating, one for not, one for travel.
    //
    // red/pink normal.  blue/lightblue repeating.
    // Green/light green travel.
    //
    // show for each day, that day traveling into next :)
    // e.g. work out the pixel values.  the lines.

    if (!paintCache.contains(cDay)) {
        QCalendarWidget::paintCell(p, cr, cDay);
        return;
    }

    DayPaintCache *dpc = paintCache[cDay];

    // now, do the painting.
    // later will want to pattern on all day, for now, just paint
    // and draw.

    if (dpc->nAllDay || dpc->rAllDay || dpc->tAllDay) {
        p->save();

        if (dpc->nAllDay)
            if (dpc->rAllDay) {
                p->fillRect( cr.left(), cr.top(), cr.width(), cr.height() / 2,
                        normalBgColor );
                p->fillRect( cr.left(), cr.top() + (cr.height() / 2), cr.width(), cr.height() / 2,
                        repeatBgColor );
            } else
                p->fillRect( cr.left(), cr.top(), cr.width(), cr.height(),
                        normalBgColor );
        else if (dpc->rAllDay)
            p->fillRect( cr.left(), cr.top(), cr.width(), cr.height(),
                    repeatBgColor );

        p->restore();
    }
    //else
        //QCalendarWidget::paintCell(p, cr, cDay);

    // now for the lines.
    int h = line_height;
    int y = cr.height() / 2 - h;


    QList<int>::Iterator it = dpc->nLine.begin();
    while (it != dpc->nLine.end()) {
        int h1 = *it;
        ++it;
        int h2 = *it;
        ++it;

        //
        // Divide up the available day into 24 hour chunks, and use
        // 4 pixels to denote an appointment.  Be aware of the boundary case,
        // and step back a little if we run over.
        //
        int x1, x2;

        x1 = h1 * cr.width() / 24;
        x2 = (h2 * cr.width() / 24) + 4;

        if (x2 > cr.width()) {
            if (x1 > cr.width() - 4) {
                x1 = cr.width() - 4;
            }
            x2 = cr.width();
        }

        p->fillRect(cr.left() + x1, cr.top() + y, x2 - x1, h, normalColor);
    }

    y += h;

    it = dpc->rLine.begin();
    while (it != dpc->rLine.end()) {
        int h1 = *it;
        ++it;
        int h2 = *it;
        ++it;

        //
        // Divide up the available day into 24 hour chunks, and use
        // 4 pixels to denote an appointment.  Be aware of the boundary case,
        // and step back a little if we run over.
        //
        int x1;
        int x2;

        x1 = h1 * cr.width() / 24;
        x2 = (h2 * cr.width() / 24) + 4;
        if (x2 > cr.width()) {
            if (x1 > cr.width() - 4) {
                x1 = cr.width() - 4;
            }
            x2 = cr.width();
        }

        p->fillRect(cr.left() + x1, cr.top() + y, x2 - x1, h, repeatColor);
    }
    QCalendarWidget::paintCell(p, cr, cDay);
}

MonthView::MonthView(QWidget *parent)
    : QCalendarWidget(parent), line_height(5)
{
    setObjectName("monthview");

    setVerticalHeaderFormat(NoVerticalHeader);
    setFirstDayOfWeek( Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday );

    QDate start = QDate::currentDate();
    start.setYMD(start.year(), start.month(), 1);
    QDate end = start.addDays(start.daysInMonth() - 1);

    model = new QOccurrenceModel(QDateTime(start, QTime(0, 0, 0)), QDateTime(end.addDays(1), QTime(0, 0)), this);

    connect(model, SIGNAL(modelReset()), this, SLOT(rebuildCache()));
    connect(this, SIGNAL(currentPageChanged(int, int)), this, SLOT(updateModelRange(int, int)));
}

MonthView::~MonthView()
{
}

void MonthView::rebuildCache()
{
    // Clear the old cache
    qDeleteAll(paintCache);
    paintCache.clear();

    for (int i = 0; i < model->rowCount(); ++i) {
        QOccurrence o = model->occurrence(i);
        QAppointment a = o.appointment();

        if (a.hasRepeat()) {
            if (!foundRColor) {
                repeatColor = qvariant_cast<QColor>(model->data(model->index(i, 0), Qt::BackgroundColorRole));
                repeatBgColor = repeatColor.light(170);
                foundRColor = true;
            }
        } else {
            if (!foundNColor) {
                normalColor = qvariant_cast<QColor>(model->data(model->index(i, 0), Qt::BackgroundColorRole));
                normalBgColor = normalColor.light(170);
                foundNColor = true;
            }
        }

        QDate f = o.startInCurrentTZ().date();
        QDate t = o.endInCurrentTZ().date();

        bool normalAllDay = false;
        bool repeatAllDay = false;
        int startPos = 0;
        int endPos = 24;
        if (a.isAllDay()) {
            if (a.repeatRule() == QAppointment::NoRepeat)
                normalAllDay = true;
            else
                repeatAllDay = true;
        } else {
            startPos =  a.startInCurrentTZ().time().hour();
            endPos = a.endInCurrentTZ().time().hour();
        }

        for (QDate i = f; i <= t; i = i.addDays(1)) {

            // get item.
            DayPaintCache *dpc;
            if (!paintCache.contains(i)) {
                dpc = new DayPaintCache();
                paintCache.insert(i, dpc);
            } else {
                dpc = paintCache[i];
            }

            if (normalAllDay)
                dpc->nAllDay = true;
            else if (repeatAllDay)
                dpc->rAllDay = true;
            else {
                if (a.repeatRule() == QAppointment::NoRepeat) {
                    if (i == f)
                        dpc->nLine.append(startPos);
                    else
                        dpc->nLine.append(0);

                    if (i == t)
                        dpc->nLine.append(endPos);
                    else
                        dpc->nLine.append(24);
                } else {
                    if (i == f)
                        dpc->rLine.append(startPos);
                    else
                        dpc->rLine.append(0);

                    if (i == t)
                        dpc->rLine.append(endPos);
                    else
                        dpc->rLine.append(24);
                }
            }
        }
    }

    update();
}

void MonthView::updateModelRange(int year, int month)
{
    QDate start(year, month, 1);
    QDate end = start.addDays(start.daysInMonth() - 1);

    model->setRange(QDateTime(start, QTime(0, 0, 0)), QDateTime(end.addDays(1), QTime(0, 0)));
}










