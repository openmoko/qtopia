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

#include "qmediaseekcontrolserver.h"


// {{{ QMediaSeekDelegate
QMediaSeekDelegate::~QMediaSeekDelegate()
{
}
// }}}


// {{{ QMediaSeekControlServer
QMediaSeekControlServer::QMediaSeekControlServer
(
 QMediaHandle const& handle,
 QMediaSeekDelegate* seekDelegate,
 QObject* parent
):
    QMediaAbstractControlServer(handle, "Seek", parent),
    m_seekDelegate(seekDelegate)
{
    proxyAll();

    connect(m_seekDelegate, SIGNAL(positionChanged(quin32)),
            this, SIGNAL(positionChanged(quint32)));
}

QMediaSeekControlServer::~QMediaSeekControlServer()
{
}

//public slots:
void QMediaSeekControlServer::seek(quint32 position, QtopiaMedia::Offset offset)
{
    m_seekDelegate->seek(position, offset);
}

void QMediaSeekControlServer::jumpForward(quint32 ms)
{
    m_seekDelegate->jumpForward(ms);
}

void QMediaSeekControlServer::jumpBackwards(quint32 ms)
{
    m_seekDelegate->jumpBackwards(ms);
}

void QMediaSeekControlServer::seekToStart()
{
    m_seekDelegate->seekToStart();
}

void QMediaSeekControlServer::seekToEnd()
{
    m_seekDelegate->seekToEnd();
}

void QMediaSeekControlServer::seekForward()
{
    m_seekDelegate->seekForward();
}

void QMediaSeekControlServer::seekBackward()
{
    m_seekDelegate->seekBackward();
}
// }}}

