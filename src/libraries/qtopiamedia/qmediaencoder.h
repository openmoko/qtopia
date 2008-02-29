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

#ifndef __QTOPIA_MEDIASERVER_MEDIAENCODER_H
#define __QTOPIA_MEDIASERVER_MEDIAENCODER_H

#include <QMediaDevice>

#include <qtopiaglobal.h>


class QMediaPipe;

class QTOPIAMEDIA_EXPORT QMediaEncoder : public QMediaDevice
{
public:
    virtual ~QMediaEncoder();

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;

    virtual quint32 length() = 0;
    virtual bool seek(qint64 ms) = 0;

    virtual void setVolume(int volume) = 0;
    virtual int volume() = 0;

    virtual void setMuted(bool mute) = 0;
    virtual bool isMuted() = 0;
};



#endif  // __QTOPIA_MEDIASERVER_MEDIAENCODER_H
