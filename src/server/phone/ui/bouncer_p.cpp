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

#include "bouncer_p.h"
#include "oscillator_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>

/*!
  \internal
  \class Bouncer

  \brief A class which knows how to animate SelectedItem objects within
  the PhoneLauncherView.
  \mainclass The effect is that of the object shaking. Note
  that this operation is fairly expensive, since Bouncer has been
  immplemented as a bounded sin wave.
*/


const QString Bouncer::description("Bounce");
const qreal Bouncer::DEFAULT_MIN_VARIATION = -0.2;
const qreal Bouncer::DEFAULT_MAX_VARIATION = 0.3;
const qreal Bouncer::SPEED_FACTOR = 0.2;


/*!
  \internal
  \fn Bouncer::Bouncer(qreal minVariation,qreal maxVariation,int frameMax)
  \a minVariation: Percent (+/- 0-1) of the object's normal dimensions, this
  being the amount that the object should shrink down to. For example, if
  the SelectedItem's image has a width and height of 50 pixels and the
  minimum variation is -0.2, the object can be reduced by 20%, and so will
  'bounce down' to 40 pixels.
  \a maxVariation: Percent (+/- 0-1) of the object's normal dimensions, this
  being the amount that the object should increase up to. For example, if
  the SelectedItem's image has a width and height of 50 pixels and the
  minimum variation is 0.5, the object can be reduced by 50%, and so will
  'bounce up' to 75 pixels.
  \a frameMax Maximum number of frames in the timeline controlling the
  animation.
*/
Bouncer::Bouncer(qreal _minVariation,qreal _maxVariation,
                 int _frameMax)
    : minVariation(_minVariation)
    , maxVariation(_maxVariation)
    , frameMax(_frameMax)
{
}

/*!
  \internal
  \fn static const QString &Bouncer::getDescription()
  Returns description of this class, for configuration purposes.
*/

/*!
  \internal
  \fn void Bouncer::animate(QPainter *painter,SelectedItem *item,qreal percent)
*/
void Bouncer::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    GridItem *currentItem = item->getCurrent();
    if ( !currentItem ) {
        return;
    }
    int imageSize = currentItem->getSelectedImageSize();

    // Create an oscillator which will produce a sin wave whose lower bound is the
    // first parameter (i.e. the lowest dimension the image will take, and whose
    // upper bound is the second parameter (i.e. the highest dimension the image
    // will take).
    Oscillator oscillator(imageSize + minVariation * imageSize,
                          imageSize + maxVariation * imageSize,
                          frameMax, // upper bound of x
                          SPEED_FACTOR);

    // Ask the oscillator to produce a suitable width/height value for this stage
    // of the animation.
    int y = static_cast<int>(oscillator(percent*frameMax));

    draw(painter,item,y,y);
}
