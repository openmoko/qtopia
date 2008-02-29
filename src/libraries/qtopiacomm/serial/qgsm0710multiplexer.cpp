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

#include <qgsm0710multiplexer.h>
#include <qtopialog.h>
#include <qmap.h>
#include <alloca.h>

/*!
    \class QGsm0710Multiplexer
    \mainclass
    \brief The QGsm0710Multiplexer class provides a multiplexing implementation based on 3GPP TS 07.10/27.010
    \ingroup telephony::serial

    This is the default multiplexer implementation that is tried if a multiplexer
    plug-in override is not available.

    By default, this class uses the 3GPP TS 07.10/27.010 basic multiplexing mode,
    with a frame size of 31 bytes.  The mode and/or frame size can be modified
    by writing a multiplexer plug-in.  See the \l{Tutorial: Writing a Multiplexer Plug-in}
    for more information on how to write a multiplexer plug-in that modifies the
    3GPP TS 07.10/27.010 parameters.

    \sa QSerialIODeviceMultiplexer, QMultiPortMultiplexer, QSerialIODeviceMultiplexerPlugin
*/

class QGsm0710MultiplexerChannel;

class QGsm0710MultiplexerPrivate
{
public:
    QGsm0710MultiplexerPrivate( QSerialIODevice *device,
                                int frameSize, bool advanced,
                                bool server )
    {
        this->device = device;
        this->frameSize = frameSize;
        this->advanced = advanced;
        this->used = 0;
        this->server = server;
    }
    ~QGsm0710MultiplexerPrivate();

    void closeAll();

    QSerialIODevice *device;
    int frameSize;
    bool advanced;
    QMap<int,QGsm0710MultiplexerChannel *> channels;
    char buffer[4096];
    uint used;
    bool server;
};

class QGsm0710MultiplexerChannel : public QSerialIODevice
{
public:
    QGsm0710MultiplexerChannel( QGsm0710Multiplexer *mux, int channel );
    ~QGsm0710MultiplexerChannel();

    int chan() const { return channel; }

    // Override methods from QIODevice.
    bool open( OpenMode mode );
    void close();
    qint64 bytesAvailable() const;
    bool waitForReadyRead( int msecs );

    // Override methods from QSerialIODevice.
    int rate() const;
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool setCarrier( bool value );
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();

    void reopen();
protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

public:
    static void writeFrame( QGsm0710Multiplexer *mux, int channel,
                            int type, const char *data, uint len );
    void add( const char *data, uint len );
    void setStatus( int status );

private:
    void updateStatus();

private:
    QGsm0710Multiplexer *mux;
    int channel;
    char incomingStatus;
    char outgoingStatus;
    bool previouslyOpened;
    bool currentlyOpen;
    bool waitingForReadyRead;
    int used;
    QByteArray buffer;
};

QGsm0710MultiplexerPrivate::~QGsm0710MultiplexerPrivate()
{
    if ( !server )      // Don't delete the device when in server mode.
        delete device;
}

void QGsm0710MultiplexerPrivate::closeAll()
{
    QMap<int,QGsm0710MultiplexerChannel *>::Iterator iter;
    for ( iter = channels.begin(); iter != channels.end(); ++iter ) {
        delete iter.value();
    }
}

#define Gsm710_FC       0x02
#define Gsm710_DTR      0x04
#define Gsm710_DSR      0x04
#define Gsm710_RTS      0x08
#define Gsm710_CTS      0x08
#define Gsm710_DCD      0x80

QGsm0710MultiplexerChannel::QGsm0710MultiplexerChannel
        ( QGsm0710Multiplexer *mux, int channel )
{
    this->mux = mux;
    this->channel = channel;
    this->incomingStatus = Gsm710_DSR | Gsm710_CTS;
    this->outgoingStatus = Gsm710_DTR | Gsm710_RTS | Gsm710_DCD | 0x01;
    this->previouslyOpened = false;
    this->currentlyOpen = false;
    this->waitingForReadyRead = false;
    this->used = 0;
}

