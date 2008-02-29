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

#ifndef _QTOPIA_MEDIALIBRARY_MEDIA_H
#define _QTOPIA_MEDIALIBRARY_MEDIA_H

#include <qtopiaipcmarshal.h>

// Media Server definitions XXX: move out to private
#define QTOPIA_MEDIASERVER_CHANNEL  "MediaServer"
#define QTOPIA_MEDIALIBRARY_CHANNEL "QPE/MediaLibrary/%1"
#define QT_MEDIASERVER_CHANNEL      "QPE/MediaServer"

namespace QtopiaMedia
{
enum State { Playing, Paused, Stopped, Buffering, Error};
}

Q_DECLARE_USER_METATYPE_ENUM(QtopiaMedia::State);

#endif  // _QTOPIA_MEDIALIBRARY_MEDIA_H

