/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef RADIALBACKGROUND_P_H
#define RADIALBACKGROUND_P_H

#include "animator_p.h"

#include <QString>

class Painter;
class SelectedItem;
class QColor;


class RadialBackground : public Animator
{
public:

    void animate(QPainter *,SelectedItem *,qreal percent);

    static const QString &getDescription() { return description; }

private:

    static const QString description;

    static const QColor color;
};

#endif