QGsm0710MultiplexerChannel::~QGsm0710MultiplexerChannel()
{
    // Send a close request for this channel if it was ever open.
    if ( previouslyOpened )
        writeFrame( mux, channel, 0x53, 0, 0 );
}

bool QGsm0710MultiplexerChannel::open( OpenMode mode )
{
    QIODevice::setOpenMode( ( mode & ReadWrite ) | Unbuffered );
    if ( !previouslyOpened && !mux->d->server ) {
        // Send an open request for this channel.
        writeFrame( mux, channel, 0x3F, 0, 0 );
        previouslyOpened = true;
    }
    currentlyOpen = true;
    return true;
}


void QGsm0710MultiplexerChannel::reopen()
{
    if ( !mux->d->server )
        writeFrame( mux, channel, 0x3F, 0, 0 );
}

void QGsm0710MultiplexerChannel::close()
{
    setOpenMode( NotOpen );
    currentlyOpen = false;
    used = 0;
}

qint64 QGsm0710MultiplexerChannel::bytesAvailable() const
{
    return used;
}

bool QGsm0710MultiplexerChannel::waitForReadyRead( int msecs )
{
    // Wait for the underlying device to have data and process it.
    // We must loop around because we could see data for other channels.
    if ( used > 0 ) {
        // We already have data ready to be read.
        return true;
    }
    waitingForReadyRead = true;
    while ( mux->d->device->waitForReadyRead( msecs ) ) {
        mux->incoming();
        if ( used > 0 ) {
            waitingForReadyRead = false;
            return true;
        }
    }
    waitingForReadyRead = false;
    return false;
}

int QGsm0710MultiplexerChannel::rate() const
{
    return mux->d->device->rate();
}

bool QGsm0710MultiplexerChannel::dtr() const
{
    return ( ( outgoingStatus & Gsm710_DTR ) != 0 );
}

void QGsm0710MultiplexerChannel::setDtr( bool value )
{
    if ( value != dtr() ) {
        if ( value )
            outgoingStatus |= Gsm710_DTR;
        else
            outgoingStatus &= ~Gsm710_DTR;
        updateStatus();
    }
}

bool QGsm0710MultiplexerChannel::dsr() const
{
    return ( ( incomingStatus & Gsm710_DSR ) != 0 );
}

bool QGsm0710MultiplexerChannel::carrier() const
{
    return ( ( incomingStatus & Gsm710_DCD ) != 0 );
}

bool QGsm0710MultiplexerChannel::setCarrier( bool value )
{
    if ( value )
        outgoingStatus |= Gsm710_DCD;
    else
        outgoingStatus &= ~Gsm710_DCD;
    updateStatus();
    return true;
}

bool QGsm0710MultiplexerChannel::rts() const
{
    return ( ( outgoingStatus & Gsm710_RTS ) != 0 );
}

void QGsm0710MultiplexerChannel::setRts( bool value )
{
    if ( value != rts() ) {
        outgoingStatus &= ~( Gsm710_RTS | Gsm710_FC );
        if ( value )
            outgoingStatus |= Gsm710_RTS;
        else
            outgoingStatus |= Gsm710_FC;
        updateStatus();
    }
}

bool QGsm0710MultiplexerChannel::cts() const
{
    return ( ( incomingStatus & Gsm710_CTS ) != 0 );
}

void QGsm0710MultiplexerChannel::discard()
{
    used = 0;
}

qint64 QGsm0710MultiplexerChannel::readData( char *data, qint64 maxlen )
{
    uint size;
    if ( used == 0 ) {
        size = 0;
    } else if ( maxlen >= (qint64)used ) {
        memcpy( data, buffer.data(), used );
        size = used;
        used = 0;
    } else {
        memcpy( data, buffer.data(), (int)maxlen );
        memmove( buffer.data(), buffer.data() + (uint)maxlen,
                 used - (uint)maxlen );
        used -= (uint)maxlen;
        size = (uint)maxlen;
    }
    return size;
}

