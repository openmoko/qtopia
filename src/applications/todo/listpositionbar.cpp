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

#include "listpositionbar.h"
#include <QPainter>
#include <QIcon>
#include <QPaintEvent>

ListPositionBar::ListPositionBar(QWidget *parent)
 : QWidget(parent), mCurrent(0), mMax(0), mShowPrev(false), mShowNext(false)
{
    // We'd like around 6pt, but a minimum of 16px for legibility in CJK fonts etc
    // 16px @ 72dpi == 16pt
    // 16px @ 144 dpi == 8pt
    // 16px @ foo dpi == 16 / (dpi/72) == (16 * 72 / dpi)pt
    QFont f(font());
    qreal ptsize = 6;
    if (logicalDpiY() > 0 && (16.0 * 72)/logicalDpiY() > ptsize)
        ptsize = (16.0*72) / logicalDpiY();
    f.setPointSizeF(ptsize);
    setFont(f);

    mMetric = fontMetrics().height();
    setMinimumHeight(mMetric + 2);

    mLeftPixmap = QIcon(":icon/left").pixmap(mMetric);
    mRightPixmap = QIcon(":icon/right").pixmap(mMetric);
    setFocusPolicy(Qt::NoFocus);

    setMessage(tr("%1 of %2", "page 1 of 2"));
    setPosition(0,0);
}

ListPositionBar::~ListPositionBar()
{
}

void ListPositionBar::setMessage(const QString& format)
{
    mFormat = format;
    setPosition(mCurrent, mMax);
}

void ListPositionBar::setPosition(int current, int max)
{
    mMax = max;
    mCurrent = current;

    if (max <= 1 || current < 1) {
        hide();
    } else {
        show();
        parentWidget()->updateGeometry();
        update();
    }

    if (current > 1)
        mShowPrev = true;
    else
        mShowPrev = false;

    if (current < max)
        mShowNext = true;
    else
        mShowNext = false;

    mPosition = mFormat.arg(current).arg(max);
}

void ListPositionBar::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::LeftButton) {
        bool next = false;
        if (me->x() < width()/ 2) {
            if (layoutDirection() == Qt::RightToLeft)
                next = true;
        } else {
            if (layoutDirection() == Qt::LeftToRight)
                next = true;
        }

        if (next)
            emit nextPosition();
        else
            emit previousPosition();
    }
}

void ListPositionBar::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    QPainter p(this);

    p.fillRect(pe->rect(), palette().alternateBase());


    QRect pixmaprect(0, 2, mMetric,mMetric);
    if (layoutDirection() == Qt::RightToLeft) {
        if (mShowPrev)
            p.drawPixmap(pixmaprect.adjusted(rect().right() - mMetric - 1, 0, 0, 0), mRightPixmap);
        if (mShowNext)
            p.drawPixmap(pixmaprect.adjusted(1,0,0,0), mLeftPixmap);
    } else {
        if (mShowNext)
            p.drawPixmap(pixmaprect.adjusted(rect().right() - mMetric - 1, 0, 0, 0), mRightPixmap);
        if (mShowPrev)
            p.drawPixmap(pixmaprect.adjusted(1,0,0,0), mLeftPixmap);
    }

    p.drawText(rect(), Qt::AlignCenter, mPosition);
}

//===========================================================================

