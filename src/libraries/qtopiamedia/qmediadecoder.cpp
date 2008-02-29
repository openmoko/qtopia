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

#include "qmediadecoder.h"


/*!
    \class QMediaDecoder
    \preliminary
    \brief The QMediaDecoder class is an abstraction for decoders written for
    Qtopia's media engine.

    \ingroup multimedia
*/


/*!
    Destroy the QMediaDecoder object.
*/

QMediaDecoder::~QMediaDecoder()
{
}

/*!
    \fn void QMediaDecoder::start();

    Start decoding data from the inputPipe

    \sa QMediaDevice
*/

/*!
    \fn void QMediaDecoder::stop();

    Stop decoding data to the outputPipe

    \sa QMediaDevice
*/

/*!
    \fn void QMediaDecoder::pause();

    Pause decoding data.
*/

/*!
    \fn quint32 QMediaDecoder::length();

    Return the length of the decoded data, to date if total not available.
*/

/*!
    \fn bool QMediaDecoder::seek(qint64 ms);

    Seek to the absolute position \a ms from the beginning of the file.
*/

/*!
    \fn void QMediaDecoder::setVolume(int volume);

    Adjust the \a volume of the data, the decoder should not attempt to adjust
    hardware volume, only amplification of the data produced by the decoder.
    The \a volume will be normalized by the system to a value between 0 and
    100, the decoder should adjust internally as necessary.
*/

/*!
    \fn int QMediaDecoder::volume();

    Return the current volume level.
*/

/*!
    \fn void QMediaDecoder::setMuted(bool mute);

    Set the \a mute state of the data, the decoder should continue to process
    audio data but not produce output, if the volume is muted.
*/

/*!
    \fn bool QMediaDecoder::isMuted();

    Return the mute state of the decoder.
*/
