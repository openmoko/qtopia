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

#include "radioservice.h"
#include "radioplayer.h"

/*!
    \service RadioService Radio
    \brief Provides the Qtopia Radio service.

    The \i Radio service enables applications to control the radio
    tuner program.
*/

/*!
    \internal
*/
RadioService::RadioService( RadioPlayer *parent )
    : QtopiaAbstractService( "Radio", parent )
{
    publishAll();
    this->parent = parent;
}

/*!
    \internal
*/
RadioService::~RadioService()
{
}

/*!
    Turn on the radio mute.
*/
void RadioService::mute()
{
    parent->setMute( true );
}

/*!
    Turn off the radio mute.
*/
void RadioService::unmute()
{
    parent->setMute( false );
}

/*!
    Set the radio to tune in a specific station on \a band and \a frequency.
*/
void RadioService::setStation( const QString& band, qlonglong frequency )
{
    parent->setStation( band, frequency );
}
