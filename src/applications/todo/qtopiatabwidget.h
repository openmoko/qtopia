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

#ifndef QTOPIATABWIDGET_H
#define QTOPIATABWIDGET_H

#include <QTabWidget>
#include <QMap>

class QScrollArea;
class QDelayedScrollArea;
class QtopiaTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    
    QtopiaTabWidget(QWidget *parent = 0);
    ~QtopiaTabWidget();

    // not virtual in parent...
    int addTab(const QString &label);
    int addTab(const QIcon &icon, const QString &label);

signals:
    void prepareTab(int index, QScrollArea *parent);

private slots:
    void layoutTab(int);

private:
    QWidget *widget(int index) const;
    // hiding these functions.
    int addTab(QWidget *child, const QString &label);
    int addTab(QWidget *child, const QIcon &icon, const QString &label);
    int insertTab(int index, QWidget *widget, const QString &label);
    int insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label);

    QMap<int, QDelayedScrollArea *> unpreparedTabs;
};

#endif // QTOPIATABWIDGET_H
