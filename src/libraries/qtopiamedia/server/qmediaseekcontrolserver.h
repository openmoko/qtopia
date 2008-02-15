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

#ifndef __QTOPIA_MEDIA_SEEKCONTROLSERVER_H
#define __QTOPIA_MEDIA_SEEKCONTROLSERVER_H

#include <qmediahandle_p.h>

#include "qmediaabstractcontrolserver.h"

#include <qtopiaglobal.h>

#include "media.h"

class QTOPIAMEDIA_EXPORT QMediaSeekDelegate : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaSeekDelegate();

    virtual void seek(quint32 position, QtopiaMedia::Offset offset) = 0;
    virtual void jumpForward(quint32 ms) = 0;
    virtual void jumpBackwards(quint32 ms) = 0;
    virtual void seekToStart() = 0;
    virtual void seekToEnd() = 0;
    virtual void seekForward() = 0;
    virtual void seekBackward() = 0;

signals:
    void positionChanged(quint32 ms);
};


class QTOPIAMEDIA_EXPORT QMediaSeekControlServer :
    public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    QMediaSeekControlServer(QMediaHandle const& handle,
                            QMediaSeekDelegate* seekDelegate,
                            QObject* parent = 0);
    ~QMediaSeekControlServer();

public slots:
    void seek(quint32 position, QtopiaMedia::Offset offset);
    void jumpForward(quint32 ms);
    void jumpBackwards(quint32 ms);
    void seekToStart();
    void seekToEnd();
    void seekForward();
    void seekBackward();

signals:
    void positionChanged(quint32 ms);

private:
    QMediaSeekDelegate*     m_seekDelegate;
};


#endif  // __QTOPIA_MEDIA_VIDEOCONTROLSERVER_H

