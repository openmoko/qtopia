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

#ifndef __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H
#define __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H


#include <qobject.h>
#include "qtopiainputevents.h"


class AudioVolumeManager;
class QValueSpaceItem;

class MediaKeyService :
    public QObject,
    public QtopiaKeyboardFilter
{
    Q_OBJECT

    static const int INCREMENT = 1;

public:
    MediaKeyService(AudioVolumeManager* avm);
    ~MediaKeyService();

    void setVolume(bool up);

signals:
    void volumeChanged(bool up);

private:
    bool filter(int unicode, int keycode, int modifiers,
                bool press, bool autoRepeat);

    void timerEvent(QTimerEvent* timerEvent);

    bool keyLocked();

    int                 m_increment;
    int                 m_repeatTimerId;
    int                 m_repeatKeyCode;
    AudioVolumeManager* m_avm;
    bool                m_keyLocked;
    QValueSpaceItem*    m_vs;
};

#endif  // __QTOPIA_SERVER_MEDIA_MEDIAKEYSERVICE_H
