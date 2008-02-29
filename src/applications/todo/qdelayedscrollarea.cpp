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

#include "qdelayedscrollarea.h"
#include <QEvent>
#include <QDebug>
#include <QLayout>

QDelayedScrollArea::QDelayedScrollArea(int index, QWidget *parent) : QScrollArea(parent), i(index)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    setFrameStyle(QFrame::NoFrame);
    viewport()->installEventFilter(this);
}

QDelayedScrollArea::QDelayedScrollArea(QWidget *parent) : QScrollArea(parent), i(-1) {}

QDelayedScrollArea::~QDelayedScrollArea() {}

void QDelayedScrollArea::adjustWidget(int width)
{
    QWidget *w = widget();
    if (w) {
        w->setFixedWidth(width);
        QLayout *l = w->layout();
        if (l) {
            if (l->hasHeightForWidth())
                w->setMinimumHeight(l->heightForWidth(width));
        } else {
            if (w->sizePolicy().hasHeightForWidth())
                w->setMinimumHeight(w->heightForWidth(width));
        }
    }
}

bool QDelayedScrollArea::eventFilter( QObject *receiver, QEvent *event )
{
    if (widget() && receiver == viewport() && event->type() == QEvent::Resize )
        adjustWidget(viewport()->width());
    if (widget() == receiver && event->type() == QEvent::LayoutRequest)
        adjustWidget(viewport()->width());
    return false;
}

void QDelayedScrollArea::showEvent(QShowEvent *event)
{
    emit aboutToShow(i);
    adjustWidget(viewport()->width());

    QScrollArea::showEvent(event);
}

void QDelayedScrollArea::resizeEvent(QResizeEvent *re)
{
    adjustWidget(viewport()->width());
    QScrollArea::resizeEvent(re);
}

