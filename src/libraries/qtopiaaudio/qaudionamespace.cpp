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

#include "qaudionamespace.h"

/*!
    \class QAudio
    \mainclass
    \brief The QAudio namespace provides a container for miscellaneous Audio functionality.

    The QAudio namespace defines various functions and enums that
    are used globally by the QAudio library.
  */

/*!
    \enum QAudio::AudioCapability
    Defines the types of audio features that a particular audio device
    is capable of.  The QAudio::AudioCapabilities defines all capabilities
    supported by the audio device.  For instance, InputOnly | OutputOnly
    signifies that the Audio Device is capable of Input or Output capability
    at a particular time, but not both.  InputOnly | OutputOnly | InputAndOuput
    specifies that the device can be used in full duplex or half duplex modes.

    \value None No capabilities.
    \value InputOnly Only input capability is supported.
    \value OutputOnly Only output capability is supported.
    \value InputAndOutput Both input and output capabilities are supported.
*/
