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
#ifndef ROTATOR_P_H
#define ROTATOR_P_H

#include "animator_p.h"

class Painter;
class SelectedItem;


class Rotator : public Animator
{
public:

    explicit Rotator(int numSpins = DEFAULT_SPINS);

    void animate(QPainter *,SelectedItem *item,qreal percent);

    // Returns description of this class for configuration purposes.
    static const QString &getDescription() { return description; }

private:

    static const QString description;

    static const int DEFAULT_SPINS = 3;

    int numSpins;
};

#endif