qint64 QGsm0710MultiplexerChannel::writeData( const char *data, qint64 len )
{
    qint64 result = len;
    uint framelen;
    while ( len > 0 ) {
        framelen = len;
        if ( framelen > (uint)( mux->d->frameSize ) ) {
            framelen = (uint)( mux->d->frameSize );
        }
        writeFrame( mux, channel, 0xEF, data, framelen );
        data += framelen;
        len -= framelen;
    }
    return result;
}

static const unsigned char crcTable[256] = {
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
    0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
    0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
    0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
    0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
    0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
    0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
    0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
    0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
    0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
    0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
    0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
    0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
    0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
    0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
    0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
    0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

static int computeCrc( const char *data, uint len )
{
    int sum = 0xFF;
    while ( len > 0 ) {
        sum = crcTable[ ( sum ^ *data++ ) & 0xFF ];
        --len;
    }
    return ((0xFF - sum) & 0xFF);
}


void QGsm0710MultiplexerChannel::writeFrame
        ( QGsm0710Multiplexer *mux, int channel,
          int type, const char *data, uint len )
{
    char *frame = (char *)alloca(mux->d->frameSize * 2 + 8);
    if ( mux->d->advanced ) {
        uint size = 0;
        int temp, crc;
        frame[0] = (char)((channel << 2) | 0x03);
        frame[1] = (char)type;
        crc = computeCrc( frame, 2 );
        frame[size++] = (char)0x7E;
        temp = ((channel << 2) | 0x03);
        if ( temp != 0x7E && temp != 0x7D ) {
            frame[size++] = (char)temp;
        } else {
            frame[size++] = (char)0x7D;
            frame[size++] = (char)(temp ^ 0x20);
        }
        if ( type != 0x7E && type != 0x7D ) {
            frame[size++] = (char)type;
        } else {
            frame[size++] = (char)0x7D;
            frame[size++] = (char)(type ^ 0x20);
        }
        while ( len > 0 ) {
            temp = *data++ & 0xFF;
            --len;
            if ( temp != 0x7E && temp != 0x7D ) {
                frame[size++] = (char)temp;
            } else {
                frame[size++] = (char)0x7D;
                frame[size++] = (char)(temp ^ 0x20);
            }
        }
        if ( crc != 0x7E && crc != 0x7D ) {
            frame[size++] = (char)crc;
        } else {
            frame[size++] = (char)0x7D;
            frame[size++] = (char)(crc ^ 0x20);
        }
        frame[size++] = (char)0x7E;
        if ( qLogEnabled(Mux) ) {
            qLog(Mux) << "QMuxDevice::writeFrameAdvanced()";
            for ( uint i = 0; i < size; ++i ) {
                if( i >= 16 && (i % 16) == 0 )
                    fprintf(stdout, "\n");
                fprintf(stdout, "%02x ", frame[i] & 0xFF);
            }
            fprintf(stdout, "\n");
        }
        mux->d->device->write( frame, size );
    } else {
        uint size;
        frame[0] = (char)0xF9;
        frame[1] = (char)((channel << 2) | 0x03);
        frame[2] = (char)type;
        if ( len <= 127 ) {
            frame[3] = (char)((len << 1) | 0x01);
            size = 4;
        } else {
            frame[3] = (char)(len << 1);
            frame[4] = (char)(len >> 7);
            size = 5;
        }
        if ( len > 0 )
            memcpy( frame + size, data, len);
        // Note: GSM 07.10 says that the CRC is only computed over the header.
        frame[len + size] = (char)computeCrc( frame + 1, size - 1 );
        frame[len + size + 1] = (char)0xF9;
        if ( qLogEnabled(Mux) ) {
            qLog(Mux) << "QMuxDevice::writeFrameBasic()";
            for ( uint i = 0; i < len + size + 2; ++i ) {
                if( i >= 16 && (i % 16) == 0 )
                    fprintf(stdout, "\n");
                fprintf(stdout, "%02x ", frame[i] & 0xFF);
            }
            fprintf(stdout, "\n");
        }
        mux->d->device->write( frame, len + size + 2 );
    }
}

void QGsm0710MultiplexerChannel::add( const char *data, uint len )
{
    if ( !currentlyOpen ) {
        // The upper layers haven't opened for this channel yet,
        // so we discard any input.  This prevents unused channels
        // from increasing their buffer sizes indefinitely to
        // cache data that will never be read.
        return;
    }
    if ( ((int)( used + len )) > buffer.size() ) {
        buffer.resize( used + len );
    }
    memcpy( buffer.data() + used, data, len );
    used += len;
    if ( len > 0 && !waitingForReadyRead ) {
        internalReadyRead();
    }
}

// Set the incoming status on this channel and emit the relevant signals.
void QGsm0710MultiplexerChannel::setStatus( int status )
{
    int lastStatus = incomingStatus;
    status &= ( Gsm710_DSR | Gsm710_DCD | Gsm710_CTS );
    incomingStatus = (char)status;
    if ( ( ( status ^ lastStatus ) & Gsm710_DSR ) != 0 ) {
        emit dsrChanged( ( status & Gsm710_DSR ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & Gsm710_DCD ) != 0 ) {
        emit carrierChanged( ( status & Gsm710_DCD ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & Gsm710_CTS ) != 0 ) {
        emit ctsChanged( ( status & Gsm710_CTS ) != 0 );
    }
}

// Update the outgoing status of DTR and RTS on this channel.
void QGsm0710MultiplexerChannel::updateStatus()
{
    char data[4];
    data[0] = (char)0xE3;
    data[1] = (char)0x03;
    data[2] = ( channel << 2 ) | 0x03;
    data[3] = outgoingStatus;
    writeFrame( mux, 0, 0xEF, data, 4 );
}

/*!
    Construct a new GSM 07.10 multiplexer around \a device and attach
    it to \a parent.  The size of frames is \a frameSize.  If \a advanced
    is true, then use the Advanced multiplexing option; otherwise use
    the Basic multiplexing option.

    Ownership of \a device will pass to this object; it will be
    deleted when this object is deleted.
*/
QGsm0710Multiplexer::QGsm0710Multiplexer( QSerialIODevice *device,
                                          int frameSize, bool advanced,
                                          QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QGsm0710MultiplexerPrivate( device, frameSize, advanced, false );
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );

    // Create the control channel (0).
    QGsm0710MultiplexerChannel::writeFrame( this, 0, 0x3F, 0, 0 );
}

// This constructor is called from QGsm0710MultiplexerServer only.
QGsm0710Multiplexer::QGsm0710Multiplexer( QSerialIODevice *device,
                                          int frameSize, bool advanced,
                                          QObject *parent, bool server )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QGsm0710MultiplexerPrivate( device, frameSize, advanced, server );
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );
}

