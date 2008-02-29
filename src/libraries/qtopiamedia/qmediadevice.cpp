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

/*!
    \class QMediaDevice
    \preliminary
    \brief The QMediaDevice class is an abstraction for Audio devices written for
    Qtopia'a media engine.

    \sa QMediaDecoder
*/


/*!
    \enum QMediaDevice::Info::DataType

    This enum specifies the type of data produced by the QMediaDevice.

    \value Raw This is raw uncooked data.
    \value PCM This is PCM data.
*/

/*!
    \class QMediaDevice::Info

    This structure contains information on the type and characteristics of the
    media stream that this QMediaDevice produces.

    When the stream type is Raw - the dataSize member will indicate the
    complete size of the data available to the device, or -1 if the size is not
    available.

    When the stream type is PCM - the PCM data will have a frequency, sample
    size, and number of channels corresponding to the frequency, bitsPerSample
    and channels members. The expected volume of the data is set with the
    volume member.
*/

/*!
    \fn QMediaDevice::Info QMediaDevice::dataType() const;

    Return information about the Data produced by this QMediaDevice.
*/

/*!
    \fn bool QMediaDevice::connectToInput(QMediaDevice* input);

    The device is being connected to the \a input to be used as a source of data.
    Returns true if the connection was successful; false otherwise.
*/

/*!
    \fn void QMediaDevice::disconnectFromInput(QMediaDevice* input);

    The QMediaDevice is being disconnected from \a input source of data.
*/
