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


#ifndef __QTOPIA_ENGINELOADER_H
#define __QTOPIA_ENGINELOADER_H

#include <qobject.h>
#include <qlist.h>

#include <qmediaengine.h>


namespace mediaserver
{

class EngineLoaderPrivate;

class EngineLoader : public QObject
{
    Q_OBJECT

public:
    EngineLoader();
    ~EngineLoader();

    void load();
    void unload();

    QMediaEngineList const& engines();

private:
    EngineLoaderPrivate*    d;
};

}   // ns mediaserver

#endif  // __QTOPIA_ENGINELOADER_H