/*!
    Destruct this GSM 07.10 multiplexer, closing the session.
*/
QGsm0710Multiplexer::~QGsm0710Multiplexer()
{
    // Close all of the active channels.
    d->closeAll();

    // Send the terminate command to exit AT+CMUX multiplexing in client mode.
    if ( !d->server ) {
        static char const gsm0710_terminate[] = {0xC3, 0x01};
        QGsm0710MultiplexerChannel::writeFrame
            ( this, 0, 0xEF, gsm0710_terminate, 2 );
    }

    // Clean up everything else.
    delete d;
}

/*!
    \reimp
*/
QSerialIODevice *QGsm0710Multiplexer::channel( const QString& name )
{
    // Convert the name into a channel number.
    int number = channelNumber( name );
    if ( number == -1 )
        return 0;

    // Look up the channel's device by its number.
    if ( d->channels.contains( number ) )
        return d->channels.value( number );

    // Create a new channel with the specified number.
    QGsm0710MultiplexerChannel *channel =
            new QGsm0710MultiplexerChannel( this, number );
    d->channels.insert( number, channel );
    return channel;
}

/*!
    Construct a \c{AT+CMUX} command with the specified parameters and
    send it to \a device.  Returns true if the command succeeded, or
    false if the command failed.  This is typically called by plug-ins
    that use GSM 07.10 with a different frame size or operating mode.

    The size of frames is \a frameSize.  If \a advanced is true, then use
    the Advanced multiplexing option; otherwise use the Basic multiplexing
    option.  The baud rate is acquired from \a device, and the subset is
    always set to zero.

    This is a convenience function for the most common options that
    are passed to \c{AT+CMUX}.  If more involved options are required
    (e.g. subset or timeout values), then the plug-in should call
    QSerialIODeviceMultiplexer::chat() instead.

    \sa QSerialIODeviceMultiplexer::chat()
*/
bool QGsm0710Multiplexer::cmuxChat( QSerialIODevice *device,
                                    int frameSize, bool advanced )
{
    int portSpeed;
    switch ( device->rate() ) {
        case 9600:      portSpeed = 1; break;
        case 19200:     portSpeed = 2; break;
        case 38400:     portSpeed = 3; break;
        case 57600:     portSpeed = 4; break;
        case 115200:    portSpeed = 5; break;
        case 230400:    portSpeed = 6; break;
        default:        portSpeed = 5; break;
    }
    QString cmd = "AT+CMUX=";
    if ( advanced )
        cmd += "1,0,";
    else
        cmd += "0,0,";
    cmd += QString::number( portSpeed ) + "," + QString::number( frameSize );
    return chat( device, cmd );
}

