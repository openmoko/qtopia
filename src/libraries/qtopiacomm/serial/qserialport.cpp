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

#include <qserialport.h>
#include <qtopialog.h>

#include <qsocketnotifier.h>
#include <qtimer.h>

#ifndef Q_OS_WIN32
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define USE_POSIX_SYSCALLS  1
#define USE_TERMIOS         1
#endif


class QSerialPortPrivate
{
public:
    QSerialPortPrivate( const QString& device, int rate, bool trackStatus )
    {
        this->device = device;
        this->rate   = rate;
        this->fd     = -1;
        this->status = 0;
        this->track  = trackStatus;
        this->isTty  = false;
        this->dtr    = true;
        this->rts    = true;
        this->flowControl = false;
        this->keepOpen = true;
        this->notifier = 0;
        this->timer  = 0;
    }
    ~QSerialPortPrivate()
    {
        if ( notifier )
            delete notifier;
        if ( timer )
            delete timer;
    }

public:
    QString device;
    int     rate;
    int     fd;
    int     status;
    bool    track;
    bool    isTty;
    bool    dtr;
    bool    rts;
    bool    flowControl;
    bool    keepOpen;
    QSocketNotifier *notifier;
    QTimer *timer;
};

/*!
    \class QSerialPort
    \brief The QSerialPort class provides a simple serial device interface.
    \ingroup io
    \ingroup telephony::serial

    This class manages a very simple serial device, which is accessed
    at a specific baud rate with no parity, 8 data bits, and 1 stop bit.
    It is intended for communicating with GSM modems and the like.

    The recommended way to create an instance of this class is to
    call the QSerialPort::create() method.

    \sa QSerialIODevice
*/

/*!
    Construct a new serial device handler object for the specified
    \a device at the given \a rate.  After construction, it is necessary
    to call QSerialPort::open() to complete initialization.

    If \a trackStatus is true, then the device should attempt to
    track changes in DSR, DCD, and CTS.  This may require the use
    of a regular timeout, which will be detrimental to battery life.

    Status tracking should only be enabled when absolutely required.
    It isn't required for modems that support GSM 07.10 multiplexing,
    as the multiplexing mechanism has its own method of tracking
    status changes that does not require the use of a timeout.

    The device name is usually something like "/dev/ttyS0", but it can
    have the special form "sim:hostname", where "hostname" is the name
    of a host running a phone simulator daemon (usually localhost).
    The phone simulator mode is intended for debugging purposes only.
*/
QSerialPort::QSerialPort( const QString& device, int rate, bool trackStatus )
{
    d = new QSerialPortPrivate( device, rate, trackStatus );
}

/*!
    Destruct this serial device.  If the device is currently open,
    it will be closed.
*/
QSerialPort::~QSerialPort()
{
    close();
    delete d;
}

