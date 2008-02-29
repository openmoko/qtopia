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

#include "mediapowercontrol.h"


namespace mediaserver
{

MediaPowerControl::MediaPowerControl(QObject* parent):
    QObject(parent),
    m_powerConstraint(QtopiaApplication::Enable)
{
}

MediaPowerControl::~MediaPowerControl()
{
}

void MediaPowerControl::activeSessionCount(int activeSessions)
{
    if (activeSessions == 0)
        QtopiaApplication::setPowerConstraint(m_powerConstraint = QtopiaApplication::Enable);
    else
    {
        if (activeSessions > 0 && m_powerConstraint != QtopiaApplication::DisableSuspend)
            QtopiaApplication::setPowerConstraint(m_powerConstraint = QtopiaApplication::DisableSuspend);
    }
}

} // ns mediaserver
