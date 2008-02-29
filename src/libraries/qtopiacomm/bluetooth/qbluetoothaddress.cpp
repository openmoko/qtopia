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

#include <qbluetoothaddress.h>

#include <string.h>
#include <stdio.h>

/*!
    \class QBluetoothAddress
    \brief The QBluetoothAddress class represents a bluetooth address (bdaddr).

The QBluetoothAddress class refers to a single BluetoothAddress object.
Each Bluetooth device can have one and only one address.  There are certain
special addresses defined.  Namely \bold{any}, \bold{local}, \bold{all}.

    \ingroup qtopiabluetooth
    \sa QBluetoothLocalDevice, QBluetoothRemoteDevice
 */

/*!
    Construct a new invalid bluetooth address object.
 */
QBluetoothAddress::QBluetoothAddress()
{
    m_valid = false;
    m_bdaddr = "00:00:00:00:00:00";
}

/*!
    Construct a copy of a bluetooth address object from \a other object.
*/
QBluetoothAddress::QBluetoothAddress(const QBluetoothAddress &other)
{
    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;
}

/*!
    Construct a bluetooth address object based on string representation given by \a addr.
    The string should be in the format of XX:XX:XX:XX:XX:XX where XX represents
    a hexadecimal number.  If the string is in an invalid format, and invalid bdaddr
    is constructed.
*/
QBluetoothAddress::QBluetoothAddress(const QString &addr)
{
    unsigned int baddr[6];

    QByteArray asciiArr = addr.toAscii();
    const char *buf = asciiArr.constData();

    if (sscanf(buf, "%x:%x:%x:%x:%x:%x",
        &baddr[0], &baddr[1], &baddr[2], &baddr[3], &baddr[4], &baddr[5]) == 6) {
        m_valid = true;
        m_bdaddr = addr;
    }
    else {
        m_valid = false;
        m_bdaddr = QBluetoothAddress::invalid.m_bdaddr;
    }
}

/*!
    Destroys a Bluetooth address object.
*/
QBluetoothAddress::~QBluetoothAddress()
{
}

/*!
    Assigns the contents of the Bluetooth address object \a other to the current object.
*/
QBluetoothAddress &QBluetoothAddress::operator=(const QBluetoothAddress &other)
{
    if (this == &other)
        return *this;

    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;

    return *this;
}

/*!
    Compares the current Bluetooth address against a Bluetooth address given by \a other.
*/
bool QBluetoothAddress::operator==(const QBluetoothAddress &other) const
{
    return m_bdaddr == other.m_bdaddr;
}

/*!
    Converts the Bluetooth address object into a QString.  The format will be of the form
    XX:XX:XX:XX:XX:XX where XX is a hexadecimal number.
*/
QString QBluetoothAddress::toString() const
{
    return m_bdaddr;
}

/*!
    Returns whether the address is valid.
*/
bool QBluetoothAddress::valid() const
{
    return m_valid;
}

/*!
    \variable QBluetoothAddress::invalid
    Invalid Bluetooth address.
*/
const QBluetoothAddress QBluetoothAddress::invalid = QBluetoothAddress();

/*!
    \variable QBluetoothAddress::any
    Bluetooth address that represents a special address \bold any
*/
const QBluetoothAddress QBluetoothAddress::any = QBluetoothAddress("00:00:00:00:00:00");

/*!
    \variable QBluetoothAddress::all
    Bluetooth address that represents a special address \bold all
 */
const QBluetoothAddress QBluetoothAddress::all = QBluetoothAddress("FF:FF:FF:FF:FF:FF");

/*!
    \variable QBluetoothAddress::local
    Bluetooth address that represents a special address \bold local
*/
const QBluetoothAddress QBluetoothAddress::local = QBluetoothAddress("00:00:00:FF:FF:FF");

/*!
    \fn bool QBluetoothAddress::operator!=(const QBluetoothAddress &other) const

    Compares the current address to \a other.
    Returns true if the addresses are not equal, false if they are.
*/

/*!
    \internal
    \fn void QBluetoothAddress::serialize(Stream &stream) const
 */
template <typename Stream> void QBluetoothAddress::serialize(Stream &stream) const
{
    stream << m_bdaddr;
    stream << m_valid;
}

/*!
    \internal
    \fn void QBluetoothAddress::deserialize(Stream &stream)
 */
template <typename Stream> void QBluetoothAddress::deserialize(Stream &stream)
{
    stream >> m_bdaddr;
    stream >> m_valid;
}

Q_IMPLEMENT_USER_METATYPE(QBluetoothAddress)
