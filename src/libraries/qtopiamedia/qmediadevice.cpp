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

/*!
    \class QMediaDevice
    \preliminary
    \brief The QMediaDevice class is an abstraction for Audio devices written for
    Qtopia'a media engine.

    \sa QMediaPipe, QMediaDecoder
*/


/*!
    \fn void QMediaDevice::connectInputPipe(QMediaPipe* inputPipe);

    The device is being connected to the \a inputPipe, as a source of encoded
    data for this device.
*/

/*!
    \fn void QMediaDevice::connectOutputPipe(QMediaPipe* outputPipe);

    The device is being onnectied to the \a outputPipe as a destination for its
    data.
*/

/*!
    \fn void QMediaDevice::disconnectInputPipe(QMediaPipe* inputPipe);

    The device is being disconnected from the \a inputPipe source of data.
*/

/*!
    \fn void QMediaDevice::disconnectOutputPipe(QMediaPipe* outputPipe);

    The device is being disconnected from the \a outputPipe destination for its
    data.
*/

/*!
    \fn void QMediaDevice::setValue(QString const& name, QVariant const& value);

    Set \a name to have the \a value. This is device specific.
*/

/*!
    \fn QVariant QMediaDevice::value(QString const& name);

    Return the value of \a name, this is device specific.
*/
