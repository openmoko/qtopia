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

#include "qtopiatabwidget.h"
#include "qdelayedscrollarea.h"

#include <QList>
#include <QDebug>

#define DELAY_LOAD_TABS

QtopiaTabWidget::QtopiaTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
}

QtopiaTabWidget::~QtopiaTabWidget()
{
}

// implemented to hide them
int QtopiaTabWidget::addTab(QWidget *child, const QString &label)
{
    Q_UNUSED(child)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::addTab(QWidget *child, const QIcon &icon, const QString &label)
{
    Q_UNUSED(child)
    Q_UNUSED(icon)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::insertTab(int index, QWidget *widget, const QString &label)
{
    Q_UNUSED(index)
    Q_UNUSED(widget)
    Q_UNUSED(label)
    return -1;
}

int QtopiaTabWidget::insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label)
{
    Q_UNUSED(index)
    Q_UNUSED(widget)
    Q_UNUSED(icon)
    Q_UNUSED(label)
    return -1;
}

QWidget *QtopiaTabWidget::widget(int index) const
{
    Q_UNUSED(index)
    return 0;
}

int QtopiaTabWidget::addTab(const QString &label)
{
#ifdef DELAY_LOAD_TABS
    QDelayedScrollArea *sc = new QDelayedScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    sc->setIndex(QTabWidget::addTab(sc, label));
    unpreparedTabs.insert(sc->index(), sc);
    connect(sc, SIGNAL(aboutToShow(int)), this, SLOT(layoutTab(int)));
    return sc->index();
#else
    QScrollArea *sc = new QScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    int index = QTabWidget::addTab(sc, label);
    emit prepareTab(index, sc);
    sc->setFocusPolicy(Qt::NoFocus);
    return index;
#endif
}

int QtopiaTabWidget::addTab(const QIcon &icon, const QString &label)
{
#ifdef DELAY_LOAD_TABS
    QDelayedScrollArea *sc = new QDelayedScrollArea;
    sc->setWidgetResizable(true);
    sc->setIndex(QTabWidget::addTab(sc, icon, label));
    sc->setFrameStyle(QFrame::NoFrame);
    unpreparedTabs.insert(sc->index(), sc);
    connect(sc, SIGNAL(aboutToShow(int)), this, SLOT(layoutTab(int)));
    return sc->index();
#else
    QScrollArea *sc = new QScrollArea;
    sc->setWidgetResizable(true);
    sc->setFrameStyle(QFrame::NoFrame);
    int index = QTabWidget::addTab(sc, icon, label);
    emit prepareTab(index, sc);
    sc->setFocusPolicy(Qt::NoFocus);
    return index;
#endif
}

void QtopiaTabWidget::layoutTab(int index)
{
    if (unpreparedTabs.contains(index)) {
        QDelayedScrollArea *sc = unpreparedTabs.value(index);
        emit prepareTab(index, sc);
        sc->setFocusPolicy(Qt::NoFocus);
        unpreparedTabs.remove(index);
    }
}
