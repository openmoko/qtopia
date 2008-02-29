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

#include <qserialiodevicemultiplexer.h>
#include <qgsm0710multiplexer.h>
#include <qserialiodevicemultiplexerplugin.h>
#include <qserialport.h>
#include <qtopiacomm/private/qpassthroughserialiodevice_p.h>
#include <qtopialog.h>
#include <qpluginmanager.h>
#include <qtopianamespace.h>
#include <custom.h>
#include <qmap.h>
#include <alloca.h>

/*!
    \class QSerialIODeviceMultiplexer
    \brief The QSerialIODeviceMultiplexer class provides a base class for serial device multiplexing
    \ingroup io
    \ingroup telephony::serial

    The QSerialIODeviceMultiplexer class provides a base class for serial
    device multiplexers on AT-based modems (e.g. GSM 07.10 multiplexing).

    A multiplexer manages one or more channels, identified by their
    names.  The standard names are:

    \list
    \o \c{primary} Primary AT command channel.
    \o \c{secondary} Secondary AT command channel for slow AT commands.
                     Should return the same as \c{primary} if there
                     is no explicit secondary channel on the modem.
    \o \c{data} Data channel for GPRS and similar data sessions.
                Should return null if there is no explicit data channel
                on the modem.
    \o \c{datasetup} Channel for data call setup commands.  Should return
                     null if \c{data} is null.
    \endlist

    Sometimes these channels may overlap.  For example, \c{datasetup}
    may be the same as \c{data} if data call setup should be sent on the
    same channel as the data itself; or it may be the same as \c{primary}
    if data call setup should be sent on the primary AT command channel.

    If a channel is not supported, the channel() method returns null.
    The \c{primary} and \c{secondary} channels must always be supported,
    even if they are the same.

    Auxillary channels can be created and released dynamcially.  The
    channel name should begin with \c{aux}.  The rest of the name gives
    an indication as to the type of auxillary data: \c{auxdata}, \c{auxvideo},
    etc.  The multiplexer may return a pre-defined channel (e.g. \c{data})
    if the number of channels on the multiplexer is limited.  The
    isOpen() method can be used to determine if the channel is already
    in use, or is free for auxillary traffic.

    \sa QGsm0710Multiplexer, QNullSerialIODeviceMultiplexer
*/

/*!
    Construct a new multiplexer attached to \a parent.
*/
QSerialIODeviceMultiplexer::QSerialIODeviceMultiplexer( QObject *parent )
    : QObject( parent )
{
    // Nothing to do here.
}

/*!
    Destruct a multiplexer.
*/
QSerialIODeviceMultiplexer::~QSerialIODeviceMultiplexer()
{
    // Nothing to do here.
}

/*!
    \fn QSerialIODevice *QSerialIODeviceMultiplexer::channel( const QString& name )

    Get the serial device corresponding the channel \a name.  If the
    indicated channel does not exist, it should be created if possible.
    Some multiplexers may not be able to create all types of channels,
    and will return null for unsupported channels.
*/

// Read a line of text from a serial device.
static QString readLine( QSerialIODevice *device )
{
    char line[256];
    int posn = 0;
    char ch;
    for(;;)
    {
        if ( !device->waitForReadyRead( 5000 ) ) {
            qLog(Mux) << "*** mux setup timed out ***";
            return QString();
        }
        if ( device->getChar( &ch ) ) {
            if ( ch == 0x0D || ch == 0x0A ) {
                break;
            } else if ( posn < (int)( sizeof(line) - 1 ) ) {
                line[posn++] = ch;
            }
        } else {
            break;
        }
    }
    line[posn] = '\0';
    qLog(AtChat) << "F :" << line;
    return QString( line );
}