/*!
    Open the serial device with the specified \a mode.
*/
bool QSerialPort::open( OpenMode mode )
{
    if ( isOpen() ) {
        qLog(Modem) << "QSerialPort already open";
        return false;
    }

#ifdef USE_POSIX_SYSCALLS
    if ( d->device.startsWith( "sim:" ) ) {

        // Connect to a phone simulator via a TCP socket.
        d->fd = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( d->fd == -1 ) {
            qLog(Modem) << "could not open socket";
            return false;
        }
        QString host = d->device.mid( 4 );
        int index = host.indexOf(QChar(':'));
        int port = 12345;
        if ( index != -1 ) {
            port = host.mid( index + 1 ).toInt();
            host = host.left( index );
        }
        QByteArray hoststr = host.toLatin1();
        const char *lhost = (const char *)hoststr;
        struct hostent *ent;
        struct sockaddr_in addr;
        ::memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr( lhost );
        if ( addr.sin_addr.s_addr == INADDR_NONE ) {
            ent = gethostbyname( lhost );
            if ( !ent ) {
                qLog(Modem) << "could not resolve " << lhost;
                ::close( d->fd );
                d->fd = -1;
                return false;
            }
            ::memcpy( &(addr.sin_addr), ent->h_addr, sizeof( addr.sin_addr ) );
        }
        addr.sin_port = htons( (unsigned short)port );
        if ( ::connect( d->fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ) {
            qLog(Modem) << "could not connect to phone simulator at " << lhost;
            ::close( d->fd );
            d->fd = -1;
            return false;
        }
        ::fcntl( d->fd, F_SETFL, O_NONBLOCK );
        d->isTty = 0;

    } else {

        // Must use a non-blocking open to prevent devices such as
        // "/dev/ircomm" and "/dev/ttyS0" from locking up if they
        // aren't presently connected to a remote machine.
        d->fd = ::open( (const char *)d->device.toLatin1(), O_RDWR | O_NONBLOCK, 0 );
        if ( d->fd == -1 ) {
            qLog(Modem) << "QSerialPort " << d->device << " cannot be openned";
            perror("QSerialPort cannot open");
            return false;
        }
        d->isTty = ::isatty( d->fd );

        qLog(Modem) << "Device:" << d->device << "is a tty device:" << (d->isTty ? "True" : "False");

        int fdflags;
        if ((fdflags = fcntl(d->fd, F_GETFL)) == -1 ||
             fcntl(d->fd, F_SETFL, fdflags & ~O_NONBLOCK) < 0)
        {
            perror("couldn't reset non-blocking mode");
            return false;
        }

        qLog(Modem) << "NONBLOCK successfully reset";
    }
#endif
#ifdef USE_TERMIOS
    if ( d->isTty ) {
        // Set the serial port attributes.
        struct termios t;
        speed_t speed;
        ::tcgetattr( d->fd, &t );
        t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
        t.c_cflag |= (CREAD | CLOCAL | CS8);
        t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
        t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
        t.c_iflag &= ~(IXON | IXOFF);
        t.c_oflag &= ~(OPOST | OCRNL);
    #ifdef CRTSCTS
        if ( d->flowControl )
            t.c_cflag |= CRTSCTS;
        else
            t.c_cflag &= ~CRTSCTS;
    #endif
        t.c_cc[VMIN] = 0;
        t.c_cc[VINTR] = _POSIX_VDISABLE;
        t.c_cc[VQUIT] = _POSIX_VDISABLE;
        t.c_cc[VSTART] = _POSIX_VDISABLE;
        t.c_cc[VSTOP] = _POSIX_VDISABLE;
        t.c_cc[VSUSP] = _POSIX_VDISABLE;
        switch( d->rate ) {
            case 50:            speed = B50; break;
            case 75:            speed = B75; break;
            case 110:           speed = B110; break;
            case 134:           speed = B134; break;
            case 150:           speed = B150; break;
            case 200:           speed = B200; break;
            case 300:           speed = B300; break;
            case 600:           speed = B600; break;
            case 1200:          speed = B1200; break;
            case 1800:          speed = B1800; break;
            case 2400:          speed = B2400; break;
            case 4800:          speed = B4800; break;
            case 9600:          speed = B9600; break;
            case 19200:         speed = B19200; break;
            case 38400:         speed = B38400; break;
        #ifdef B57600
            case 57600:         speed = B57600; break;
        #endif
        #ifdef B115200
            case 115200:        speed = B115200; break;
        #endif
        #ifdef B230400
            case 230400:        speed = B230400; break;
        #endif
        #ifdef B460800
            case 460800:        speed = B460800; break;
        #endif
        #ifdef B500000
            case 500000:        speed = B500000; break;
        #endif
        #ifdef B576000
            case 576000:        speed = B576000; break;
        #endif
        #ifdef B921600
            case 921600:        speed = B921600; break;
        #endif
        #ifdef B1000000
            case 1000000:       speed = B1000000; break;
        #endif
        #ifdef B1152000
            case 1152000:       speed = B1152000; break;
        #endif
        #ifdef B1500000
            case 1500000:       speed = B1500000; break;
        #endif
        #ifdef B2000000
            case 2000000:       speed = B2000000; break;
        #endif
        #ifdef B2500000
            case 2500000:       speed = B2500000; break;
        #endif
        #ifdef B3000000
            case 3000000:       speed = B3000000; break;
        #endif
        #ifdef B3500000
            case 3500000:       speed = B3500000; break;
        #endif
        #ifdef B4000000
            case 4000000:       speed = B4000000; break;
        #endif
            default:            speed = 9600; break;
        }
        ::cfsetispeed( &t, speed );
        ::cfsetospeed( &t, speed );
        if( ::tcsetattr( d->fd, TCSANOW, &t ) < 0 )
            qLog(Modem) << "tcsetattr(" << d->fd << ") errno = " << errno;
        int status = TIOCM_DTR | TIOCM_RTS;
        ::ioctl( d->fd, TIOCMBIS, &status );

        // Use a timer to track status changes.  This should be replaced
        // with a separate thread that uses TIOCMIWAIT instead.
        if ( d->track ) {
            ::ioctl( d->fd, TIOCMGET, &(d->status) );
            d->timer = new QTimer( this );
            connect( d->timer, SIGNAL(timeout()), this, SLOT(statusTimeout()) );
            d->timer->start( 500 );
        }
    }
#endif
#ifdef F_SETFD
    // prevent the fd from being passed across a fork/exec.
    ::fcntl( d->fd, F_SETFD, FD_CLOEXEC );
#endif
    d->notifier = new QSocketNotifier
        ( d->fd, QSocketNotifier::Read, this );
    connect( d->notifier, SIGNAL(activated(int)),
             this, SLOT(internalReadyRead()) );
    QIODevice::setOpenMode( mode | QIODevice::Unbuffered );

    return true;
}

/*!
    Close the serial device.
*/
void QSerialPort::close()
{
    if ( d->notifier ) {
        delete d->notifier;
        d->notifier = 0;
    }
    if ( d->timer ) {
        delete d->timer;
        d->timer = 0;
    }
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd != -1 ) {
        ::close( d->fd );
        d->fd = -1;
    }
#endif
    setOpenMode( NotOpen );
}

