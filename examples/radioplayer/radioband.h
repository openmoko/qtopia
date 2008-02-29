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

#ifndef RADIOBAND_H
#define RADIOBAND_H

#include <QObject>

class RadioBandPrivate;

class RadioBand : public QObject
{
    Q_OBJECT
public:
    typedef qint64 Frequency;

    RadioBand( const QString& name, QObject *parent = 0 );
    RadioBand( const QString& name, RadioBand::Frequency low,
               RadioBand::Frequency high, QObject *parent = 0 );
    ~RadioBand();

    static QString standardBandForFrequency( RadioBand::Frequency freq );

    virtual bool active() const = 0;
    virtual bool muted() const = 0;
    virtual RadioBand::Frequency frequency() const = 0;
    virtual bool signal() const = 0;
    virtual bool stereo() const = 0;
    virtual bool rds() const = 0;
    virtual bool signalDetectable() const = 0;
    virtual int volume() const = 0;

    QString name() const;
    RadioBand::Frequency lowFrequency() const;
    RadioBand::Frequency highFrequency() const;
    RadioBand::Frequency scanStep() const;
    RadioBand::Frequency scanOffStationStep() const;
    int scanWaitTime() const;
    bool frequencyIsChannelNumber() const;

public slots:
    virtual void setActive( bool value ) = 0;
    virtual void setMuted( bool value ) = 0;
    virtual void setFrequency( RadioBand::Frequency value ) = 0;
    virtual void adjustVolume( int diff ) = 0;

protected:
    void setLowFrequency( RadioBand::Frequency value );
    void setHighFrequency( RadioBand::Frequency value );
    void setScanStep( RadioBand::Frequency value );
    void setScanOffStationStep( RadioBand::Frequency value );
    void setScanWaitTime( int value );
    void setFrequencyIsChannelNumber( bool value );

private:
    RadioBandPrivate *d;
};

#endif /* RADIOBAND_H */
