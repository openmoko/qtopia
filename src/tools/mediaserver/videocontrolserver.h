/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __QTOPIA_MEDIASERVER_VIDEOCONTROLSERVER_H
#define __QTOPIA_MEDIASERVER_VIDEOCONTROLSERVER_H

#include <quuid.h>
#include <qwsembedwidget.h>

#include <qabstractipcinterface.h>


namespace mediaserver
{

class VideoControlServer : public QAbstractIpcInterface
{
    Q_OBJECT

public:

    VideoControlServer(QUuid const& id);
    ~VideoControlServer();

    void setWId(WId wid);

signals:
    void videoAvailable();
};

} // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_VIDEOCONTROLSERVER_H