/*!
    Returns the GSM 07.10 channel number associated with the channel \a name.
    Returns -1 if the channel name is not recognized.  The default channel
    assignment is as follows:

    \table
    \header \o Name \o Number
    \row \o \c{primary} \o 1
    \row \o \c{secondary} \o 2
    \row \o \c{data} \o 3
    \row \o \c{datasetup} \o 3
    \row \o \c{aux*} \o 4
    \endtable

    This assignment causes data call setup commands to be sent on the data
    channel.  If a modem needs data call setup on the primary AT command
    channel, then it should override this method and return 1 for \c{datasetup}.

    The current implementation is limited to channel numbers between 1 and 63.
    Numbers outside this range should not be returned.

    \sa channel()
*/
int QGsm0710Multiplexer::channelNumber( const QString& name ) const
{
    if ( name == "primary" )
        return 1;
    else if ( name == "secondary" )
        return 2;
    else if ( name == "data" || name == "datasetup" )
        return 3;
    else if ( name.startsWith( "aux" ) )
        return 4;
    else
        return -1;
}

/*!
    Re-initialize the multiplexing session.  This is called by
    subclasses that have detected that the modem has dropped out
    of multiplexing mode.
*/
void QGsm0710Multiplexer::reinit()
{
    // Send the AT+CMUX command to re-initialize the session.
    if ( !cmuxChat( d->device, d->frameSize, d->advanced ) ) {
        qLog(Mux) << "Could not re-initialize multiplexing with AT+CMUX";
        return;
    }

    // Re-create the control channel (0).
    QGsm0710MultiplexerChannel::writeFrame( this, 0, 0x3F, 0, 0 );

    // Reopen any channels that we were using before the session aborted.
    QMap<int,QGsm0710MultiplexerChannel *>::Iterator iter;
    for ( iter = d->channels.begin(); iter != d->channels.end(); ++iter ) {
        iter.value()->reopen();
        iter.value()->add( "\r\nERROR\r\n", 9 );
    }
}

