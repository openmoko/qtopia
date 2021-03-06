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

#ifndef _PHONELOCK_H_
#define _PHONELOCK_H_

#include <QObject>
#include <QString>
#include <qvaluespace.h>
#include <QKeyEvent>
#ifdef QTOPIA_CELL
#include <qpinmanager.h>
#include "cellmodemmanager.h"
#endif

class BasicKeyLockPrivate;
class BasicKeyLock : public QObject
{
Q_OBJECT
public:
    BasicKeyLock(QObject *parent = 0);
    virtual ~BasicKeyLock();

    enum State { Open,
                 KeyLocked,
                 KeyLockIncorrect,
                 KeyLockToComplete };
    State state() const;

    bool locked() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);

    void setStateTimeout(int);
    int stateTimeout();

public slots:
    void reset();
    void lock();
    void unlock();

signals:
    void keyLockDetected();
    void stateChanged(BasicKeyLock::State);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void stopTimer();
    void startTimer();
    void setState(State);
    BasicKeyLockPrivate *d;
};

#ifdef QTOPIA_CELL

class BasicEmergencyLockPrivate;
class BasicEmergencyLock : public QObject
{
    Q_OBJECT
public:
    BasicEmergencyLock(QObject *parent = 0);
    virtual ~BasicEmergencyLock();

    enum State { NoEmergencyNumber, 
                 PartialEmergencyNumber,
                 EmergencyNumber };
    State state() const;
    QString emergencyNumber() const;
    bool emergency() const;

    static Qt::Key lockKey();

    bool processKeyEvent(QKeyEvent *);

public slots:
    void reset();

signals:
    void stateChanged(BasicEmergencyLock::State, const QString &);
    void dialEmergency(const QString &);

private:
    BasicEmergencyLockPrivate *d;
};

class BasicSimPinLockPrivate;
class BasicSimPinLock : public QObject
{
Q_OBJECT
public:
    BasicSimPinLock(QObject *parent = 0);
    virtual ~BasicSimPinLock();

    enum State { Pending, Open, Waiting, SimPinRequired, VerifyingSimPin, SimPukRequired,
                 NewSimPinRequired, VerifyingSimPuk };

    State state() const;

    QString number() const;
    bool open() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);

public slots:
    void reset();

signals:
    void stateChanged(BasicSimPinLock::State, 
                      const QString &);

private slots:
    void cellStateChanged(CellModemManager::State newState);

private:
    State stateFromCellState(CellModemManager::State);
    BasicSimPinLockPrivate *d;
};

#endif // QTOPIA_CELL

#endif // _PHONELOCK_H_
