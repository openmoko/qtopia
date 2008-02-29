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

#include "sxemonqlog.h"

/*!
  \internal
  \class SxeMonQLog
  \brief produces strings for qLog outputs

  The SxeMonQLog class is a utility class to that provides strings
  for qlog output by sxemonitor.  It's purpose is to improve
  extensibility and ease of testing by centrally grouping all
  qlog outputs in a single location.
*/

/*!
  Produces a string depending on the value of \a m and accompanying
  \a args.  See implementation of SxeMonQLog::messages for the
  message types and appropriate args
*/
QString SxeMonQLog::getQLogString( QLogMessage m, QStringList args )
{
    QString s( messages[m] );
    for( int i = 0; i < args.size(); i++ )
        s = s.arg( args[i] );
    return s;
}

//ensure this is kept in sync with QLogMessage enum
QString SxeMonQLog::messages[]= {
"SxeMonitor::init() Log Path set to: %1", //LogPathSet param %1= log file path
"SxeMonitor::init() Forcing log file creation or rotation", //forceLog
"SxeMonitor:: init() Successful initialization", //SuccessInit
"SxeMonitor::logUpdated() Forced log rotation attempted", //ForceRot
"SxeMonitor::dispatchMessage() delayed message prepared", //delayMsgPrep
"SxeMonitor::dispatchMessage() message dispatched", //MsgDispatched
"SxeMonitor::dispatchMessaged() delayed message dispatched", //DelayMsgDispatched
"SxeMonitor::killApplication() killing all instances of application: %1 %2", //KillApplication param %1 = executable path, %2 = identifier
"SxeMonitor::lockdown() lockdown initiated", //Lockdown
"SxeMonitor::processLogFile() processing log", //Processing
"SxeMonitor::processingLogFile() Detected policy breach \n" //BreachDetail param %1 = errant process pid
                                 "PID: %1 \t Request: %2",  //                   %2 = qcop request
"SxeMonitor::processingLogFile()  Exe Link: %1 Name: %2", //BreachDetail2 param %1 = executable path
                                                         //                    %2 = application name
"SxeMonitor::processingLogFile() Application level breach: %1", //AppBreach %1 = log entry
"SxeMonitor::processingLogFile() Kernel level breach: %1", //KernelBreach %1 = log entry
"SxeMonitor::init() Log Size set to %1", //LogPathSize param %1 = log file size
"SxeMonitor::processingLogFile() Process has already terminated, PID: %1" //ProcAlreadyTerminated param %1 = PID
};
