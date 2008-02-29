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
#include "monthview.h"

#include <QSettings>
#include <QPainter>
#include <QResizeEvent>
#include <QTimeString>

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
                p->fillRect( cr.left(), cr.top(), cr.width() / 2, cr.height(),
                        normalBgColor );
                p->fillRect( cr.left() + (cr.width() / 2), cr.top(), cr.width() - (cr.width() / 2), cr.height(),
                        repeatBgColor );
            } else
                p->fillRect( cr.left(), cr.top(), cr.width(), cr.height(),
                        normalBgColor );
        else if (dpc->rAllDay)
            p->fillRect( cr.left(), cr.top(), cr.width(), cr.height(),
                    repeatBgColor );

        p->restore();
    }

    // Draw the numbers etc
    QCalendarWidget::paintCell(p, cr, cDay);

    // now for the lines.
    int h = line_height;
    int offset = 1; //cr.width() / 2 - h;

    int limit = cr.height();

    QList<int>::Iterator it = dpc->nLine.begin();
    while (it != dpc->nLine.end()) {
        // Divide up the available day into 24 hour chunks, and use
        // at least 4 pixels to denote an appointment.
        int p1 = (*it++) * limit / 24;
        int p2 = 4 + ((*it++) * limit / 24);

        // Be aware of the boundary case,
        // and step back a little if we run over.
        if (p2 > limit) {
            if (p1 > limit - 4)
                p1 = limit - 4;
            p2 = limit;
        }

        p->fillRect(cr.left() + offset, cr.top() + p1, h, p2 - p1, normalColor);
    }

    offset = cr.width() - h - offset;

    // Same again for the repeating appointments
    it = dpc->rLine.begin();
    while (it != dpc->rLine.end()) {
        int p1 = (*it++) * limit / 24;
        int p2 = 4 + ((*it++) * limit / 24);

        if (p2 > limit) {
            if (p1 > limit - 4)
                p1 = limit - 4;
            p2 = limit;
        }

        p->fillRect(cr.left() + offset, cr.top() + p1, h, p2 - p1, repeatColor);
    }
}

MonthView::MonthView(QWidget *parent)
    : QCalendarWidget(parent), line_height(5)
{
    setObjectName("monthview");

    setVerticalHeaderFormat(NoVerticalHeader);
    setFirstDayOfWeek( Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday );
    setGridVisible(true);

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
                repeatBgColor = repeatColor.light(160);
                repeatColor.setAlpha(96);
                foundRColor = true;
            }
        } else {
            if (!foundNColor) {
                normalColor = qvariant_cast<QColor>(model->data(model->index(i, 0), Qt::BackgroundColorRole));
                normalBgColor = normalColor.light(160);
                normalColor.setAlpha(96);
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
                int sp = startPos;
                int ep = endPos;
                if (i != f)
                    sp = 0;
                if (i != t)
                    ep = 24;
                if (a.repeatRule() == QAppointment::NoRepeat) {
                    dpc->nLine.append(sp);
                    dpc->nLine.append(ep);
                } else {
                    dpc->rLine.append(sp);
                    dpc->rLine.append(ep);
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

void MonthView::categorySelected( const QCategoryFilter &c )
{
    model->setCategoryFilter( c );
}









