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

#ifndef __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H
#define __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H


#include <qobject.h>
#include <qwindowsystem_qws.h>


class AudioVolumeManager;


class MediaKeyService :
    public QObject,
    public QWSServer::KeyboardFilter
{
    Q_OBJECT

    static const int INCREMENT = 1;

public:
    MediaKeyService(AudioVolumeManager* avm);
    ~MediaKeyService();

private:
    bool filter(int unicode, int keycode, int modifiers,
                bool press, bool autoRepeat);

    void timerEvent(QTimerEvent* timerEvent);

    int                 m_increment;
    int                 m_repeatTimerId;
    int                 m_repeatKeyCode;
    AudioVolumeManager* m_avm;
};

#endif  // __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H
