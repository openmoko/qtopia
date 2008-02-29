/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/qtimer.h>
#include <QtCore/qdatetime.h>
#include <QtGui/qwidget.h>
#include <QtGui/qtextedit.h>
#include <QtGui/private/qwidget_p.h>
#include <qdebug.h>

#include "qwidgetanimator_p.h"

static const int g_animation_steps = 14;
static const int g_animation_interval = 20;

// 1000 * (x/(1 + x*x) + 0.5) on interval [-1, 1]
static const int g_animate_function[] =
{
    0, 1, 5, 12, 23, 38, 58, 84, 116, 155, 199, 251, 307, 368,
    433, 500, 566, 631, 692, 748, 799, 844, 883, 915, 941, 961,
    976, 987, 994, 998, 1000
};
static const int g_animate_function_points = sizeof(g_animate_function)/sizeof(int);

static inline int animate(int start, int stop, int step, int steps)
{
    if (start == stop)
        return start;
    if (step == 0)
        return start;
    if (step == steps)
        return stop;

    int x = g_animate_function_points*step/(steps + 1);
    return start + g_animate_function[x]*(stop - start)/1000;
}

QWidgetAnimator::QWidgetAnimator(QObject *parent)
    : QObject(parent)
{
    m_time = new QTime();
    m_timer = new QTimer(this);
    m_timer->setInterval(g_animation_interval);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animationStep()));
}

QWidgetAnimator::~QWidgetAnimator()
{
    delete m_time;
}

void QWidgetAnimator::animate(QWidget *widget, const QRect &_final_geometry, bool animate)
{
    QRect final_geometry = _final_geometry;

    QRect r = widget->geometry();
    if (r.right() < 0 || r.bottom() < 0)
        r = QRect();

    if (r.isNull() || final_geometry.isNull())
        animate = false;

    AnimationMap::const_iterator it = m_animation_map.constFind(widget);
    if (it == m_animation_map.constEnd()) {
        if (r == final_geometry) {
            emit finished(widget);
            return;
        }
    } else {
        if ((*it).r2 == final_geometry)
            return;
    }

    if (animate) {
        AnimationItem item(widget, r, final_geometry);
        m_animation_map[widget] = item;
        if (!m_timer->isActive()) {
            m_timer->start();
            m_time->start();
        }
    } else {
        m_animation_map.remove(widget);
        if (m_animation_map.isEmpty())
            m_timer->stop();

        if (!final_geometry.isValid() && !widget->isWindow()) {
            // Make the wigdet go away by sending it to negative space
            QSize s = widget->size();
            final_geometry = QRect(-500 - s.width(), -500 - s.height(), s.width(), s.height());
        }
        widget->setGeometry(final_geometry);

        emit finished(widget);

        if (m_animation_map.isEmpty())
            emit finishedAll();

        return;
    }
}

void QWidgetAnimator::animationStep()
{
    int steps = (1 + m_time->restart())/g_animation_interval;
    AnimationMap::iterator it = m_animation_map.begin();
    while (it != m_animation_map.end()) {
        AnimationItem &item = *it;

        item.step = qMin(item.step + steps, g_animation_steps);

        int x = ::animate(item.r1.left(), item.r2.left(),
                            item.step, g_animation_steps);
        int y = ::animate(item.r1.top(), item.r2.top(),
                            item.step, g_animation_steps);
        int w = ::animate(item.r1.width(), item.r2.width(),
                            item.step, g_animation_steps);
        int h = ::animate(item.r1.height(), item.r2.height(),
                            item.step, g_animation_steps);

        item.widget->setGeometry(x, y, w, h);

        if (item.step == g_animation_steps) {
            emit finished(item.widget);
            AnimationMap::iterator tmp = it;
            ++it;
            m_animation_map.erase(tmp);
        } else {
            ++it;
        }
    }

    if (m_animation_map.isEmpty()) {
        m_timer->stop();
        emit finishedAll();
    }
}

bool QWidgetAnimator::animating() const
{
    return m_timer->isActive();
}

bool QWidgetAnimator::animating(QWidget *widget)
{
    return m_animation_map.contains(widget);
}
