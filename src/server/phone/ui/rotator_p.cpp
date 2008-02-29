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
#include "rotator_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>

const QString Rotator::mDescription("Rotate");


Rotator::Rotator(int _numSpins)
    : numSpins(_numSpins)
{
}

void Rotator::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    painter->translate(item->rect().x() + item->rect().width()/2,item->rect().y() + item->rect().height()/2);
    painter->rotate(360.0 * percent * numSpins);
    painter->translate(-(item->rect().width()/2) - item->rect().x() ,-(item->rect().height()/2)-item->rect().y());

    int imageSize = item->current()->selectedImageSize();
    draw(painter,item,imageSize,imageSize);
}