/*!
    Send \a cmd to the specified serial \a device and wait for the
    response.  Returns true if the command responds with \c{OK},
    or false if the command responds with an error or it times out.
    This is typically used by multiplexing plugins to turn on multiplexing
    prior to creating a subclass of QSerialIODeviceMultiplexer.
*/
bool QSerialIODeviceMultiplexer::chat
            ( QSerialIODevice *device, const QString& cmd )
{
    // If the device is not valid, then there is no point going any further.
    if ( !device->isValid() )
        return false;

    // Send the command to the serial device.
    qLog(AtChat) << "T :" << cmd;
    QByteArray cmdstr = cmd.toLatin1();
    const char *str = (const char *)cmdstr;
    device->write( str, strlen( str ) );
    device->putChar( 0x0D );

    // Read the response to check for "OK" or "ERROR".
    // We also support some alternatives for "OK" and "ERROR"
    // that are present on some modems.
    for(;;)
    {
        QString line = readLine( device );
        if ( line.isNull() ||
             line.startsWith( "ERROR" ) ||
             line.startsWith( "COMMAND NOT SUPPORT" ) ||
             line.startsWith( "+CME ERROR:" ) ) {
            return false;
        }
        if ( line.startsWith( "OK" ) ||
             line.startsWith( "AT-command Interpreter ready" ) ) {
            return true;
        }
    }
}

/*!
    Send \a cmd to the specified serial \a device and wait for the
    response.  Returns a null string if the command fails or times
    out, or the contents of the command's response if \c{OK}.

    This function can be used to issue commands such as \c{AT+CGMI} where
    the caller is interested in the text of the response.
*/
QString QSerialIODeviceMultiplexer::chatWithResponse
            ( QSerialIODevice *device, const QString& cmd )
{
    // If the device is not valid, then there is no point going any further.
    if ( !device->isValid() )
        return QString();

    // Send the command to the serial device.
    qLog(AtChat) << "T :" << cmd;
    QByteArray cmdstr = cmd.toLatin1();
    const char *str = (const char *)cmdstr;
    device->write( str, strlen( str ) );
    device->putChar( 0x0D );

    // Read the response to check for "OK" or "ERROR".
    // We also support some alternatives for "OK" and "ERROR"
    // that are present on some modems.
    QString response;
    for(;;)
    {
        QString line = readLine( device );
        if ( line.isNull() ||
             line.startsWith( "ERROR" ) ||
             line.startsWith( "+CME ERROR:" ) ) {
            return QString();
        }
        if ( line.startsWith( "OK" ) ||
             line.startsWith( "AT-command Interpreter ready" ) ) {
            return response;
        }
        response += line;
        response += QChar('\n');
    }
}

// Send out a command to forcibly stop a previous AT+CMUX session.
// Some modems need this or AT+CMUX will not work correctly.
static void forceStop( QSerialIODevice *device )
{
    // If the device is not valid, then there is no point going any further.
    if ( !device->isValid() )
        return;

    // Send out the AT+CMUX terminate command, followed by a CR.
    static char const force_terminate[] =
        {0xF9, 0x03, 0xEF, 0x05, 0xC3, 0x01, 0xF2, 0xF9, 0x0D};
    device->write( force_terminate, 9 );

    // Wait for the line to settle.
    Qtopia::msleep(250);

    // Throw away any data that arrived in the meantime.
    char buffer[256];
    device->discard();
    device->read( buffer, sizeof( buffer ) );

}

