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

#ifndef __QTOIPA_GSTREAMER_ENGINE_H
#define __QTOPIA_GSTREAMER_ENGINE_H


#include <qmediaengine.h>


class QMediaEngineInformation;

namespace gstreamer
{

class EnginePrivate;

class Engine : public QMediaEngine
{
    Q_OBJECT

public:
    Engine();
    ~Engine();

    void initialize();

    void start();
    void stop();

    void suspend();
    void resume();

    QMediaEngineInformation const* engineInformation();

private:
    EnginePrivate*    d;
};

}   // ns cruxus

#endif  // __QTOPIA_GSTREAMER_ENGINE_H


