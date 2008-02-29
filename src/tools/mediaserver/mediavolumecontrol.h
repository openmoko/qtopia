/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_MEDIASERVER_MEDIAVOLUMEPROVIDER_H
#define __QTOPIA_MEDIASERVER_MEDIAVOLUMEPROVIDER_H

#include <qtopiaipcadaptor.h>


namespace mediaserver
{

class SessionManager;

class MediaVolumeControlPrivate;

class MediaVolumeControl : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    MediaVolumeControl(SessionManager* sessionManager);
    ~MediaVolumeControl();

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMuted(bool mute);

private:
    MediaVolumeControlPrivate*  d;
};

} // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_MEDIAVOLUMEPROVIDER_H