/*!
    Flush all buffered data to the physical serial device.
*/
bool QSerialPort::flush()
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        ::tcdrain( d->fd );
    }
#endif
    return true;
}

/*!
    Wait up to \a msecs milliseconds for the serial device to
    have data ready to read.
*/
bool QSerialPort::waitForReadyRead(int msecs)
{
#ifdef USE_POSIX_SYSCALLS
    struct timeval tv;
    fd_set readSet;
    if ( d->fd != -1 ) {
        tv.tv_sec = msecs / 1000;
        tv.tv_usec = msecs * 1000;
        FD_ZERO( &readSet );
        FD_SET( d->fd, &readSet );
        return ( select( d->fd + 1, &readSet, 0, 0, &tv ) > 0 );
    } else {
        return false;
    }
#else
    return false;
#endif
}

/*!
    Return the number of bytes that are currently waiting to be read.
*/
qint64 QSerialPort::bytesAvailable() const
{
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd != -1 ) {
        // See comments in Qt4's qsocketlayer_unix.cpp for the reason
        // why this ioctl call sequence is a little bizarre in structure.
        size_t nbytes = 0;
        qint64 available = 0;
        if (::ioctl( d->fd, FIONREAD, (char *) &nbytes ) >= 0)
            available = (qint64) *((int *) &nbytes);
        return available;
    } else {
        return 0;
    }
#else
    return 0;
#endif
}

