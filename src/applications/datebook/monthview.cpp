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
#include "monthview.h"

#include <QPainter>
#include <QResizeEvent>
#include <QTimeString>
#include <QTimer>

static QColor normalBgColor(0,0,0);
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
    if (dirtyModel)
        rebuildCache();
    // Grumble.
    bool selected = (cDay == selectedDate());
    bool allday = false;
    bool timed = false;
    if (paintCache.contains(cDay)) {
        DayPaintCache *dpc = paintCache[cDay];
        allday = dpc->allDay;
        timed = dpc->timed;
    }

    p->save();
    if (timed || allday) {
        QFont f = p->font();
        f.setWeight(QFont::Bold);
        p->setFont(f);
    }
    if (selected) {
        p->fillRect(cr, palette().highlight());
        p->setPen(palette().highlightedText().color());
    } else {
        if (allday)
            p->fillRect(cr, normalBgColor);
        if (cDay.dayOfWeek() == Qt::Sunday || cDay.dayOfWeek() == Qt::Saturday)
            p->setPen(Qt::red); //xxx
    }
    if (monthShown() != cDay.month())
        p->setPen(palette().brush(QPalette::Disabled, QPalette::Text).color());
    p->drawText(cr, Qt::AlignCenter, QString::number(cDay.day()));
    p->restore();
}

MonthView::MonthView(QWidget *parent, const QCategoryFilter& c, QSet<QPimSource> set)
    : QCalendarWidget(parent), line_height(5)
{
    setObjectName("monthview");

    setVerticalHeaderFormat(NoVerticalHeader);
    setFirstDayOfWeek( Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday );

    QDate start = QDate::currentDate();
    start.setYMD(start.year(), start.month(), 1);
    QDate end = start.addDays(start.daysInMonth() - 1);

    model = new QOccurrenceModel(QDateTime(start, QTime(0, 0, 0)), QDateTime(end.addDays(1), QTime(0, 0)), this);
    if (set.count() > 0)
        model->setVisibleSources(set);
    model->setCategoryFilter(c);

    connect(model, SIGNAL(modelReset()), this, SLOT(rebuildCacheSoon()));
    connect(this, SIGNAL(currentPageChanged(int,int)), this, SLOT(updateModelRange(int,int)));

    // Since we don't know if we'll get a model reset from the model
    // at startup, force a timer
    dirtyTimer = new QTimer();
    dirtyTimer->setSingleShot(true);
    dirtyTimer->setInterval(0);
    connect(dirtyTimer, SIGNAL(timeout()), this, SLOT(rebuildCache()));

    rebuildCacheSoon();

    // XXX find the QCalendarView class so we can handle Key_Back properly :/
    // [this comes from qtopiaapplication.cpp]
    QWidget *table = findChild<QWidget*>("qt_calendar_calendarview");
    if (table)
        table->installEventFilter(this);
}

MonthView::~MonthView()
{
}

void MonthView::rebuildCacheNow()
{
    dirtyModel = true;
    rebuildCache();
}

void MonthView::rebuildCacheSoon()
{
    dirtyModel = true;
    dirtyTimer->start();
    update();
}

void MonthView::rebuildCache() const
{
    if (dirtyModel) {
        dirtyModel = false;
        // Clear the old cache
        qDeleteAll(paintCache);
        paintCache.clear();

        for (int i = 0; i < model->rowCount(); ++i) {
            // get just the data needed for drawing.
            QDateTime f = model->data(model->index(i, QAppointmentModel::Start), Qt::EditRole).toDateTime();
            QDateTime t = model->data(model->index(i, QAppointmentModel::End), Qt::EditRole).toDateTime();
            bool isAllDay = model->data(model->index(i, QAppointmentModel::AllDay), Qt::EditRole).toBool();

            if (!foundNColor) {
                normalBgColor = palette().button().color();
                foundNColor = true;
            }

            bool normalAllDay = false;
            int startPos = 0;
            int endPos = 24;
            if (isAllDay) {
                normalAllDay = true;
            } else {
                startPos =  f.time().hour();
                endPos = t.time().hour();
            }

            for (QDate i = f.date(); i <= t.date(); i = i.addDays(1)) {

                // get item.
                DayPaintCache *dpc;
                if (!paintCache.contains(i)) {
                    dpc = new DayPaintCache();
                    paintCache.insert(i, dpc);
                } else {
                    dpc = paintCache[i];
                }

                if (normalAllDay)
                    dpc->allDay = true;
                else {
                    // Weed out things that end at midnight (e.g should be previous day)
                    if (t != QDateTime(t.date()) || i != t.date()) {
                        dpc->timed = true;
                    }
                }
            }
        }
    }
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

void MonthView::setVisibleSources( QSet<QPimSource> set)
{
    model->setVisibleSources(set);
}

bool MonthView::eventFilter(QObject *o, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Back) {
            emit closeView();
            ke->accept();
            return true;
        }
    }
    return QCalendarWidget::eventFilter(o, e);
}

