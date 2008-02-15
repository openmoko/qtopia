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

#ifndef __QAUDIONAMESPACE_H__
#define __QAUDIONAMESPACE_H__

#include <QFlags>

#include <qtopiaglobal.h>

class QString;
#ifndef Q_QDOC
namespace QAudio
{
#else
class QAudio
{
public:
#endif
    enum AudioCapability { None = 0x0, InputOnly = 0x01, OutputOnly = 0x02, InputAndOutput = 0x04 };
    Q_DECLARE_FLAGS(AudioCapabilities, AudioCapability)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAudio::AudioCapabilities)

#endif
