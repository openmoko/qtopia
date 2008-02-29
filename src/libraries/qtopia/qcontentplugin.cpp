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

#include "qcontentplugin.h"
#include "contentpluginmanager_p.h"
#include "drmcontent_p.h"
#include <QtDebug>
#include <qcategorymanager.h>
#include <qmimetype.h>


/*!
    \class QContentPlugin

    The QContentPlugin class provides an interface to discover the attributes of a file that should
    be assigned to a new QContent.

  \ingroup content
*/

/*!
    Destroys a QContentPlugin.
*/
QContentPlugin::~QContentPlugin()
{
}

/*!
    \fn QContentPlugin::keys() const

    Returns a list of the file extensions the QContentPlugin instance can report on.
*/

/*!
    \fn QContentPlugin::installContent( const QString &filePath, QContent *content )

    Populates \a content with data from the file located at \a filePath.  Returns true on success.
*/

/*!
    \class QContentFactory

    Creates QContentHandlers.

  \ingroup content
*/

Q_GLOBAL_STATIC( ContentPluginManager, pluginManager );

/*!
    Populates \a content with data from the file located at \a filePath if an appropriate plug-in exists
    to handle the file.  Returns true on success.
*/
bool QContentFactory::installContent( const QString &filePath, QContent *content )
{
    QList< QContentPlugin * > plugins = pluginManager()->findPlugins( filePath );

    foreach( QContentPlugin *p, plugins )
        if( p->installContent( filePath, content ) )
            return true;

    return false;
}
