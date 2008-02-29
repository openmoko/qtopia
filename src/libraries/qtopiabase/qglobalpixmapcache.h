/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef QGLOBALPIXMAPCACHE_H
#define QGLOBALPIXMAPCACHE_H

#include <QStringList>
#include <QPixmap>

#include <qtopiaglobal.h>

#define QGLOBAL_PIXMAP_CACHE_LIMIT 1048576     // 1 Mb

class QTOPIABASE_EXPORT QGlobalPixmapCache
{
public:
    static bool find( const QString &key, QPixmap &pixmap );
    static bool insert( const QString &key, const QPixmap &pixmap );
    static void remove( const QString &key );
};


#endif //QGLOBALPIXMAPCACHE_H
