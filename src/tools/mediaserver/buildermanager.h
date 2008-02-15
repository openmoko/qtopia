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

#ifndef __MEDIASERVER_BUILDERMANAGER_H
#define __MEDIASERVER_BUILDERMANAGER_H

#include <QString>

#include <QMediaSessionBuilder>


class QMediaServerSession;


namespace mediaserver
{

class BuilderManagerPrivate;

class BuilderManager
{
public:
    BuilderManager();
    ~BuilderManager();

    void addBuilders(QString const& engineName,
                     QMediaSessionBuilderList const& builderList);

    void removeBuilders(QString const& engineName,
                    QMediaSessionBuilderList const& builderList);

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

private:
    BuilderManagerPrivate*  d;
};

}   // ns mediaserver

#endif  // __MEDIASERVER_BUILDERMANAGER_H

