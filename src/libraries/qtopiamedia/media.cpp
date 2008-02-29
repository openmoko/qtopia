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

#include "media.h"


/*!
    \class QtopiaMedia
    \mainclass
    \brief The QtopiaMedia namespace provides a container for general media types.

    The QtopiaMedia namespace defines two enumerations to deal with state
    information and seek offsets.

    \sa QtopiaMedia::State, QtopiaMedia::Offset
*/

/*!
    \enum QtopiaMedia::State

    This enum specifies the state of a media content.

    \value Playing The Media is currently playing.
    \value Paused The Media is currently paused.
    \value Stopped The Media is currently stopped.
    \value Buffering The Media is being buffered before playback.
    \value Error An error has occurred with the media
*/

/*!
    \enum QtopiaMedia::Offset

    This enum specifies the offset from which a seek operation should occur.

    \value Beginning The Beginning of the content.
    \value Current The current position in Playing or Paused content.
    \value End The end of the content.
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaMedia::State);

