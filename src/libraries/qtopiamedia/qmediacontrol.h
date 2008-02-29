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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H
#define __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H

#include <qstring.h>
#include <qobject.h>
#include <quuid.h>

#include "media.h"
#include "qmediahandle.h"

class QMediaControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaControl(QMediaHandle const& mediaHandle, QObject* parent = 0);
    ~QMediaControl();

    QtopiaMedia::State playerState() const;
    quint32 length() const;
    quint32 position() const;

    bool isMuted() const;
    int volume() const;

    QString errorString() const;

    QMediaHandle handle() const;

    QStringList controls() const;

    static QString name();

public slots:
    void start();
    void pause();
    void stop();
    void seek(quint32 ms);

    void setVolume(int volume);
    void setMuted(bool mute);

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
    void controlAvailable(const QString& control);
    void controlUnavailable(const QString& control);

private:
    QMediaControlPrivate*   d;
};

#endif  // __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H
