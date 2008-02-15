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

#include <qstring.h>
#include <qstringlist.h>

#ifndef SXEMONQLOG_H
#define SXEMONQLOG_H

class SxeMonQLog
{
#ifdef TEST_SXEMONITOR
public:
#else
private:
#endif
    enum QLogMessage{
                    LogPathSet,
                    ForceLog,
                    SuccessInit,
                    ForceRot,
                    DelayMsgPrep,
                    MsgDispatched,
                    DelayMsgDispatched,
                    KillApplication,
                    Lockdown,
                    Processing,
                    BreachDetail,
                    BreachDetail2,
                    AppBreach,
                    KernelBreach,
                    LogPathSize,
                    ProcAlreadyTerminated
                    };

    static QString getQLogString( QLogMessage, QStringList args = QStringList() );

    static QString messages[ ProcAlreadyTerminated + 1 ];

friend class SxeMonitor;
};
#endif //SXEMONQLOG_H
