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


#include "audiovolumemanager.h"
#include "mediakeyservice.h"
#include "qtopiapowermanager.h"
#include <qevent.h>
#include <QValueSpaceItem>


MediaKeyService::MediaKeyService(AudioVolumeManager* avm):
    m_increment(INCREMENT),
    m_repeatTimerId(-1),
    m_repeatKeyCode(-1),
    m_avm(avm)
{
    m_vs = new QValueSpaceItem("/UI", this);
    QtopiaInputEvents::addKeyboardFilter(this);
}

MediaKeyService::~MediaKeyService()
{
}

//private:
bool MediaKeyService::filter
(
 int unicode,
 int keycode,
 int modifiers,
 bool press,
 bool autoRepeat
)
{
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);

    bool    rc = false;

    if (keyLocked())
        return rc;

    // TODO: Configurable key/function matching
    if (keycode == Qt::Key_VolumeUp || keycode == Qt::Key_VolumeDown)
    {
        if (m_repeatKeyCode != -1 && autoRepeat)
        {
            rc = true;
        }
        else
        {
            rc = m_avm->canManageVolume();

            if (rc)
            {
                if (autoRepeat)
                {
                    m_repeatKeyCode = keycode;
                    m_repeatTimerId = startTimer(200);
                }
                else
                {
                    if (press)
                    {
                        switch (keycode)
                        {
                        case Qt::Key_VolumeUp:
                            m_avm->increaseVolume(m_increment);
                            emit volumeChanged(true);
                            break;

                        case Qt::Key_VolumeDown:
                            m_avm->decreaseVolume(m_increment);
                            emit volumeChanged(false);
                            break;
                        }
                    }
                    else
                    {
                        if (m_repeatKeyCode != -1)
                        {
                            killTimer(m_repeatTimerId);
                            m_repeatKeyCode = -1;
                            m_repeatTimerId = -1;
                            m_increment = 1;
                        }
                    }
                }
            }
        }
    }

    if (rc)
        QtopiaPowerManager::setActive(false);

    return rc;
}

void MediaKeyService::timerEvent(QTimerEvent* timerEvent)
{
    if (m_repeatTimerId == timerEvent->timerId())
    {
        switch (m_repeatKeyCode)
        {
        case Qt::Key_VolumeUp:
            m_avm->increaseVolume(m_increment);
            emit volumeChanged(true);
            break;

        case Qt::Key_VolumeDown:
            m_avm->decreaseVolume(m_increment);
            emit volumeChanged(false);
            break;
        }

        m_increment += INCREMENT;
    }
}

/*! \internal */
bool MediaKeyService::keyLocked()
{
    return m_vs->value("KeyLock").toBool() || m_vs->value("SimLock").toBool();
}

void MediaKeyService::setVolume(bool up)
{
    if ( up )
        m_avm->increaseVolume(m_increment);
    else
        m_avm->decreaseVolume(m_increment);
}

