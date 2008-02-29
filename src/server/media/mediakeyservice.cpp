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


#include "audiovolumemanager.h"
#include "mediakeyservice.h"


MediaKeyService::MediaKeyService(AudioVolumeManager* avm):
    m_increment(INCREMENT),
    m_repeatTimerId(-1),
    m_repeatKeyCode(-1),
    m_avm(avm)
{
    QWSServer::addKeyboardFilter(this);
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
//    qDebug() << "MediaKeyService::filter" << unicode << keycode << modifiers << press << autoRepeat;

    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);

    bool    rc = false;

    // TODO: Configurable key/function matching
    if (keycode == Qt::Key_VolumeUp || keycode == Qt::Key_VolumeDown)
    {
        if (m_repeatKeyCode != -1 && autoRepeat)
        {
//            qDebug() << "snaffling key due to autoRepeat";
            rc = true;
        }
        else
        {
            rc = m_avm->canManageVolume();

            if (rc)
            {
                if (autoRepeat)
                {
//                    qDebug() << "starting auto repeat timer";
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
                            break;

                        case Qt::Key_VolumeDown:
                            m_avm->decreaseVolume(m_increment);
                            break;
                        }
                    }
                    else
                    {
                        if (m_repeatKeyCode != -1)
                        {
//                            qDebug() << "killing autorepeat timer";
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

    return rc;
}

void MediaKeyService::timerEvent(QTimerEvent* timerEvent)
{
    if (m_repeatTimerId == timerEvent->timerId())
    {
//        qDebug() << "timeEvent()" << m_repeatKeyCode;

        switch (m_repeatKeyCode)
        {
        case Qt::Key_VolumeUp:
            m_avm->increaseVolume(m_increment);
            break;

        case Qt::Key_VolumeDown:
            m_avm->decreaseVolume(m_increment);
            break;
        }

        m_increment += INCREMENT;
    }
}


