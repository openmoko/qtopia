/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_MEDIASERVER_MEDIAPIPE_H
#define __QTOPIA_MEDIASERVER_MEDIAPIPE_H

#include <qobject.h>
#include <qtopiaglobal.h>

class QIODevice;


class QMediaPipePrivate;

class QTOPIAMEDIA_EXPORT QMediaPipe : public QObject
{
    Q_OBJECT

public:
    QMediaPipe(QIODevice* inputDevice,
                QIODevice* outputDevice,
                QObject* parent = 0);
    ~QMediaPipe();

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

signals:
    void readyRead();
    void bytesWritten(qint64 bytes);

private:
    QMediaPipePrivate*   d;
};



#endif  // __QTOPIA_MEDIASERVER_MEDIAPIPE_H
