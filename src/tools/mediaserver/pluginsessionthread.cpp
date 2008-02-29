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

#include <qtimer.h>

#include "pluginsessionthread.h"


namespace qtopia_helix
{

class PluginSessionThread::PluginSessionThreadPrivate
{
};

PluginSessionThread::PluginSessionThread()
{
}

PluginSessionThread::~PluginSessionThread()
{
}

void PluginSessionThread::run()
{
    QTimer      timer;

    // one at at time, in future mixing device is passed?
    connect(&timer, SIGNAL(timeout()),
            this, SLOT(processAudio()));

    timer.start(2);

    exec();
}

void PluginSessionThread::processAudio()
{
}

}
