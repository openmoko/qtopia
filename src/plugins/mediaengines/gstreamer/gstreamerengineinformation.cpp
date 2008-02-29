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

#include "gstreamerurisessionbuilder.h"

#include "gstreamerengineinformation.h"


namespace gstreamer
{

class EngineInformationPrivate
{
public:
    Engine*                     engine;
    QMediaSessionBuilderList    sessionBuilders;
};

/*!
    \class gstreamer::EngineInformation
    \internal
*/

EngineInformation::EngineInformation(Engine* engine):
    d(new EngineInformationPrivate)
{
    d->engine = engine;
    d->sessionBuilders.append(new UriSessionBuilder(engine));
}

EngineInformation::~EngineInformation()
{
    delete d;
}


QString EngineInformation::name() const
{
    return "GStreamer";
}

QString EngineInformation::version() const
{
    return "0.1";
}


int EngineInformation::idleTime() const
{
    return -1;
}

bool EngineInformation::hasExclusiveDeviceAccess() const
{
#ifdef HAVE_OSS
    return true;
#else
    return false;
#endif
}

QMediaSessionBuilderList EngineInformation::sessionBuilders() const
{
    return d->sessionBuilders;
}

}   // ns gstreamer

