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
#include "outlookthread.h"
#include "qmapi.h"
#include "outlooksync.h"

#include <qdwin32.h>
using namespace QDWIN32;

#include <qdplugin.h>
#include <trace.h>
QD_LOG_OPTION(OutlookThread)

#include <qcopenvelope_qd.h>
#include <desktopsettings.h>

#include <QBuffer>
#include <QApplication>
#include <QXmlStreamReader>
#include <QTimer>
#include <QProcess>
#include <QPointer>

#define _WIN32_DCOM
#include <objbase.h>

// =====================================================================

OutlookThread::OutlookThread( QObject *parent )
    : QDThread( parent ), o( 0 )
{
    TRACE(OutlookThread) << "OutlookThread::OutlookThread";
}

OutlookThread::~OutlookThread()
{
    TRACE(OutlookThread) << "OutlookThread::~OutlookThread";
    quit();
    wait();
}

void OutlookThread::t_init()
{
    TRACE(OutlookThread) << "OutlookThread::t_init";

    LOG() << "CoInitialize(0)";
    HRESULT hr = CoInitialize(0);
    if ( hr != S_OK )
        return;

    o = new OutlookThreadObject;
}

void OutlookThread::t_quit()
{
    TRACE(OutlookThread) << "OutlookThread::t_quit";
    if ( !o )
        return;

    delete o;
    o = 0;

    LOG() << "CoUninitialize";
    CoUninitialize();
}

OutlookThread *OutlookThread::getInstance( QObject *syncObject )
{
    TRACE(OutlookThread) << "OutlookThread::getInstance";
    static QPointer<OutlookThread> sInstance;

    if ( sInstance.isNull() ) {
        LOG() << "Creating singleton instance";
        sInstance = new OutlookThread( syncObject );
        sInstance->start();
    }

    return sInstance;
}

// =====================================================================

OutlookThreadObject::OutlookThreadObject()
    : QObject()
{
    TRACE(OutlookThread) << "OutlookThreadObject::OutlookThreadObject";
}

OutlookThreadObject::~OutlookThreadObject()
{
    TRACE(OutlookThread) << "OutlookThreadObject::~OutlookThreadObject";
    if ( ns ) {
        LOG() << "ns->Logoff()";
        ns->Logoff();
        LOG() << "ns.Detach()->Release()";
        ns.Detach()->Release();
    }
    if ( ap ) {
        LOG() << "ap.Detach()->Release()";
        ap.Detach()->Release();
    }
}

bool OutlookThreadObject::logon()
{
    TRACE(OutlookThread) << "OutlookThreadObject::logon";

    if ( ap ) {
        // This is not the first call to logon()
        return true;
    }

    ap.CreateInstance("Outlook.Application");

    if ( ap ) {
        LOG() << "ns = ap->GetNamespace(\"MAPI\")";
        ns = ap->GetNamespace("MAPI");
        LOG() << "ns->Logon()";
        ns->Logon();
        return true;
    }

    WARNING() << "Can't connect to Outlook";
    return false;
}

#include "outlookthread.moc"
