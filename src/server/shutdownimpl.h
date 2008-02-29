/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef SHUTDOWNIMPL_H
#define SHUTDOWNIMPL_H

#include "shutdown.h"

class QTimer;

class ShutdownImpl : public Shutdown
{
    Q_OBJECT
public:
    ShutdownImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

    enum Type { ShutdownSystem, RebootSystem, RestartDesktop, TerminateDesktop };

signals:
    void shutdown( ShutdownImpl::Type );

private slots:
    void buttonClicked( int );
    void cancelClicked();
    void timeout();

private:
    QTimer *timer;
    int progress;
    Type operation;
};

#endif

