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

#ifndef RADIOBANDDUMMY_H
#define RADIOBANDDUMMY_H

#include "radioband.h"
#include <QList>

class RadioBandDummy : public RadioBand
{
    Q_OBJECT
public:
    RadioBandDummy( const QString& name, RadioBand::Frequency low,
                    RadioBand::Frequency high, bool isXm,
                    QList<RadioBand::Frequency>& stations,
                    QObject *parent = 0 );
    ~RadioBandDummy();

    static void createBands( QList<RadioBand *>& bands, QObject *parent = 0 );

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
    bool _active;
    bool _muted;
    RadioBand::Frequency _frequency;
    QList<RadioBand::Frequency> stations;
    static int _volume;
};

#endif /* RADIOBANDDUMMY_H */
