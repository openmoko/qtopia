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
#include "regthread.h"

#include <trace.h>

using namespace QDWIN32;

// =====================================================================

RegThread::RegThread()
    : QThread()
{
    handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    quit = CreateEvent(NULL, FALSE, FALSE, NULL);
    hKey = openRegKey( HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM" );
    getPorts();
}

RegThread::~RegThread()
{
    CloseHandle( handle );
    CloseHandle( quit );
    if ( hKey )
        RegCloseKey( hKey );
}

void RegThread::run()
{
#define HANDLES 2
    HANDLE handles[HANDLES] = { handle, quit };
    for ( ;; ) {
        RegNotifyChangeKeyValue( hKey, false, REG_NOTIFY_CHANGE_LAST_SET, handle, true );
        DWORD ret = WaitForMultipleObjects( HANDLES, handles, false, INFINITE );
        if ( ret == WAIT_OBJECT_0 ) {
            getPorts();
            emit comPortsChanged();
        } else if ( ret == WAIT_OBJECT_0+1 ) {
            break;
        }
    }
}

void RegThread::getPorts()
{
    TRACE(QDLink) << "RegThread::getPorts";
    portMutex.lock();
    mPorts = QStringList();
    if ( hKey )
        mPorts = readRegKeys( hKey );
    portMutex.unlock();
}

QStringList RegThread::ports()
{
    portMutex.lock();
    QStringList ret = mPorts;
    portMutex.unlock();
    return ret;
}

