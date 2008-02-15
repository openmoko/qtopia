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

#include <gst/gst.h>

#include <qmediaengineinformation.h>
#include <qmediasessionbuilder.h>

#include "gstreamerengineinformation.h"
#include "gstreamerengine.h"


namespace gstreamer
{

class EnginePrivate
{
public:
    EngineInformation*          engineInformation;
    QMediaSessionBuilderList    sessionBuilders;
};

/*!
    \class gstreamer::Engine
    \internal
*/

Engine::Engine():
    d(new EnginePrivate)
{
    d->engineInformation = new EngineInformation(this);
}

Engine::~Engine()
{
    delete d;
}


void Engine::initialize()
{
    gst_init(NULL, NULL);
//    gst_debug_set_active(TRUE);
//    gst_debug_set_default_threshold(GST_LEVEL_INFO);
}

void Engine::start()
{
}

void Engine::stop()
{
}

void Engine::suspend()
{
}

void Engine::resume()
{
}

QMediaEngineInformation const* Engine::engineInformation()
{
    return d->engineInformation;
}

}   // ns gstreamer

