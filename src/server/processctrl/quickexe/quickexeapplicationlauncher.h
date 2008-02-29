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

#ifndef _QUICKEXEAPPLICATIONLAUNCHER_H_
#define _QUICKEXEAPPLICATIONLAUNCHER_H_

#include "applicationlauncher.h"

class QuickExeApplicationLauncherPrivate;
class QuickExeApplicationLauncher : public ExeApplicationLauncher
{
Q_OBJECT
public:
    QuickExeApplicationLauncher();
    virtual ~QuickExeApplicationLauncher();

    // ApplicationTypeLauncher
    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);

private slots:
    void quickLauncherChannel(const QString &,const QByteArray &);
    void startNewQuicklauncher();
    void qlProcessExited(int);
    void qlProcessError(QProcess::ProcessError);

private:
    void respawnQuicklauncher(bool);

    static QStringList quicklaunchPlugin(const QString &);

    QuickExeApplicationLauncherPrivate *d;
};



#endif // _QUICKEXEAPPLICATIONLAUNCHER_H_

