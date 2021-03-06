/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include "parabola_p.h"

Parabola::Parabola(qreal maxX,qreal _peak)
    : peak(_peak)
{
    // Half the width of the parabola when x = 0
    halfX = maxX/2;
    a = -peak/(halfX * halfX);
}

qreal Parabola::operator()(qreal x)
{
    x -= halfX;
    qreal rc = a * x * x + peak;
    return rc;
}
