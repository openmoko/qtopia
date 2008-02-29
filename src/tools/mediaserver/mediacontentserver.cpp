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

#include <QMediaServerSession>

#include "qmediahandle_p.h"

#include "mediacontentserver.h"


namespace mediaserver
{

class MediaContentServerPrivate
{
public:
    QMediaServerSession*    session;
};

MediaContentServer::MediaContentServer
(
 QMediaServerSession* session,
 QMediaHandle const& handle
):
    QMediaAbstractControlServer(handle, "Session"),
    d(new MediaContentServerPrivate)
{
    d->session = session;

    setValue("controls", d->session->interfaces() << "Session");

    connect(d->session, SIGNAL(interfaceAvailable(QString)),
            this, SLOT(interfaceAvailable(QString)));

    connect(d->session, SIGNAL(interfaceUnavailable(QString)),
            this, SLOT(interfaceUnavailable(QString)));

    proxyAll();
}

MediaContentServer::~MediaContentServer()
{
    delete d;
}

//private slots:
void MediaContentServer::interfaceAvailable(const QString& name)
{
    setValue("controls", d->session->interfaces() << "Session");

    emit controlAvailable(name);
}

void MediaContentServer::interfaceUnavailable(const QString& name)
{
    setValue("controls", d->session->interfaces() << "Session");

    emit controlUnavailable(name);
}


}   // ns mediaserver
