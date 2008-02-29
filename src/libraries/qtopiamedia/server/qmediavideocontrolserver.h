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

#ifndef __QTOPIA_MEDIA_VIDEOCONTROLSERVER_H
#define __QTOPIA_MEDIA_VIDEOCONTROLSERVER_H

#include <QWSEmbedWidget>

#include "qmediahandle_p.h"
#include "qmediaabstractcontrolserver.h"

#include <qtopiaglobal.h>


class QTOPIAMEDIA_EXPORT QMediaVideoControlServer :
    public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    QMediaVideoControlServer(QMediaHandle const& handle,
                             QWidget* target = 0,
                             QObject* parent = 0);
    ~QMediaVideoControlServer();

    void setRenderTarget(QWidget* target);
    void setRenderTarget(int wid);

    void unsetRenderTarget();

signals:
    void videoTargetAvailable();
    void videoTargetRemoved();
};


#endif  // __QTOPIA_MEDIA_VIDEOCONTROLSERVER_H
