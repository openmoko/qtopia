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


#ifndef NAVIGATIONBAR_P_H
#define NAVIGATIONBAR_P_H

#include <QWidget>

class QToolButton;


class NavigationBar : public QWidget
{
    Q_OBJECT

public:

    NavigationBar( QWidget* parent = 0);

    void triggerBackwards();

    void triggerForwards();

public slots:

    void setBackwardsEnabled(bool);

    void setForwardsEnabled(bool);

    void labelsChanged(const QString &previous,const QString &next);

signals:

    // Signal that is emitted when the left arrow button is clicked.
    void backwards();

    // Signal that is emitted when the right arrow button is clicked.
    void forwards();

private:

    // Called by ctor.
    void init();

    QToolButton *createButton(Qt::ArrowType);

    // The left button can be used as a backwards navigator.
    QToolButton *leftBn;

    // The right button can be used as a forwards navigator.
    QToolButton *rightBn;
};

#endif
