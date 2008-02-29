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

#include "qmediaenginefactory.h"


/*!
    \class QMediaEngineFactory
    \preliminary
    \brief The QMediaEngineFactory class is a factory used by the Media Server to construct Media Engines loaded from libraries.

    The Media Server loads Qt style plugins tagged with "mediaengine". Each
    Media Engine in the Qtopia Media system is implemented as a Qt style
    plugin. The plugin must implement the QMediaEngineFactory inteface, as it
    used by the Media Server to ensure that the plugin is able to be used in
    the server. The Media Server will call the create() function when it needs
    to construct the Engine.

    \sa QMediaEngine
*/


/*!
    Destruct a QMediaEngineFactory
*/

QMediaEngineFactory::~QMediaEngineFactory()
{
}

/*!
    \fn QMediaEngine* QMediaEngineFactory::create()

    This function is called by the Media Server to create a instance of the
    Media Engine. The factory should construct a new instance of the Media
    Engine on the heap, it will later be deleted by the Media Server. This
    function will only be called once for the lifetime of the plugin.
*/




