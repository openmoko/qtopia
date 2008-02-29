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

#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QtAlgorithms>

#include <qbluetoothscoserver.h>
#include <qbluetoothscosocket.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qbluetoothaddress.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <bluetooth/sco.h>
#include <fcntl.h>

class QBluetoothScoServerPrivate
{
public:
    QBluetoothAddress m_address;
};

/*!
    \class QBluetoothScoServer
    \brief The QBluetoothScoServer class represents a SCO server socket.

    This class makes it possible to accept incoming SCO connections.
 */

/*!
    Constructs a new QBluetoothScoServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QBluetoothScoServer::QBluetoothScoServer(QObject *parent)
    : QBluetoothAbstractServer(parent)
{
    m_data = new QBluetoothScoServerPrivate();
}

/*!
    Destructor.
 */
QBluetoothScoServer::~QBluetoothScoServer()
{
    if (m_data)
        delete m_data;
}

/*!
    Tells the server to listen for incoming connections on
    address \a local.

    Returns true on success; otherwise returns false.

    \sa isListening()
 */
bool QBluetoothScoServer::listen(const QBluetoothAddress &local)
{
    if (isListening()) {
        return false;
    }

    int fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);

    if (fd < 0) {
        setError(QBluetoothAbstractServer::ResourceError);
        return false;
    }

    struct sockaddr_sco addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    bool ret = initiateListen(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret)
        m_data->m_address = local;

    return ret;
}

/*!
    \reimp
 */
void QBluetoothScoServer::close()
{
    QBluetoothAbstractServer::close();

    m_data->m_address = QBluetoothAddress::invalid;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.
 */
QBluetoothAddress QBluetoothScoServer::serverAddress() const
{
    return m_data->m_address;
}

/*!
    \reimp
*/
QBluetoothAbstractSocket * QBluetoothScoServer::createSocket()
{
    return new QBluetoothScoSocket(this);
}
