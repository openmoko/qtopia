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

#include "media.h"


/*!
    \enum QtopiaMedia::State

    This enum specifies the state of a media content.

    \value Playing The Media is currently playing.
    \value Paused The Media is currently paused.
    \value Stopped The Media is currently stopped.
    \value Buffering The Media is being buffered before playback.
    \value Error An error has occurred with the media
*/


Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaMedia::State);

