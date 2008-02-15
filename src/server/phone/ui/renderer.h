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

#ifndef RENDERER_H
#define RENDERER_H

#include <QString>

class QPainter;
class QRectF;

class Renderer
{
public:
    virtual ~Renderer() {}
    virtual bool load(const QString &filename) = 0;
    virtual void render(QPainter *painter, const QRectF &bounds) = 0;

    static Renderer *rendererFor(const QString &filename);
};

#endif

