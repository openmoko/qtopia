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

#include "animatorfactory_p.h"
#include "animator_p.h"

#include <QString>
#include "bouncer_p.h"
#include "zoomer_p.h"
#include "shearer_p.h"
#include "rotator_p.h"
#include "radialbackground_p.h"


/*!
  \internal
  \class AnimatorFactory

  \brief Creates Animator objects, based on a textual description.
*/


/*!
  \internal
  \fn Animator *AnimatorFactory::getAnimator(const QString &description)
   Creates and returns an Animator which corresponds to the given description,
   or 0 if no Animator class corresponds to that description. Note that it
   is the calling code's responsibility to delete the returned object.
*/
Animator *AnimatorFactory::getAnimator(const QString &description)
{
    if ( description == Bouncer::getDescription() ) {
        return new Bouncer;
    }
    if ( description == Zoomer::getDescription() ) {
        return new Zoomer;
    }
    if ( description == Shearer::getDescription() ) {
        return new Shearer;
    }
    if ( description == Rotator::getDescription() ) {
        return new Rotator;
    }
    if ( description == RadialBackground::getDescription() ) {
        return new RadialBackground;
    }

    // Couldn't find an Animator class for the given description.
    return 0;
}
