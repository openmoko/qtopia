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

#ifndef RADIOBANDV4L_H
#define RADIOBANDV4L_H

#include "radioband.h"
#include <QList>

class RadioBandV4LVolume;

class RadioBandV4L : public RadioBand
{
    Q_OBJECT
private:
    RadioBandV4L( int fd, const QByteArray& deviceName, int tuner,
                  bool v4l2, bool tunerLow, bool canClose,
                  const QString& name, RadioBand::Frequency low,
                  RadioBand::Frequency high,
                  QObject *parent = 0 );

public:
    ~RadioBandV4L();

    static bool createBands( QList<RadioBand *>& bands, QObject *parent = 0 );

    bool active() const;
    bool muted() const;
    RadioBand::Frequency frequency() const;
    bool signal() const;
    bool stereo() const;
    bool rds() const;
    bool signalDetectable() const;
    int volume() const;

public slots:
    void setActive( bool value );
    void setMuted( bool value );
    void setFrequency( RadioBand::Frequency value );
    void adjustVolume( int diff );

private:
    int _fd;
    int tuner;
    bool v4l2;
    bool tunerLow;
    bool canClose;
    bool _active;
    bool _muted;
    bool _signalDetectable;
    bool muteViaVideo;
    RadioBand::Frequency _frequency;
    QByteArray deviceName;
    RadioBandV4LVolume *volumeControl;

    void updateMute();
    int fd() const;
    void shutdownfd();
};

class RadioBandV4LVolume : public QObject
{
    Q_OBJECT
public:
    RadioBandV4LVolume( QObject *parent = 0 );
    ~RadioBandV4LVolume();

    int volume() const;
    void adjustVolume( int diff );

private:
    int savedVolume;
};

#endif /* RADIOBANDV4L_H */
