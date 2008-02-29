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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H
#define __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H

#include <QObject>
#include <QString>

#include "media.h"


class QMediaContent;


class QMediaControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaControl(QMediaContent* mediaContent);
    ~QMediaControl();

    QtopiaMedia::State playerState() const;
    quint32 length() const;
    quint32 position() const;

    bool isMuted() const;
    int volume() const;

    QString errorString() const;

    static QString name();

public slots:
    void start();
    void pause();
    void stop();
    void seek(quint32 ms);

    void setVolume(int volume);
    void setMuted(bool mute);

signals:
    void valid();
    void invalid();

    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);

private:
    Q_DISABLE_COPY(QMediaControl);

    QMediaControlPrivate*   d;
};

#endif  // __QTOPIA_MEDIALIBRARY_MEDIACONTROL_H
