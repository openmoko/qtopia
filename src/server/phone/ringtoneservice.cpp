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

#include "ringtoneservice.h"

/*!
    \service RingtoneService Ringtone
    \brief Provides the Qtopia Ringtone service.

    The \i Ringtone service enables applications to request ringtones playback to 
    be started or stopped.
*/

/*!
    \fn RingtoneService::RingtoneService( QObject *parent )
    \internal
*/

/*!
    \internal
*/
RingtoneService::~RingtoneService()
{
}

/*!
    \fn void RingtoneService::startMessageRingtone()

    Requests that the ringtone associated with message arrival should start playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::startMessageRingtone()}.
*/

/*!
    \fn void RingtoneService::stopMessageRingtone()

    Requests that the ringtone associated with message arrival should stop playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::stopMessageRingtone()}.
*/

/*!
    \fn void RingtoneService::startRingtone( const QString& fileName )

    Requests that the ringtone specified by \a fileName should start playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::startRingtone(QString)}.
*/

/*!
    \fn void RingtoneService::stopRingtone( const QString& fileName )

    Requests that the ringtone specified by \a fileName should stop playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::stopRingtone(QString)}.
*/
