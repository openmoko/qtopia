/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGPLUGINMANAGER_P_H
#define QGPLUGINMANAGER_P_H

#ifndef QT_H
#include "qdict.h"
#include "qlibrary.h"
#include "quuid.h"
#include "qstringlist.h"
#include "qcom_p.h"
#endif // QT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_NO_COMPONENT

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
//Q_TEMPLATE_EXTERN template class Q_EXPORT QDict<QLibrary>;
// MOC_SKIP_END
#endif

class Q_EXPORT QGPluginManager
{
public:
    QGPluginManager( const QUuid& id, const QStringList& paths = QString::null, const QString &suffix = QString::null, bool cs = TRUE );
    ~QGPluginManager();

    void addLibraryPath( const QString& path );
    const QLibrary* library( const QString& feature ) const;
    QStringList featureList() const;

    bool autoUnload() const;
    void setAutoUnload( bool );

protected:
    bool enabled() const;
    bool addLibrary( QLibrary* plugin );

    QRESULT queryUnknownInterface(const QString& feature, QUnknownInterface** iface) const;

    QUuid interfaceId;
    QDict<QLibrary> plugDict;	    // Dict to match feature with library
    QDict<QLibrary> libDict;	    // Dict to match library file with library
    QStringList libList;

    uint casesens : 1;
    uint autounload : 1;
};

inline void QGPluginManager::setAutoUnload( bool unload )
{
    autounload = unload;
}

inline bool QGPluginManager::autoUnload() const
{
    return autounload;
}

#endif

#endif //QGPLUGINMANAGER_P_H
