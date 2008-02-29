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
    \fn QContentPlugin::installContent( const QString &fileName, QContent *content )

    Populates \a content with data from the file with the file name \a fileName.  Returns true on success.

    Installation is only performed when the content is first identified by the content system, if the file changes
    after installation updateContent() will be called to ensure the content data is up to date.
*/

/*!
    Refreshes the content data of \a content following a change to the file it references.

    Returns true if the content data was out of date, false otherwise.
*/
bool QContentPlugin::updateContent( QContent *content )
{
    Q_UNUSED( content );

    return false;
}

/*!
    \class QContentFactory

    Creates QContentHandlers.

  \ingroup content
*/

Q_GLOBAL_STATIC( ContentPluginManager, pluginManager );

/*!
    Populates \a content with data from the file with the file name \a fileName if an appropriate plug-in exists
    to handle the file.  Returns true if a plug-in is found to extract the content data.

    Installation is only performed when the content is first identified by the content system, if the file changes
    after installation updateContent() will be called to ensure the content data is up to date.
*/
bool QContentFactory::installContent( const QString &fileName, QContent *content )
{
    QList< QContentPlugin * > plugins = pluginManager()->findPlugins( fileName );

    foreach( QContentPlugin *p, plugins )
        if( p->installContent( fileName, content ) )
            return true;

    return false;
}

/*!
    Refreshes the content data of \a content following a change to the file it references if an appropriate plug-in exists
    to handle the file.  Returns true if a plug-in is found to update the content data, and the data is out of date.
*/
bool QContentFactory::updateContent( QContent *content )
{
    QList< QContentPlugin * > plugins = pluginManager()->findPlugins( content->file() );

    foreach( QContentPlugin *p, plugins )
        if( p->updateContent( content ) )
            return true;

    return false;
}
