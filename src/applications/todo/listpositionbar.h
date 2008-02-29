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

#ifndef LISTPOSITIONBAR_H
#define LISTPOSITIONBAR_H

#include <QWidget>
#include <QString>
#include <QPixmap>

class ListPositionBar : public QWidget
{
    Q_OBJECT;

public:
    ListPositionBar(QWidget *parent = 0);
    ~ListPositionBar();

    void setPosition(int current, int max);

    void setMessage(const QString& formatString);

protected:
    void paintEvent(QPaintEvent *pe);
    void mousePressEvent(QMouseEvent *me);

signals:
    void nextPosition();
    void previousPosition();

protected:
    QString mPosition;
    QString mFormat;
    int mMetric;
    int mCurrent;
    int mMax;
    bool mShowPrev;
    bool mShowNext;
    QPixmap mLeftPixmap;
    QPixmap mRightPixmap;
};

#endif // LISTPOSITIONBAR_H

