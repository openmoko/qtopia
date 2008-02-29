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

#ifndef ZOOMER_P_H
#define ZOOMER_P_H

#include "animator_p.h"

#include <QString>

class Painter;
class SelectedItem;


class Zoomer : public Animator
{
public:

    //void animate(QPainter *,QSvgRenderer *,int imageSize,QGraphicsRectItem *,qreal percent);
    void animate(QPainter *,SelectedItem *,qreal percent);

    // Returns description of this class for configuration purposes.
    static const QString &getDescription() { return description; }

private:

    static const QString description;

    // Percentag by which the image grows during the zoom.
    static const qreal growthFactor = 0.8;

    static const qreal stop1 = 0.6;
    static const qreal stop2 = 0.8;
};

#endif
