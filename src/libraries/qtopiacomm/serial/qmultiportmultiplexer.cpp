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

#include <qmultiportmultiplexer.h>
#include <qmap.h>
#include <qset.h>

/*!
    \class QMultiPortMultiplexer
    \mainclass
    \brief The QMultiPortMultiplexer class provides multiplexing across several serial ports
    \ingroup telephony::serial

    Use this class instead of QGsm0710Multiplexer if the operating system kernel
    has built-in support for multiplexing.

    Instances of QMultiPortMultiplexer are created by multiplexer plug-ins.
    See the \l{Tutorial: Writing a Multiplexer Plug-in} for more information
    on how to write a multiplexer plug-in that uses QMultiPortMultiplexer.

    \sa QSerialIODeviceMultiplexer, QGsm0710Multiplexer, QSerialIODeviceMultiplexerPlugin
*/

class QMultiPortMultiplexerPrivate
{
public:
    ~QMultiPortMultiplexerPrivate();

    QMap<QString, QSerialIODevice *> channels;
    QSet<QSerialIODevice *> devices;
};

QMultiPortMultiplexerPrivate::~QMultiPortMultiplexerPrivate()
{
    QSet<QSerialIODevice *>::ConstIterator iter;
    for ( iter = devices.begin(); iter != devices.end(); ++iter ) {
        delete *iter;
    }
}

/*!
    Construct a new multi-port multiplexer object and attach it to \a parent.
    The \c{primary} channel will be set to \a device.  Further channels
    can be added by calling addChannel().  Ownership of \a device will
    pass to this object; it will be deleted when this object is deleted.
*/
QMultiPortMultiplexer::QMultiPortMultiplexer
        ( QSerialIODevice *device, QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QMultiPortMultiplexerPrivate();
    addChannel( "primary", device );
}

/*!
    Destruct this multi-port multiplexer object.
*/
QMultiPortMultiplexer::~QMultiPortMultiplexer()
{
    delete d;
}

/*!
    Add a new channel to this multiplexer, with requests for \a name
    being redirected to \a device.  Returns false if \a name already
    exists.

    Ownership of \a device will pass to this object; it will be
    deleted when this object is deleted.  A single device can be added
    for multiple channels (e.g. \c{data} and \c{datasetup}).  This
    object will ensure that such devices will be deleted only once.
*/
bool QMultiPortMultiplexer::addChannel
        ( const QString& name, QSerialIODevice *device )
{
    if ( d->channels.contains( name ) )
        return false;
    d->channels.insert( name, device );
    if ( !d->devices.contains( device ) )
        d->devices.insert( device );
    return true;
}

/*!
    \reimp
*/
QSerialIODevice *QMultiPortMultiplexer::channel( const QString& name )
{
    if ( d->channels.contains( name ) ) {
        return d->channels.value( name );
    } else if ( name == "secondary" && d->channels.contains( "primary" ) ) {
        // No explicit "secondary" channel, so use "primary".
        return d->channels.value( "primary" );
    } else if ( name == "datasetup" && d->channels.contains( "data" ) ) {
        // No explicit "datasetup" channel, so use "data".
        return d->channels.value( "data" );
    } else if ( name.startsWith( "aux" ) && d->channels.contains( "aux" ) ) {
        // No explicit "aux*" channel, so use "aux".
        return d->channels.value( "aux" );
    } else {
        return 0;
    }
}
