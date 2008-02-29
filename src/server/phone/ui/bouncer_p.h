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

#ifndef BOUNCER_P_H
#define BOUNCER_P_H

#include "animator_p.h"

#include <QString>

class Painter;
class SelectedItem;


class Bouncer : public Animator
{
public:

    explicit Bouncer(qreal minVariation = DEFAULT_MIN_VARIATION,qreal maxVariation = DEFAULT_MAX_VARIATION,
                     int frameMax = 50);

    void animate(QPainter *,SelectedItem *,qreal percent);

    static const QString &getDescription() { return description; }

protected:

    static const qreal DEFAULT_MIN_VARIATION;
    static const qreal DEFAULT_MAX_VARIATION;

    // Speed factor to be passed to the oscillator. For example, if SPEED_FACTOR is 2.0,
    // the oscillator will calculate sin(2x), and therefore the oscillations will occur
    // twice as quickly. TODO: make this configurable.
    static const qreal SPEED_FACTOR;

private:

    // Description which can be used by a factory to identify this class.
    static const QString description;

    qreal minVariation;
    qreal maxVariation;
    int frameMax;
};

#endif