void QGsm0710Multiplexer::incoming()
{
    // Read more data from the serial device.
    int len = d->device->read( d->buffer + d->used,
                               sizeof(d->buffer) - d->used );
    if ( len <= 0 )
        return;

    // Log the new data to the debug stream.
    if ( qLogEnabled(Mux) ) {
        qLog(Mux) << "QGsm0710Multiplexer::incoming()";
        for ( uint i = d->used; i < d->used + (uint)len; ++i ) {
            if( i >= 16 && (i % 16) == 0 )
                fprintf(stdout, "\n");
            fprintf(stdout, "%02x ", d->buffer[i] & 0xFF);
        }
        fprintf(stdout, "\n");
    }

    // Update the buffer size.
    d->used += (uint)len;

    // Break the incoming data up into packets.
    uint posn = 0;
    uint posn2;
    uint headerSize;
    int channel, type;
    while ( posn < d->used ) {
        if ( d->buffer[posn] == (char)0xF9 ) {

            // Basic format: skip additional 0xF9 bytes between frames.
            while ( ( posn + 1 ) < d->used &&
                    d->buffer[posn + 1] == (char)0xF9 ) {
                ++posn;
            }

            // We need at least 4 bytes for the header.
            if ( ( posn + 4 ) > d->used )
                break;

            // The low bit of the second byte should be 1,
            // which indicates a short channel number.
            if ( ( d->buffer[posn + 1] & 0x01 ) == 0 ) {
                ++posn;
                continue;
            }

            // Get the packet length and validate it.
            len = (d->buffer[posn + 3] >> 1) & 0x7F;
            if ( ( d->buffer[posn + 3] & 0x01 ) != 0 ) {
                // Single-byte length indication.
                headerSize = 3;
            } else {
                // Double-byte length indication.
                if ( ( posn + 5 ) > d->used )
                    break;
                len |= ((int)(unsigned char)(d->buffer[posn + 4])) << 7;
                headerSize = 4;
            }
            if ( ( posn + headerSize + 2 + len ) > d->used )
                break;

            // Verify the packet header checksum.
            if ( ( ( computeCrc( d->buffer + posn + 1, headerSize ) ^
                     d->buffer[posn + len + headerSize + 1] ) & 0xFF )
                            != 0 ) {
                qLog(Mux) << "*** GSM 07.10 checksum check failed ***";
                int totalsize = len + headerSize + 2;
                if ( qLogEnabled(Mux) ) {
                    qLog(Mux) << "printing out failed mux packet, starting at buffer addr " << posn << ". read data length from serial port is " << len << ", packet headersize is " << headerSize;
                    for( uint i = posn ; i <= (posn+totalsize) ; ++i ) {
                        if( (posn % 8) == 0 )
                            fprintf( stdout, "\n");
                        fprintf( stdout, "%x ", d->buffer[i] );
                    }
                }

                posn += len + headerSize + 2;
                continue;
            }

            // Get the channel number and packet type from the header.
            channel = (d->buffer[posn + 1] >> 2) & 0x3F;
            type = d->buffer[posn + 2] & 0xEF;  // Strip "PF" bit.

            // Dispatch data packets to the appropriate channel.
            if ( !packet( channel, type,
                          d->buffer + posn + headerSize + 1, len ) ) {
                // Session has been terminated.
                d->used = 0;
                return;
            }
            posn += len + headerSize + 2;

        } else if ( d->buffer[posn] == (char)0x7E ) {

            // Advanced format: skip additional 0x7E bytes between frames.
            while ( ( posn + 1 ) < d->used &&
                    d->buffer[posn + 1] == (char)0x7E ) {
                ++posn;
            }

            // Search for the end of the packet (the next 0x7E byte).
            len = posn + 1;
            while ( ((uint)len) < d->used &&
                    d->buffer[len] != (char)0x7E ) {
                ++len;
            }
            if ( ((uint)len) >= d->used ) {
                // There are insufficient bytes for a packet at present.
                if ( posn == 0 && len >= (int)sizeof( d->buffer ) ) {
                    // The buffer is full and we were unable to find a
                    // legitimate packet.  Discard the buffer and restart.
                    posn = len;
                }
                break;
            }

            // Undo control byte quoting in the packet.
            posn2 = 0;
            ++posn;
            while ( posn < (uint)len ) {
                if ( d->buffer[posn] == 0x7D ) {
                    ++posn;
                    if ( posn >= (uint)len )
                        break;
                    d->buffer[posn2++] = (char)(d->buffer[posn++] ^ 0x20);
                } else {
                    d->buffer[posn2++] = d->buffer[posn++];
                }
            }

            // Validate the checksum on the packet header.
            if ( posn2 >= 3 ) {
                if ( ( ( computeCrc( d->buffer, 2 ) ^
                     d->buffer[posn2 - 1] ) & 0xFF ) != 0 ) {
                    qLog(Mux) << "*** GSM 07.10 advanced checksum "
                                   "check failed ***";
                    continue;
                }
            } else {
                qLog(Mux) << "*** GSM 07.10 advanced packet is too small ***";
                continue;
            }

            // Decode and dispatch the packet.
            channel = (d->buffer[0] >> 2) & 0x3F;
            type = d->buffer[1] & 0xEF;  // Strip "PF" bit.
            if ( !packet( channel, type, d->buffer + 2, posn2 - 3 ) ) {
                // Session has been terminated.
                d->used = 0;
                return;
            }

        } else {
            ++posn;
        }
    }
    if ( posn < d->used ) {
        memmove( d->buffer, d->buffer + posn, d->used - posn );
        d->used -= posn;
    } else {
        d->used = 0;
    }
}

