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

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>

/*!
    \class QSerialIODeviceMultiplexerPluginInterface
    \brief The QSerialIODeviceMultiplexerPluginInterface class defines the interface to multiplexing plugins.
    \ingroup communication

    The QSerialIODeviceMultiplexerPluginInterface class defines the interface
    to multiplexer plugins.  Plugins will typically inherit from
    QSerialIODeviceMultiplexerPlugin rather than this class.

    \sa QSerialIODeviceMultiplexerPlugin, QSerialIODeviceMultiplexer
*/

/*!
    \fn bool QSerialIODeviceMultiplexerPluginInterface::forceGsm0710Stop()

    Returns true if the caller should force an existing GSM 07.10 session
    to stop before calling detect().  The default implementation returns
    false.  This is typcially needed for plugins that issue the AT+CMUX
    command in their detect() function.
*/

/*!
    \fn bool QSerialIODeviceMultiplexerPluginInterface::detect( QSerialIODevice *device )

    Issue commands to detect if this plug-in can provide multiplexing for
    \a device.  Returns true if multiplexing is possible, or false if not.
    The plug-in can use QSerialIODeviceMultiplexer::chat() to simplify the
    issuing of commands.

    \sa QSerialIODeviceMultiplexer::chat()
*/

/*!
    \fn QSerialIODeviceMultiplexer *QSerialIODeviceMultiplexerPluginInterface::create( QSerialIODevice *device )

    Create a multiplexer handler around \a device.  This will be called
    only if detect() returns true.
*/

/*!
    \class QSerialIODeviceMultiplexerPlugin
    \brief The QSerialIODeviceMultiplexerPlugin class defines a base class for implementing multiplexing plugins.
    \ingroup communication

    The QSerialIODeviceMultiplexerPlugin class defines a base class for
    implementing multiplexer plugins.  The plug-in should override detect()
    and create().

    \sa QSerialIODeviceMultiplexerPluginInterface, QSerialIODeviceMultiplexer
*/

/*!
    Create a multiplexer plug-in instance and attach it to \a parent.
*/
QSerialIODeviceMultiplexerPlugin::QSerialIODeviceMultiplexerPlugin
            ( QObject *parent )
    : QObject( parent )
{
}

/*!
    Destruct a multiplexer plug-in instance.
*/
QSerialIODeviceMultiplexerPlugin::~QSerialIODeviceMultiplexerPlugin()
{
}

/*!
    Get the list of keys for this plugin.
*/
QStringList QSerialIODeviceMultiplexerPlugin::keys() const
{
    QStringList list;
    return list << "QSerialIODeviceMultiplexerPluginInterface";
}

/*!
    Returns true if the caller should force an existing GSM 07.10 session
    to stop before calling detect().  The default implementation returns
    false.  This is typcially needed for plugins that issue the AT+CMUX
    command in their detect() function.
*/
bool QSerialIODeviceMultiplexerPlugin::forceGsm0710Stop()
{
    return false;
}

/*!
    Issue commands to detect if this plug-in can provide multiplexing for
    \a device.  Returns true if multiplexing is possible, or false if not.
    The plug-in can use QSerialIODeviceMultiplexer::chat() to simplify the
    issuing of commands.

    \sa QSerialIODeviceMultiplexer::chat()
*/
bool QSerialIODeviceMultiplexerPlugin::detect( QSerialIODevice * )
{
    return false;
}

/*!
    Create a multiplexer handler around \a device.  This will be called
    only if detect() returns true.
*/
QSerialIODeviceMultiplexer *QSerialIODeviceMultiplexerPlugin::create
            ( QSerialIODevice * )
{
    return 0;
}
