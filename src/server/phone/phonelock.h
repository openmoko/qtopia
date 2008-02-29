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

#if 0
class QPhoneLock
{
Q_OBJECT
public:
    QPhoneLock(QObject *parent = 0);

    enum LockState { Open, KeyLock, DeviceLock };
    static LockState lockState() const;

    void setKeyLockInclusion(QWidget *window);
    void setDeviceLockExclusion(QWidget *window);

    void setExcludedWindow(QWidget *window);
    bool promptForUnlock() const;
    bool promptForUnlock(const QPoint &promptHint) const;

signals:
    void lockStateChanged(LockState);

private:
    QPhoneLockPrivate *d;
};

class QPhoneLockProvider : public QObject
{
Q_OBJECT
public:
    QPhoneLockProvider(QObject *parent = 0);

    enum State { Open, FullscreenLock, PromptForLock };

    QPoint promptHint() const;
    State state() const;

signals:
    void stateChanged(State);

private:
    QPhoneLockProviderPrivate *d;
};
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
                 KeyLockToComplete,
                 PartialEmergencyNumber,
                 EmergencyNumber };
    State state() const;
    QString emergencyNumber() const;
    bool emergency() const;

    bool locked() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);
    void reset();

    void setStateTimeout(int);
    int stateTimeout();

public slots:
    void lock();
    void unlock();

signals:
    void keyLockDetected();
    void stateChanged(BasicKeyLock::State, const QString &);
    void dialEmergency(const QString &);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void stopTimer();
    void startTimer();
    void setState(State, const QString &);
    BasicKeyLockPrivate *d;
};

#ifdef QTOPIA_CELL

class BasicSimPinLockPrivate;
class BasicSimPinLock : public QObject
{
Q_OBJECT
public:
    BasicSimPinLock(QObject *parent = 0);
    virtual ~BasicSimPinLock();

    enum State { Open, Waiting, SimPinRequired, VerifyingSimPin,
                 SimPukRequired, NewSimPinRequired, VerifyingSimPuk,
                 PartialEmergencyNumber, EmergencyNumber };

    State state() const;
    QString number() const;
    bool open() const;
    bool emergency() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);
    void reset();
signals:
    void stateChanged(BasicSimPinLock::State, const QString &);
    void dialEmergency(const QString &);

private slots:
    void cellStateChanged(CellModemManager::State newState);

private:
    State stateFromCellState(CellModemManager::State);
    BasicSimPinLockPrivate *d;
};

#endif // QTOPIA_CELL

#endif // _PHONELOCK_H_