bool QGsm0710Multiplexer::packet( int channel, int type,
                                  const char *data, uint len )
{
    QGsm0710MultiplexerChannel *chan;
    if ( type == 0xEF || type == 0x03 ) {

        if ( d->channels.contains( channel ) ) {
            // Ordinary data packet.
            chan = d->channels.value( channel );
            chan->add( data, len );
        } else if ( channel == 0 ) {
            // An embedded command or response on channel 0.
            if ( len >= 2 && data[0] == (char)0xE3 ) {
                return packet( channel, 0xE1, data + 2, len - 2 );
            } else if ( len >= 2 && data[0] == (char)0xC3 && d->server ) {
                // Incoming terminate request on server side.
                terminate();
                return false;
            } else if ( len >= 2 && data[0] == (char)0x43 ) {
                // Test command from the other side - send the same bytes back.
                char *resp = (char *)alloca( len );
                memcpy( resp, data, len );
                resp[0] = (char)0x41;   // Clear the C/R bit in the response.
                QGsm0710MultiplexerChannel::writeFrame
                    ( this, 0, 0xEF, resp, len );
            }
        }

    } else if ( type == 0xE1 && channel == 0 ) {

        // Status change message.
        if ( len >= 2 ) {
            // Handle status changes on other channels.
            channel = ( ( data[0] & 0xFC ) >> 2 );
            if ( d->channels.contains( channel ) ) {
                chan = d->channels.value( channel );
                chan->setStatus( data[1] & 0xFF );
            }
        }

        // Send the response to the status change request to ACK it.
        qLog(Mux) << "Received status line signal, sending response";
        char resp[33];
        if ( len > 31 )
            len = 31;
        resp[0] = (char)0xE1;
        resp[1] = (char)((len << 1) | 0x01);
        memcpy( resp + 2, data, len );
        QGsm0710MultiplexerChannel::writeFrame( this, 0, 0xEF, resp, len + 2 );

    } else if ( type == ( 0x3F & 0xEF ) && d->server && channel != 0 ) {

        // Incoming channel open request on server side.
        open( channel );

    } else if ( type == ( 0x53 & 0xEF ) && d->server && channel != 0 ) {

        // Incoming channel close request on server side.
        close( channel );

    }
    return true;
}

void QGsm0710Multiplexer::terminate()
{
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    QMap<int,QGsm0710MultiplexerChannel *>::Iterator iter;
    for ( iter = d->channels.begin(); iter != d->channels.end(); ++iter ) {
        emit server->closed( iter.value()->chan(), iter.value() );
        delete iter.value();
    }
    d->channels.clear();
    emit server->terminated();
}

