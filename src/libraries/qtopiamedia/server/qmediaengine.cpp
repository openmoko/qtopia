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

#include "qmediaengine.h"


/*!
    \class QMediaEngine
    \preliminary
    \brief The QMediaEngine class is a base class to be used in the creation of a Media Engine.

    The QMediaEngine is a base class for Media Engines within Qtopia's Media
    Server. To create a Media Engine capable of playing content with the
    system, QMediaEngine should be subclassed and the functions implemented.

    \sa QMediaEngineFactory
*/


/*!
    Destruct a QMediaEngine.
*/

QMediaEngine::~QMediaEngine()
{
}

/*!
    \fn void QMediaEngine::initialize()

    This function is called before any other function on the Media Engine, it
    is an opportunity to perform any initialisation required by the Media
    Engine.
*/

/*!
    \fn void QMediaEngine::start()

    This function is called to start the Media Engine. After the call to this
    function it is expected that the Media Engine is in a state such that any
    of its QMediaSessionBuilders are able to create sessions.

    \sa QMediaSessionBuilder
*/

/*!
    \fn void QMediaEngine::stop()

    This function is called to stop the Media Engine. After the call to stop
    the Media Engine will no longer actively participate in the Media Server.
    Most resources should be freed at this time. Including resources such as
    unix devices, files, threads and temporary storage related to the Media
    Engines state.
*/

/*!
    \fn void QMediaEngine::suspend()

    This function is called when the Media Engine is going to be suspended.
    Typically the Media Engine should only release resources that are cheaply
    reallocated. The Media Engine must give up access to any unix device in
    this call.

    The suspend() function can be called when another engine needs access to a
    device or optionally when a timeout is reached, the length of the timeout
    is provided by the QMediaEngineInformation class.

    \sa QMediaEngineInformation
*/

/*!
    \fn void QMediaEngine::resume()

    The resume() function is called only after a suspend() has been issued. The
    Engine should reallocate any resources freed during the suspend(). Both
    resume() and suspend() should be light calls not requiring significat time
    to execute.
*/

/*!
    \fn QMediaEngineInformation const* QMediaEngine::engineInformation();

    Return information about this Media Engine.

    \sa QMediaEngineInformation
*/

