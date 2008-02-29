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

#include "contentserverinterface_p.h"
#include <qtopialog.h>

ContentServerInterface *ContentServerInterface::serverInstance = 0;
QList<QContentSet *> ContentServerInterface::linkSets;
QMutex ContentServerInterface::listMutex;

ContentServerInterface::ContentServerInterface( QObject *parent )
    : QThread( parent )
{
}

ContentServerInterface::~ContentServerInterface()
{
}

/*!
  \internal
  Boolean true if we are "in process" with the content server
  Assumes that the ContentServer instance is initialised at boot
  time - prior to that results are undefined.
*/
bool ContentServerInterface::inServerProcess()
{
    return ContentServerInterface::serverInstance != 0;
}

/*!
  \internal
  Return a pointer to the server instance.

  Note this method will not cause an instance to be created.  This will
  return NULL unless an implementing class has created an instance.
*/
ContentServerInterface *ContentServerInterface::server()
{
    return serverInstance;
}

/*!
  Return a reference to the internally maintained list of
  this process's ContentLinkSets
*/
QList<QContentSet *> &ContentServerInterface::sets()
{
    return linkSets;
}

/*!
  Add the QContentSet \a cls to the global (to this process)
  QContentSet list, so that it can receive updates.

    TODO:  Maybe the mutex is not required here, as the order of the
    sets in the list is not significant.  It is not required if the
    internal counts in the Qt implementation of QList are atomically
    incremented.  Serializing access to this list may impact on
    performance.
*/
void ContentServerInterface::addCLS( QContentSet *cls )
{
    qLog(DocAPI) << "ContentServerInterface::addCLS";
    QMutexLocker ml( &listMutex );
    if (!linkSets.contains(cls))
        linkSets.append( cls );
}

/*!
  \internal
  Add the QContentSet \a cls to the global (to this process)
  QContentSet list, so that it can receive updates.

  TODO -  see above re mutex
*/

void ContentServerInterface::removeCLS( QContentSet *cls )
{
    qLog(DocAPI) << "ContentServerInterface::removeCLS";
    QMutexLocker ml( &listMutex );
    linkSets.removeAll( cls );
}
