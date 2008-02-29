/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef ANIMATOR_P
#define ANIMATOR_P


#include <QtGlobal>

class QPainter;
class QRectF;
class QGraphicsRectItem;
class QSvgRenderer;
class SelectedItem;
class QPixmap;


class Animator
{
public:

    virtual ~Animator() {}

    virtual void animate(QPainter *,SelectedItem *,qreal percent) = 0;

protected:

    void draw(QPainter *,SelectedItem *,int w,int h);

private:

    void draw(QPainter *,QSvgRenderer *,QGraphicsRectItem *,int w,int h);
    void draw(QPainter *,const QPixmap &,QGraphicsRectItem *,int w,int h);

    QRectF getRenderingBounds(QGraphicsRectItem *item,int w,int h);
};

#endif
