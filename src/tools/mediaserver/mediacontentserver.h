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

#ifndef __QTOPIA_MEDIASERVER_MEDIACONTENTSERVER_H
#define __QTOPIA_MEDIASERVER_MEDIACONTENTSERVER_H

#include <QMediaAbstractControlServer>


class QMediaServerSession;


namespace mediaserver
{

class MediaContentServerPrivate;

class MediaContentServer : public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    MediaContentServer(QMediaServerSession* session,
                       QMediaHandle const& handle);


    ~MediaContentServer();

signals:
    void controlAvailable(QString const& name);
    void controlUnavailable(QString const& name);

private slots:
    void interfaceAvailable(const QString& name);
    void interfaceUnavailable(const QString& name);

private:
    MediaContentServerPrivate*  d;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_MEDIACONTENTSERVER_H
