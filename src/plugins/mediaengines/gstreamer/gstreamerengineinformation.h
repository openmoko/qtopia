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

#ifndef __GSTREAMENGINEINFORMATION_H
#define __GSTREAMENGINEINFORMATION_H

#include <qmediaengineinformation.h>
#include <qmediasessionbuilder.h>


namespace gstreamer
{

class Engine;
class EngineInformationPrivate;

class EngineInformation : public QMediaEngineInformation
{
public:
    EngineInformation(Engine* engine);
    ~EngineInformation();

    QString name() const;
    QString version() const;

    int idleTime() const;

    bool hasExclusiveDeviceAccess() const;

    QMediaSessionBuilderList sessionBuilders() const;

private:
    EngineInformationPrivate*   d;
};

}   // ns gstreamer

#endif  // __GSTREAMENGINEINFORMATION_H
