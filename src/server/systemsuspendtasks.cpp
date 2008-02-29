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

#include "systemsuspend.h"
#include <QSettings>
#include <QWidget>
#include <QDesktopWidget>
#include <QPainter>
#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <qwindowsystem_qws.h>

// declare APMSuspend
class APMSuspend : public SystemSuspendHandler
{
public:
    APMSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();

private:
    bool m_hasAPM;
};
QTOPIA_DEMAND_TASK(APMSuspend, APMSuspend);
QTOPIA_TASK_PROVIDES(APMSuspend, SystemSuspendHandler);

// define APMSuspend
APMSuspend::APMSuspend()
: m_hasAPM(false)
{
    m_hasAPM = QFile::exists("/proc/apm");
}

bool APMSuspend::canSuspend() const
{
    return m_hasAPM;
}

bool APMSuspend::suspend()
{
    system("apm --suspend");
    return true;
}

bool APMSuspend::wake()
{
    return true;
}

// declare ModemSuspend
class ModemSuspend : public SystemSuspendHandler
{
Q_OBJECT
public:
    ModemSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};
QTOPIA_DEMAND_TASK(ModemSuspend, ModemSuspend);
QTOPIA_TASK_PROVIDES(ModemSuspend, SystemSuspendHandler);

// define ModemSuspend
ModemSuspend::ModemSuspend()
{
    QtopiaIpcAdaptor *suspend =
        new QtopiaIpcAdaptor( "QPE/ModemSuspend", this );
    QtopiaIpcAdaptor::connect( suspend, MESSAGE(suspendDone()),
                               this, SIGNAL(operationCompleted()) );
    QtopiaIpcAdaptor::connect( suspend, MESSAGE(wakeDone()),
                               this, SIGNAL(operationCompleted()) );
}

bool ModemSuspend::canSuspend() const
{
    return true;
}

bool ModemSuspend::suspend()
{
    QtopiaIpcEnvelope env( "QPE/ModemSuspend", "suspend()" );
    return false;
}

bool ModemSuspend::wake()
{
    QtopiaIpcEnvelope env( "QPE/ModemSuspend", "wake()" );
    return false;
}

// declare SimpleSuspend
class SimpleSuspend : public SystemSuspendHandler
{
public:
    SimpleSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();

private:
    void blankScreen();

    bool blanked;
    bool wasloggedin;
};

QTOPIA_DEMAND_TASK(SimpleSuspend, SimpleSuspend);
QTOPIA_TASK_PROVIDES(SimpleSuspend, SystemSuspendHandler);

// define SimpleSuspend
SimpleSuspend::SimpleSuspend()
: blanked(false), wasloggedin(false)
{
}

bool SimpleSuspend::canSuspend() const
{
    return true;
}

void SimpleSuspend::blankScreen()
{
#ifdef Q_WS_QWS
    QWidget w(0, (Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint));
    w.setAttribute(Qt::WA_PaintUnclipped);
    QDesktopWidget *desktop = QApplication::desktop();
    w.resize(desktop->screenGeometry(desktop->primaryScreen()).size());
    w.move(0, 0);

    QPainter p(&w);
    p.fillRect(w.rect(), QBrush(QColor(255,255,255)) );
    p.end();
    w.repaint();

    blanked = true;
#endif
}

bool SimpleSuspend::suspend()
{
    return true;
}

bool SimpleSuspend::wake()
{
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();
    }

    return true;
}

#include "systemsuspendtasks.moc"