/*!
    Read bytes from the serial port into the buffer \a data.
    Up to \a maxlen bytes of data will be read.
*/
qint64 QSerialPort::readData( char *data, qint64 maxlen )
{
#ifdef USE_POSIX_SYSCALLS
    int result;
    if ( d->fd == -1 ) {
        return -1;
    }

    while ( ( result = ::read( d->fd, data, (int)maxlen ) ) < 0 ) {
        if ( errno != EINTR ) {
            if ( errno == EWOULDBLOCK ) {
                return 0;
            }
            qLog(Modem) << "read errno = " << errno;
            return -1;
        }
    }

    if ( !result && ( !d->isTty || ( !d->keepOpen && !dsr() ) ) ) {
        // We've received a close notification.  This might happen
        // with the phone simulator if it is shut down before Qtopia.
        // Don't do this for tty devices because there are some systems
        // that return zero when they should be returning EWOULDBLOCK.
        qLog(Modem) << "QSerialPort::readData: other end closed the connection" ;
        close();
    }
    return result;
#else
    return -1;
#endif
}


/*!
    Write \a len bytes from the buffer \a data to the serial port.
*/
qint64 QSerialPort::writeData( const char *data, qint64 len )
{
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd == -1 ) {
        return -1;
    }
    int result = 0;
    int temp;
    while ( len > 0 ) {
        temp = ::write( d->fd, data, (int)len );
        if ( temp >= 0 ) {
            result += temp;
            len -= (qint64)temp;
            data += (uint)temp;
        } else if ( errno != EINTR && errno != EWOULDBLOCK ) {
            qLog(Modem) << "write(" << d->fd << ") errno = " << errno;
            return -1;
        }
    }
    return result;
#else
    return (int)len;
#endif
}


/*!
    Get the serial device's baud rate.
*/
int QSerialPort::rate() const
{
    return d->rate;
}

/*!
    Get the state of CTS/RTS flow control on the serial device.
    The default value is false.
*/
bool QSerialPort::flowControl() const
{
    return d->flowControl;
}

/*!
    Set the state of CTS/RTS flow control on the serial device to \a value.
    This must be called before QSerialPort::open().  Changes to
    the value after opening will be ignored.
*/
void QSerialPort::setFlowControl( bool value )
{
    d->flowControl = value;
}

/*!
    Determine if tty devices should be kept open on a zero-byte read.
    The default value is true.  For RFCOMM sockets, this should be false.
*/
bool QSerialPort::keepOpen() const
{
    return d->keepOpen;
}

/*!
    Set the keep open flag to \a value.  The default value is true.
    For RFCOMM sockets, this should be false.
*/
void QSerialPort::setKeepOpen( bool value )
{
    d->keepOpen = value;
}

/*!
    Get the current state of the DTR modem status line.
*/
bool QSerialPort::dtr() const
{
    return d->dtr;
}

/*!
    Set the state of the DTR modem status line to \a value.
*/
void QSerialPort::setDtr( bool value )
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = TIOCM_DTR;
        if ( value )
            ::ioctl( d->fd, TIOCMBIS, &status );
        else
            ::ioctl( d->fd, TIOCMBIC, &status );
        d->dtr = value;
    }
#endif
}

/*!
    Get the current state of the DSR modem status line.
*/
bool QSerialPort::dsr() const
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        return ( ( status & TIOCM_DSR ) != 0 );
    }
#endif
    return true;
}

/*!
    Get the current state of the DCD (carrier) modem status line.
*/
bool QSerialPort::carrier() const
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        return ( ( status & TIOCM_CAR ) != 0 );
    }
#endif
    return true;
}

/*!
    Get the current state of the RTS modem status line.
*/
bool QSerialPort::rts() const
{
    return d->rts;
}

/*!
    Set the state of the RTS modem status line to \a value.
*/
void QSerialPort::setRts( bool value )
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = TIOCM_RTS;
        if ( value )
            ::ioctl( d->fd, TIOCMBIS, &status );
        else
            ::ioctl( d->fd, TIOCMBIC, &status );
        d->rts = value;
    }
#endif
}

/*!
    Get the current state of the CTS modem status line.
*/
bool QSerialPort::cts() const
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        return ( ( status & TIOCM_CTS ) != 0 );
    }
