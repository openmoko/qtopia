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

#include <qirremotedevicewatcher.h>
#include <qtopiacomm/private/qirnamespace_p.h>
#include <qtopialog.h>

#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

class QIrRemoteDeviceWatcher_Private
{
public:
    int fd;
};

/*!
    \class QIrRemoteDeviceWatcher
    \mainclass
    \brief The QIrRemoteDeviceWatcher class watches for remote devices.

    QIrRemoteDeviceWatcher can be used to wait for remote infrared devices
    to come into range, and notify the user.  Using this class, a system
    can alert the user that another infrared device is available for communication.

    \ingroup qtopiair
 */

/*!
    Constructs a remote device watcher.  The \a parent parameter specifies 
    the object's parent.
*/
QIrRemoteDeviceWatcher::QIrRemoteDeviceWatcher(QObject *parent)
    : QObject(parent)
{
    m_data = new QIrRemoteDeviceWatcher_Private();
    m_data->fd = -1;
}

/*!
    Destroys the device watcher.  
*/
QIrRemoteDeviceWatcher::~QIrRemoteDeviceWatcher()
{
    if (m_data->fd != -1) {
        qLog(Infrared) << "Closing watcher socket";
        close(m_data->fd);
    }
}

/*!
    Starts a watch for remote devices.  The \a ms parameter specifies the
    maximum time to watch, and the \a classes parameter specifies the
    filter of the types of devices we're interested in.

    NOTE: Due to the underlying system implementation, this function
    is blocking and will not return until a device has been found or the
    timeout has elapsed.

    The function will return false if the watch could not be started or
    an error occurred, and true otherwise. In case a new device was
    found within the specified timeout, the deviceFound() signal will
    be emitted and this function will return true.

    This function is designed to be called repeatedly in a loop.  It is
    designed to be used as a low-power, passive watcher of remote
    devices, such that a user can be notified when a new device has come
    into range.

    \sa deviceFound()
*/
bool QIrRemoteDeviceWatcher::watch(int ms, QIr::DeviceClasses classes)
{
    if (m_data->fd == -1) {
        // Try to open an Infrared socket
        qLog(Infrared) << "Opening watcher Infrared socket";
        m_data->fd = socket(AF_IRDA, SOCK_STREAM, 0);
    }

    if ( m_data->fd == -1 )
        return false;

    // Try to set the hint bits, this should almost always work.
    unsigned char hints[4];

    hints[0] = 0;
    hints[1] = 0;
    hints[2] = 0;
    hints[3] = 0;

    convert_to_hints(classes, hints);

    qLog(Infrared) << "Setting the hint mask bits to: " << classes;
    int status = setsockopt(m_data->fd, SOL_IRLMP,
                            IRLMP_HINT_MASK_SET, hints, sizeof(hints));
    qLog(Infrared) << "Success: " << (status == 0);

    if (status != 0)
        return false;

    qLog(Infrared) << "Watching...";
    socklen_t len = sizeof(ms);
    if ( getsockopt(m_data->fd, SOL_IRLMP, IRLMP_WAITDEVICE, &ms, &len) != 0 ) {
        if (errno != EAGAIN) {
            qLog(Infrared) << "Watching failed with: " << strerror(errno);
            close(m_data->fd);
            m_data->fd = -1;
            return false;
        }

        return true;
    }

    emit deviceFound();

    return true;
}

/*!
    \fn void QIrRemoteDeviceWatcher::deviceFound()

    This signal is emitted whenever a new infrared device comes into range.
 */
