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

#include <qirremotedevice.h>

/*!
    \class QIrRemoteDevice
    \mainclass
    \brief The QIrRemoteDevice class represents a remote infrared device.

    QIrRemoteDevice holds information about a remote Infrared device.
    Only basic information is provided, namely the name of the remote
    device, its major device classes and the negotiated address.

    \ingroup qtopiair
    \sa QIrLocalDevice
 */

/*!
    Constructs a QIrRemoteDevice with \a name as the name,
    \a devClasses as the major device classes supported by by this
    device and address given by \a addr.

    \sa name(), deviceClasses(), address()
*/
QIrRemoteDevice::QIrRemoteDevice(const QString &name,
                                 QIr::DeviceClasses &devClasses,
                                 uint addr) :
                                 m_name(name),
                                 m_dev_class(devClasses),
                                 m_addr(addr)
{

}

/*!
    Copy constructor. Constructs the current object from the attributes of \a dev.
*/
QIrRemoteDevice::QIrRemoteDevice(const QIrRemoteDevice &dev)
{
    operator=(dev);
}

/*!
    Destroys the device object.
*/
QIrRemoteDevice::~QIrRemoteDevice()
{

}

/*!
    Assignment operator.  Assigns the contents of \a other to the contents
    of the current object.
*/
QIrRemoteDevice &QIrRemoteDevice::operator=(const QIrRemoteDevice &other)
{
    if (this == &other)
        return *this;

    m_name = other.m_name;
    m_addr = other.m_addr;
    m_dev_class = other.m_dev_class;

    return *this;
}

/*!
    Comparison operator.  Compares the contents of \a other to the contents
    of the current object.  Returns true if the contents are equal.
*/
bool QIrRemoteDevice::operator==(const QIrRemoteDevice &other) const
{
    if (this == &other)
        return true;

    if ( (m_name == other.m_name) &&
         (m_addr == other.m_addr) &&
         (m_dev_class == other.m_dev_class) ) {
        return true;
    }

    return false;
}

/*!
    Returns the address of the remote device.

    \warning This is a 32 bit integer, and due to the nature of the
    IrDA protocol this number should be considered highly dynamic.  To
    refresh the number, a new discovery must be performed. It is not
    recommended to try and reuse the address for extended periods
    of time.

    \sa QIrLocalDevice, name()
*/
uint QIrRemoteDevice::address() const
{
    return m_addr;
}

/*!
    Returns the major device classes supported by the remote device.

    \sa name()
*/
QIr::DeviceClasses QIrRemoteDevice::deviceClasses() const
{
    return m_dev_class;
}

/*!
    Returns the name of the remote device.

    \sa address(), deviceClasses()
*/
QString QIrRemoteDevice::name() const
{
    return m_name;
}
