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

#include "qglobalpixmapcache.h"

#ifdef Q_WS_X11

bool QGlobalPixmapCache::find( const QString &key, QPixmap &pixmap )
{
    Q_UNUSED(key)
    Q_UNUSED(pixmap)
    return false;
}

bool QGlobalPixmapCache::insert( const QString &key, const QPixmap &pixmap )
{
    Q_UNUSED(key);
    Q_UNUSED(pixmap);
    return false;
}

void QGlobalPixmapCache::remove( const QString &key )
{
    Q_UNUSED(key);
}

#endif // Q_WS_X11