/*!
    Create a multiplexer for handling the communications traffic on \a device.
    If \a device is null, a default device handler will be created based
    on the \c{QTOPIA_PHONE_DEVICE} setting.
*/
QSerialIODeviceMultiplexer *QSerialIODeviceMultiplexer::create
            ( QSerialIODevice *device )
{
    char *env;
    bool stopForced = false;

    // Construct the default device if necessary.
    if ( !device ) {
        env = getenv( "QTOPIA_PHONE_DEVICE" );
#ifdef QTOPIA_PHONE_DEVICE
        if ( !env || *env == '\0' )
            env = QTOPIA_PHONE_DEVICE; // Allow custom.h to override.
#endif
        if ( !env || *env == '\0' ) {
            qWarning( "QSerialIODeviceMultiplexer::create: serial device is not configured");
            device = new QNullSerialIODevice();
        } else {
            device = QSerialPort::create( env );
            if ( !device )
                device = new QNullSerialIODevice();
        }
    }

    // Bail out early if multiplexing has been temporarily disabled.
    env = getenv( "QTOPIA_PHONE_MUX" );
#ifdef QTOPIA_PHONE_MUX
    if ( !env || *env == '\0' )
        env = QTOPIA_PHONE_MUX;     // Allow custom.h to override.
#endif
    if ( env && !strcmp( env, "no" ) )
        return new QNullSerialIODeviceMultiplexer( device );

    // Get the name of the vendor multiplexer plugin to load.
    env = getenv( "QTOPIA_PHONE_VENDOR" );
#ifdef QTOPIA_PHONE_VENDOR
    if ( !env || *env == '\0' )
        env = QTOPIA_PHONE_VENDOR;  // Allow custom.h to override.
#endif

    // Load the specified multiplexer plugin and call its detection function.
    if ( env && *env != '\0' ) {
        static QPluginManager *pluginLoader = 0;
        if (!pluginLoader)
            pluginLoader = new QPluginManager( "multiplexers" );
        QSerialIODeviceMultiplexerPluginInterface *plugin = 0;
        QString name = QString(env) + "multiplex";
        qLog(Modem) << "querying multiplexer plugin" << name;
        QObject *obj = pluginLoader->instance( name );
        if( ( plugin = qobject_cast<QSerialIODeviceMultiplexerPluginInterface*>( obj ) )
                    != 0 ) {
            if( plugin->keys().contains( "QSerialIODeviceMultiplexerPluginInterface" ) ) {
                if ( plugin->forceGsm0710Stop() && !stopForced ) {
                    forceStop( device );
                    stopForced = true;
                }
                if ( plugin->detect( device ) ) {
                    return plugin->create( device );
                }
            }
        }
    }

    // Forcibly stop GSM 07.10 if that hasn't been done yet.
    if ( !stopForced )
        forceStop( device );

    // Try to activate GSM 07.10 in the most basic mode.
    if ( QGsm0710Multiplexer::cmuxChat( device ) ) {
        return new QGsm0710Multiplexer( device );
    }

    // Fall back to a null multiplexer if we could not create something better.
    return new QNullSerialIODeviceMultiplexer( device );
}

/*!
    \class QNullSerialIODeviceMultiplexer
    \brief The QNullSerialIODeviceMultiplexer class provides a null implementation of multiplexing
    \ingroup io
    \ingroup telephony::serial

    The QNullSerialIODeviceMultiplexer class provides a null implementation
    on modems that do not support multiplexing.

    This multiplexer provides a primary command channel and a data channel.
    Whenever the data channel is open, the primary command channel cannot be
    used for AT command traffic.  Thus, regular phone operations will be
    suspended while a data call is in progress.

    \sa QSerialIODeviceMultiplexer
*/

class QNullSerialIODeviceMultiplexerPrivate
{
public:
    ~QNullSerialIODeviceMultiplexerPrivate()
    {
        delete primary;
        delete data;
        delete device;
    }

    QSerialIODevice *device;
    QPassThroughSerialIODevice *primary;
    QPassThroughSerialIODevice *data;
};

/*!
    Construct a null multiplexer attached to \a parent whose primary AT
    command channel is \a device.  All other channels will be null.
    Ownership of \a device will pass to this object; it will be
    deleted when this object is deleted.
*/
QNullSerialIODeviceMultiplexer::QNullSerialIODeviceMultiplexer( QSerialIODevice *device, QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    qLog(Modem) << "Multiplexing has been disabled.";
    d = new QNullSerialIODeviceMultiplexerPrivate();

    d->device = device;
    d->primary = new QPassThroughSerialIODevice( device, this );
    d->data = new QPassThroughSerialIODevice( device, this );
    d->primary->setEnabled( true );

    connect( d->data, SIGNAL(opened()), this, SLOT(dataOpened()) );
    connect( d->data, SIGNAL(closed()), this, SLOT(dataClosed()) );
}

/*!
    Destruct this null multiplexer.
*/
QNullSerialIODeviceMultiplexer::~QNullSerialIODeviceMultiplexer()
{
    delete d;
}

/*!
    \reimp
*/
QSerialIODevice *QNullSerialIODeviceMultiplexer::channel( const QString& name )
{
    if ( name == "primary" || name == "secondary" )
        return d->primary;
    else if ( name == "data" || name == "datasetup" )
        return d->data;
    else
        return 0;
}

void QNullSerialIODeviceMultiplexer::dataOpened()
{
    // Switch control of the underlying modem to the data channel.
    d->primary->setEnabled( false );
    d->data->setEnabled( true );
}

void QNullSerialIODeviceMultiplexer::dataClosed()
{
    // Switch control of the underlying modem to the primary command channel.
    d->data->setEnabled( false );
    d->primary->setEnabled( true );
}