#endif
    return true;
}

/*!
    Discard pending buffered data without transmitting it.  The companion
    function QIODevice::flush() waits for the buffers to empty.  This function
    will do nothing if the underlying implementation cannot discard buffers.
*/
void QSerialPort::discard()
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        ::tcflush( d->fd, TCIOFLUSH );
    }
#endif
}

/*!
    Determine if this device is valid.
*/
bool QSerialPort::isValid() const
{
#ifdef USE_POSIX_SYSCALLS
    return ( d->fd != -1 );
#else
    return QSerialIODevice::isValid();
#endif
}

/*!
    \reimp

    This class overrides QSerialIODevice::run() to run pppd directly on the
    underlying device name so that it isn't necessary to create a pseudo-tty.
*/
QProcess *QSerialPort::run( const QStringList& arguments,
                              bool addPPPdOptions )
{
    if ( addPPPdOptions && d->isTty ) {
        // Bail out if we are already running a process on this device.
        if ( !isOpen() )
            return 0;

        // Close the serial device before handing it off to the child process.
        close();

        // Build a new argument list for pppd, with the device name in place.
        QStringList newargs;
        newargs << d->device;
        newargs << QString::number( d->rate );
        for ( int index = 1; index < arguments.size(); ++index )
            newargs << arguments[index];

        // Run the process.  When it exits, open() will be called again.
        QProcess *process = new QProcess();
        process->setReadChannelMode( QProcess::ForwardedChannels );
        connect( process, SIGNAL(stateChanged(QProcess::ProcessState)),
                 this, SLOT(pppdStateChanged(QProcess::ProcessState)) );
        connect( process, SIGNAL(destroyed()), this, SLOT(pppdDestroyed()) );
        process->start( arguments[0], newargs );
        return process;
    } else {
        return QSerialIODevice::run( arguments, addPPPdOptions );
    }
}

void QSerialPort::statusTimeout()
{
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int old_status = d->status;
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        d->status = status;
        if ( ( ( old_status ^ status ) & TIOCM_DSR ) != 0 )
            emit dsrChanged( ( status & TIOCM_DSR ) != 0 );
        if ( ( ( old_status ^ status ) & TIOCM_CAR ) != 0 )
            emit carrierChanged( ( status & TIOCM_CAR ) != 0 );
    }
#endif
}

void QSerialPort::pppdStateChanged( QProcess::ProcessState state )
{
    if ( state == QProcess::NotRunning && !isOpen() ) {
        // The process has exited, so re-open the device for our own use.
        open( QIODevice::ReadWrite );
    }
}

void QSerialPort::pppdDestroyed()
{
    if ( !isOpen() ) {
        // The process has been killed, so re-open the device for our own use.
        open( QIODevice::ReadWrite );
    }
}

/*!
    Create and open a serial device from a \a name of the form "device:rate".
    Returns NULL if the device could not be opened.  The \a defaultRate
    parameter indicates the default rate to use if ":rate" was not included
    in the device name.  If \a flowControl is true, then CTS/RTS flow
    control should be enabled on the device.
*/
QSerialPort *QSerialPort::create( const QString& name, int defaultRate,
                                    bool flowControl )
{
    int rate = defaultRate;
    QString dev = name;
    if ( dev.length() > 0 && dev[0] == '/' ) {
        int index = dev.indexOf(QChar(':'));
        if ( index != -1 ) {
            rate = dev.mid( index + 1 ).toInt();
            dev = dev.left( index );
        }
    }
    qLog(Modem) << "opening serial device " << dev << " at " << rate;
    QSerialPort *device = new QSerialPort( dev, rate );
    device->setFlowControl( flowControl );
    if ( !device->open( ReadWrite ) ) {
        qWarning() << "Failed opening " << dev;
        delete device;
        return 0;
    } else {
        qLog(Modem) << "Opened " << dev;
        return device;
    }
}
