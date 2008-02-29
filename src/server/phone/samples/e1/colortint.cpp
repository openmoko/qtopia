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

#include "colortint.h"
#include <QColor>

QColor ColorTint::darken(const QColor &color, int percent)
{
    QColor rv( (color.red() * percent) / 100,
               (color.green() * percent) / 100,
               (color.blue() * percent) / 100);

    return rv;
}

QColor ColorTint::lighten(const QColor &color, int percent)
{
    QColor rv( color.red() + ((255 - color.red()) * percent) / 100,
              color.green() + ((255 - color.green()) * percent) / 100,
              color.blue() + ((255 - color.blue()) * percent) / 100);

    return rv;
}

