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

#ifndef __QTOPIA_MEDIA_SEEKCONTROL_H
#define __QTOPIA_MEDIA_SEEKCONTROL_H

#include <QObject>

#include <qtopiaglobal.h>

#include "media.h"


class QMediaContent;


class QMediaSeekControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaSeekControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaSeekControl(QMediaContent* mediaContent);
    ~QMediaSeekControl();

    static QString name();

public slots:
    void seek(quint32 position, QtopiaMedia::Offset offset = QtopiaMedia::Beginning);
    void jumpForward(quint32 ms);
    void jumpBackwards(quint32 ms);
    void seekToStart();
    void seekToEnd();
    void seekForward();
    void seekBackward();

signals:
    void valid();
    void invalid();

    void positionChanged(quint32 ms);

private:
    Q_DISABLE_COPY(QMediaSeekControl);

    QMediaSeekControlPrivate*    d;
};

#endif  // __QTOPIA_MEDIALIBRARY_VIDEOCONTROL_H

