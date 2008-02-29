/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3PAINTER_H
#define Q3PAINTER_H

#include <QtGui/qpainter.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3Painter : public QPainter
{
public:
    Q3Painter() : QPainter() { }
    Q3Painter(QPaintDevice *pdev) : QPainter(pdev) { }

    inline void drawRect(const QRect &rect);
    inline void drawRect(int x1, int y1, int w, int h)
    { drawRect(QRect(x1, y1, w, h)); }

    inline void drawRoundRect(const QRect &r, int xround = 25, int yround = 25);
    inline void drawRoundRect(int x, int y, int w, int h, int xround = 25, int yround = 25)
    { drawRoundRect(QRect(x, y, w, h), xround, yround); }

    inline void drawEllipse(const QRect &r);
    inline void drawEllipse(int x, int y, int w, int h)
    { drawEllipse(QRect(x, y, w, h)); }

    inline void drawArc(const QRect &r, int a, int alen);
    inline void drawArc(int x, int y, int w, int h, int a, int alen)
    { drawArc(QRect(x, y, w, h), a, alen); }

    inline void drawPie(const QRect &r, int a, int alen);
    inline void drawPie(int x, int y, int w, int h, int a, int alen)
    { drawPie(QRect(x, y, w, h), a, alen); }

    inline void drawChord(const QRect &r, int a, int alen);
    inline void drawChord(int x, int y, int w, int h, int a, int alen)
    { drawChord(QRect(x, y, w, h), a, alen); }

private:
    QRect adjustedRectangle(const QRect &r);
};

void Q_COMPAT_EXPORT Q3Painter::drawRect(const QRect &r)
{
    QPainter::drawRect(adjustedRectangle(r));
}

void Q_COMPAT_EXPORT Q3Painter::drawEllipse(const QRect &r)
{
    QPainter::drawEllipse(adjustedRectangle(r));
}

void Q_COMPAT_EXPORT Q3Painter::drawRoundRect(const QRect &r, int xrnd, int yrnd)
{
    QPainter::drawRoundRect(adjustedRectangle(r), xrnd, yrnd);
}

void Q_COMPAT_EXPORT Q3Painter::drawArc(const QRect &r, int angle, int arcLength)
{
    QPainter::drawArc(adjustedRectangle(r), angle, arcLength);
}

void Q_COMPAT_EXPORT Q3Painter::drawPie(const QRect &r, int angle, int arcLength)
{
    QPainter::drawPie(adjustedRectangle(r), angle, arcLength);
}

void Q_COMPAT_EXPORT Q3Painter::drawChord(const QRect &r, int angle, int arcLength)
{
    QPainter::drawChord(adjustedRectangle(r), angle, arcLength);
}

QT_END_HEADER

#endif // Q3PAINTER_H
