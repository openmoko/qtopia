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

#ifndef ANIMATOR_P
#define ANIMATOR_P

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QtGlobal>

class QPainter;
class QRectF;
class QGraphicsRectItem;
class Renderer;
class SelectedItem;
class GridItem;
class QPixmap;

class Animator
{
public:

    virtual ~Animator() {}

    virtual void animate(QPainter *,SelectedItem *,qreal percent) = 0;

    virtual void initFromGridItem(GridItem *);

protected:

    void draw(QPainter *,SelectedItem *,int w,int h);
    void draw(QPainter *,const QPixmap &,QGraphicsRectItem *,int w,int h);

private:

    void draw(QPainter *,Renderer *,QGraphicsRectItem *,int w,int h);

    QRectF renderingBounds(QGraphicsRectItem *item,int w,int h);
};

#endif
