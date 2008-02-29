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

#ifndef RADIOBANDMANAGER_H
#define RADIOBANDMANAGER_H

#include "radioband.h"
#include <QList>

class RadioBandManagerPrivate;

class RadioBandManager : public QObject
{
    Q_OBJECT
public:
    RadioBandManager( QObject *parent = 0 );
    ~RadioBandManager();

    QList<RadioBand *> bands() const;

    int band() const;
    RadioBand::Frequency frequency() const;
    QString bandName() const;
    bool isValid() const { return ( band() >= 0 ); }

    bool muted() const;
    bool stereo() const;
    bool signal() const;
    bool rds() const;
    bool signalDetectable() const;
    int volume() const;

    bool scanning() const;

    RadioBand *currentBand() const;
    RadioBand *bandFromName( const QString& name ) const;

    static QString formatFrequency( RadioBand::Frequency frequency );
    static bool frequencyIsChannelNumber( RadioBand::Frequency frequency );

public slots:
    void setBand( int value );
    bool setFrequency( RadioBand::Frequency value );
    bool setFrequency( const QString& band, RadioBand::Frequency value );
    void scanForward();
    void scanBackward();
    void scanAll();
    void stopScan();
    void stepForward();
    void stepBackward();
    void setMuted( bool value );
    void adjustVolume( int diff );

signals:
    void scanProgress( RadioBand::Frequency frequency, int band );
    void scanFoundStation( RadioBand::Frequency frequency, int band );
    void scanStarted();
    void scanStopped();
    void signalChanged();

private:
    void startScan( bool forward, bool all );
    void nextScan();

private slots:
    void scanTimeout();
    void signalTimeout();
    void startSignalCheck( bool firstAfterFreqChange = true );
    void backoffSignalCheck();

private:
    RadioBandManagerPrivate *d;

    int bandFromFrequency( RadioBand::Frequency frequency ) const;
};

#endif /* RADIOBANDMANAGER_H */
