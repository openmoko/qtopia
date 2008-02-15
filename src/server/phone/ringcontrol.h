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

#ifndef _RINGCONTROL_H_
#define _RINGCONTROL_H_

#include <QObject>
#include <QTime>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include "qtopiaserverapplication.h"

class QSoundControl;
class QWSSoundClient;

class RingControlPrivate;
class RingControl : public QObject
{
Q_OBJECT
public:
    RingControl(QObject *parent = 0);
    virtual ~RingControl();

    enum RingType {
        NotRinging,
        Call,
        Msg
    };

    void setCallRingEnabled(bool);
    bool callRingEnabled() const;
    void setMessageRingEnabled(bool);
    bool messageRingEnabled() const;

    RingType ringType() const;
    int ringTime() const;
    void playSound( const QString &soundFile );
    void setVolume(int vol);

    void setVibrateDuration(int);
    int vibrateDuration() const;
    void setMsgRingTime(int);
    int msgRingTime() const;
    void stopRing();

public slots:
    void muteRing();
    void stopMessageAlert();

signals:
    void ringTypeChanged(RingControl::RingType);

private slots:
    void stateChanged();
    void nextRing();
    void profileChanged();
    void videoRingtoneFailed();
    void startMessageRingtone();
    void stopMessageRingtone();
    void startRingtone(const QString&);
    void stopRingtone(const QString&);

private:
    virtual void timerEvent(QTimerEvent *e);
    void startRinging(RingType);
    void initSound();
    void setSoundPriority(bool priorityPlay);

    QString findRingTone();

    RingControlPrivate *d;
};

QTOPIA_TASK_INTERFACE(RingControl);
#endif // _RINGCONTROL_H_
