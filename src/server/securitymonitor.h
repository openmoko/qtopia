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

#ifndef _SECURITYMONITOR_H_
#define _SECURITYMONITOR_H_

#include "qtopiaserverapplication.h"

#ifndef QT_NO_SXE
#include <QProcess>
#endif

class SecurityMonitor;
class SecurityMonitorTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    SecurityMonitorTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();


private slots:
    void finished();

#ifndef QT_NO_SXE
    void sxeMonitorProcessError(QProcess::ProcessError);
    void sxeMonitorProcessExited(int);
    void startNewSxeMonitor();
#endif

private:
    void doShutdown();

#ifndef QT_NO_SXE
    static QString sxemonitorExecutable();
    QProcess *m_sxeMonitorProcess;
#endif

};

#endif // _SECURITYMONITOR_H_