void QGsm0710Multiplexer::open( int channel )
{
    // If there is already a channel open with this number, then ignore.
    if ( d->channels.contains( channel ) )
        return;

    // Create a new channel device and register it.
    QGsm0710MultiplexerChannel *device =
            new QGsm0710MultiplexerChannel( this, channel );
    d->channels.insert( channel, device );

    // Make sure that the channel device is properly opened.
    device->open( QIODevice::ReadWrite );

    // Let interested parties know about the new channel.
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    emit server->opened( channel, device );
}

void QGsm0710Multiplexer::close( int channel )
{
    // If this channel is not currently active, then ignore.
    if ( !d->channels.contains( channel ) )
        return;

    // Remove the device from the channel list.
    QGsm0710MultiplexerChannel *device = d->channels.value( channel );
    d->channels.remove( channel );

    // Close the device for the channel.
    device->close();

    // Let interested parties know that the channel is closing.
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    emit server->closed( channel, device );

    // Delete the device as it is no longer required.
    delete device;
}

/*!
    \class QGsm0710MultiplexerServer
    \mainclass
    \brief The QGsm0710MultiplexerServer class provides a server-side multiplexing implementation based on 3GPP TS 07.10/27.010
    \ingroup telephony::serial

    This class is used by incoming AT connection managers such as the \l{Modem Emulator}
    to emulate 3GPP TS 07.10/27.010 multiplexing for the remote device that is connecting.

    When the remote device opens a channel, the opened() signal is emitted, providing a
    QSerialIODevice that can be used by the AT connection manager to communicate
    with the remote device on that channel.

    When the remote device closes a channel, the closed() signal is emitted to allow
    the AT connection manager to clean up the QSerialIODevice for the channel and
    any other associated channel-specific context information.

    When the remote device terminates the 3GPP TS 07.10/27.010 session, closed() signals
    for all open channels will be emitted, and then the terminated() signal will be
    emitted.

    \sa QGsm0710Multiplexer, {Modem Emulator}
*/

/*!
    Construct a new GSM 07.10 multiplexer in server mode around \a device
    and attach it to \a parent.  The size of frames is \a frameSize.
    If \a advanced is true, then use the Advanced multiplexing option;
    otherwise use the Basic multiplexing option.

    Unlike the base class, QGsm0710Multiplexer, the ownership of
    \a device will not pass to this object and it will not be deleted
    when this object is deleted.  It will still exist after destruction.
    This is because the device will typically return to normal AT command
    mode after the multiplexer exits.
*/
QGsm0710MultiplexerServer::QGsm0710MultiplexerServer
            ( QSerialIODevice *device, int frameSize,
              bool advanced, QObject *parent )
    : QGsm0710Multiplexer( device, frameSize, advanced, parent, true )
{
    // Nothing to do here at present.
}

/*!
    Destruct this GSM 07.10 server instance.
*/
QGsm0710MultiplexerServer::~QGsm0710MultiplexerServer()
{
    // Nothing to do here at present.
}

/*!
    \fn void QGsm0710MultiplexerServer::opened( int channel, QSerialIODevice *device )

    Signal that is emitted when the client opens a new GSM 07.10
    \a channel.  The \a device object allows access to the raw
    data and modem control signals on the channel.

    \sa closed()
*/

/*!
    \fn void QGsm0710MultiplexerServer::closed( int channel, QSerialIODevice *device )

    Signal that is emitted when the client closes a GSM 07.10 \a channel.
    After this signal is emitted, \a device will no longer be valid.

    \sa opened()
*/

/*!
    \fn void QGsm0710MultiplexerServer::terminated()

    Signal that is emitted when the client terminates the GSM 07.10
    session and wishes to return to normal command mode.

    This signal will be preceded by closed() signals for all channels
    that were still open when the terminate command was received.

    A slot that is connected to this signal should use QObject::deleteLater()
    to clean up this object.
*/
