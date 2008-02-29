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

#include "phonelock.h"
#include <QTimer>
#include <QApplication>
#include <QPhoneProfile>
#include <QKeyEvent>
#include <QPhoneProfileManager>
#include "phonelauncher.h"
#include "qkeyboardlock.h"

// declare BasicKeyLockPrivate
class BasicKeyLockPrivate
{
public:
    BasicKeyLockPrivate() : m_state(BasicKeyLock::Open),
                            m_lastKey(Qt::Key_unknown),
                            m_stateTimeout(2),
                            m_timerId(0),
                            m_vso(0) {}

    BasicKeyLock::State m_state;
    QString m_emergency;
    Qt::Key m_lastKey;
    int m_stateTimeout;
    int m_timerId;
    QValueSpaceObject *m_vso;
};

// define BasicKeyLock

BasicKeyLock::BasicKeyLock(QObject *parent)
: QObject(parent)
{
    d = new BasicKeyLockPrivate;
    d->m_vso = new QValueSpaceObject("/UI", this);
    d->m_vso->setAttribute("KeyLock", false);
}

BasicKeyLock::~BasicKeyLock()
{
    delete d;
    d = 0;
}

BasicKeyLock::State BasicKeyLock::state() const
{
    return d->m_state;
}

void BasicKeyLock::stopTimer()
{
    if(d->m_timerId) {
        killTimer(d->m_timerId);
        d->m_timerId = 0;
        d->m_lastKey = Qt::Key_unknown;
    }
}

void BasicKeyLock::startTimer()
{
    if(!d->m_timerId) {
        d->m_timerId = QObject::startTimer(d->m_stateTimeout * 1000);
    }
}

void BasicKeyLock::timerEvent(QTimerEvent *)
{
    stopTimer();
    if(KeyLockIncorrect == state() ||
       KeyLockToComplete == state()) {
        Q_ASSERT(d->m_emergency.isEmpty());
        setState(KeyLocked, QString());
    }
}

bool BasicKeyLock::locked() const
{
    return Open != state();
}

Qt::Key BasicKeyLock::lockKey()
{
    static int key = 0;
    if(!key) {
        if(Qtopia::hasKey(Qt::Key_Context1)) {
            key = Qt::Key_Context1;
        } else if(Qtopia::hasKey(Qt::Key_Menu)) {
            key = Qt::Key_Menu;
        } else {
            qWarning("BasicKeyLock: Cannot map lock key - using Select.");
            key = Qt::Key_Select;
        }
    }

    return (Qt::Key)key;
}

bool BasicKeyLock::emergency() const
{
    return PartialEmergencyNumber == state() || EmergencyNumber == state();
}

QString BasicKeyLock::emergencyNumber() const
{
    return d->m_emergency;
}

void BasicKeyLock::lock()
{
    if(locked()) return;

    Q_ASSERT(d->m_emergency.isEmpty());
    setState(KeyLocked, QString());
    d->m_vso->setAttribute("KeyLock", true);
}

void BasicKeyLock::unlock()
{
    if(!locked()) return;
    d->m_lastKey = Qt::Key_unknown;
    setState(Open, QString());
    d->m_vso->setAttribute("KeyLock", false);
}

void BasicKeyLock::processKeyEvent(QKeyEvent *e)
{
    if(!locked()) {
        // We're just checking for a lock sequence
        if(e->key() == lockKey()) {
            d->m_lastKey = (Qt::Key)e->key();
            startTimer();
        } else if(d->m_lastKey == lockKey() && e->key() == Qt::Key_Asterisk) {
            stopTimer();
            emit keyLockDetected();
        }
    } else {
        // See if this is an unlock
        if(KeyLockToComplete == state() && e->key() == Qt::Key_Asterisk) {
            unlock();
        } else {
            // Check emergency dialing
            if(EmergencyNumber == state() &&
               (Qt::Key_Yes == e->key() || Qt::Key_Call == e->key())) {
                emit dialEmergency(emergencyNumber());
                setState(KeyLocked, QString());
            } else if (Qt::Key_Back == e->key() && !d->m_emergency.isEmpty()) {
                if (d->m_emergency.length() <= 1) {
                    setState(KeyLocked, QString());
                } else {
                    setState(PartialEmergencyNumber,
                            d->m_emergency.left(d->m_emergency.length()-1));
                }
            } else if (Qt::Key_Hangup == e->key()) {
                setState(KeyLocked, QString());
            } else {
                QString newEmergency = d->m_emergency;
                newEmergency.append(e->text());
                enum { None, Match, Partial } type = None;
            #ifdef QTOPIA_CELL
                if(!newEmergency.isEmpty()) {
                    QStringList emergency =
                        CellModemManager::emergencyNumbers();
                    for(int ii = 0; type == None &&
                                    ii < emergency.count(); ++ii) {
                        if(emergency.at(ii).length() < newEmergency.length()) {
                        } else if(emergency.at(ii) == newEmergency) {
                            type = Match;
                        } else if(emergency.at(ii).startsWith(newEmergency)) {
                            type = Partial;
                        }
                    }
                }
            #endif

                if(None == type) {
                    if(lockKey() == e->key())
                        setState(KeyLockToComplete, QString());
                    else
                        setState(KeyLockIncorrect, QString());
                } else if(Partial == type) {
                    setState(PartialEmergencyNumber, newEmergency);
                } else if(Match == type) {
                    setState(EmergencyNumber, newEmergency);
                }
            }
        }
    }
    d->m_lastKey = (Qt::Key)e->key();
}

void BasicKeyLock::setState(State state, const QString &e)
{
    if(d->m_state != state || d->m_emergency != e) {
        d->m_state = state;
        d->m_emergency = e;
        if(d->m_state == KeyLockIncorrect ||
           d->m_state == KeyLockToComplete) {
            startTimer();
        } else {
            stopTimer();
        }
        emit stateChanged(d->m_state, d->m_emergency);
    }
}

void BasicKeyLock::setStateTimeout(int timeout)
{
    d->m_stateTimeout = timeout;
}

int BasicKeyLock::stateTimeout()
{
    return d->m_stateTimeout;
}

void BasicKeyLock::reset()
{
    d->m_lastKey = Qt::Key_unknown;
    d->m_emergency = QString();
}

#ifdef QTOPIA_CELL

// declare BasicSimPinLockPrivate
class BasicSimPinLockPrivate
{
public:
    BasicSimPinLockPrivate()
        : m_state(BasicSimPinLock::Open),
          m_cell(0) {}
    BasicSimPinLock::State m_state;
    CellModemManager *m_cell;

    QString m_number;
    QString m_puk;
};

// define BasicSimPinLock
BasicSimPinLock::BasicSimPinLock(QObject *parent)
: QObject(parent)
{
    d = new BasicSimPinLockPrivate;
    d->m_cell = qtopiaTask<CellModemManager>();
    if(d->m_cell) {
        QObject::connect(d->m_cell, SIGNAL(stateChanged(CellModemManager::State,CellModemManager::State)), this, SLOT(cellStateChanged(CellModemManager::State)));

        if(CellModemManager::Initializing == d->m_cell->state()) {
            d->m_state = Waiting;
        }
    }
}

BasicSimPinLock::~BasicSimPinLock()
{
    delete d;
    d = 0;
}

BasicSimPinLock::State BasicSimPinLock::state() const
{
    return d->m_state;
}

QString BasicSimPinLock::number() const
{
    return d->m_number;
}

bool BasicSimPinLock::emergency() const
{
    return PartialEmergencyNumber == state() || EmergencyNumber == state();
}

bool BasicSimPinLock::open() const
{
    return Open == state();
}

void BasicSimPinLock::cellStateChanged(CellModemManager::State cellState)
{
    if(d->m_state == PartialEmergencyNumber || d->m_state == EmergencyNumber)
        return;

    State newState = stateFromCellState(cellState);

    if(newState != d->m_state) {
        d->m_number.clear();
        d->m_state = newState;
        emit stateChanged(d->m_state, d->m_number);
    }
}

BasicSimPinLock::State
BasicSimPinLock::stateFromCellState(CellModemManager::State cellState)
{
    State newState = d->m_state;

    switch(cellState) {
        case CellModemManager::Initializing:
            if(newState != Waiting)
                newState = Open;
            break;
        case CellModemManager::SIMMissing:
        case CellModemManager::FailureReset:
        case CellModemManager::UnrecoverableFailure:
        case CellModemManager::SIMDead:
        case CellModemManager::AerialOff:
        case CellModemManager::Ready:
        case CellModemManager::Initializing2:
        case CellModemManager::NoCellModem:
            newState = Open;
            break;
        case CellModemManager::WaitingSIMPin:
            newState = SimPinRequired;
            break;
        case CellModemManager::VerifyingSIMPin:
            newState = VerifyingSimPin;
            break;
        case CellModemManager::WaitingSIMPuk:
            if(newState != NewSimPinRequired)
                newState = SimPukRequired;
            break;
        case CellModemManager::VerifyingSIMPuk:
            newState = VerifyingSimPuk;
            break;
    }

    return newState;
}

Qt::Key BasicSimPinLock::lockKey()
{
    static int key = 0;
    if(!key) {
        if(Qtopia::hasKey(Qt::Key_Context1)) {
            key = Qt::Key_Context1;
        } else if(Qtopia::hasKey(Qt::Key_Menu)) {
            key = Qt::Key_Menu;
        } else {
            qWarning("BasicSimPinLock: Cannot map lock key - using Select.");
            key = Qt::Key_Select;
        }
    }

    return (Qt::Key)key;
}

void BasicSimPinLock::processKeyEvent(QKeyEvent *e)
{
    if(Open == state())
        return;

    Qt::Key key = (Qt::Key)e->key();

    // Check for emergency number
    if(state() == EmergencyNumber &&
       (Qt::Key_Call == key || Qt::Key_Yes == key || lockKey() == key)) {

        emit dialEmergency(number());
        d->m_number = QString();
        d->m_state = stateFromCellState(d->m_cell->state());
        emit stateChanged(d->m_state, d->m_number);

    } else {

        QString newNumber = d->m_number;
        State newState = d->m_state;

        // Key_NumberSign (#), is required for GCF compliance.
        // GSM 02.30, section 4.6.1, Entry of PIN and PIN2.
        if((key == lockKey() || key == Qt::Key_NumberSign) && !d->m_number.isEmpty()) {
            // Submit number
            Q_ASSERT(SimPinRequired == state() ||
                     SimPukRequired == state() ||
                     NewSimPinRequired == state());
            Q_ASSERT(d->m_cell);

            if(SimPinRequired == state()) {
                Q_ASSERT(CellModemManager::WaitingSIMPin == d->m_cell->state());
                QString pin = d->m_number;
                d->m_number.clear();
                d->m_cell->setSimPin(pin);
                Q_ASSERT(VerifyingSimPin == state());
                Q_ASSERT(CellModemManager::VerifyingSIMPin == d->m_cell->state());
            } else if(SimPukRequired == state()) {
                Q_ASSERT(CellModemManager::WaitingSIMPuk == d->m_cell->state());
                d->m_puk = d->m_number;
                d->m_number = QString();
                d->m_state = NewSimPinRequired;
                emit stateChanged(d->m_state, d->m_number);
            } else if(NewSimPinRequired == state()) {
                Q_ASSERT(CellModemManager::WaitingSIMPuk == d->m_cell->state());
                Q_ASSERT(!d->m_puk.isEmpty());
                QString pin = d->m_number;
                QString puk = d->m_puk;
                d->m_number.clear();
                d->m_puk.clear();
                d->m_cell->setSimPuk(puk, pin);
                Q_ASSERT(VerifyingSimPuk == state());
                Q_ASSERT(CellModemManager::VerifyingSIMPuk == d->m_cell->state());
            }

        } else if(key >= Qt::Key_0 && key <= Qt::Key_9 &&
                  d->m_number.count() < 8) {
            int number = key - Qt::Key_0;
            newNumber.append(QString::number(number));
        } else if(key == Qt::Key_No) {
            newNumber.clear();
        } else if(key == Qt::Key_Back || key == Qt::Key_Cancel) {
            newNumber = newNumber.left(newNumber.length() - 1);
        }

        enum { None, Match, Partial } type = None;
        if(!newNumber.isEmpty()) {
            QStringList emergency = CellModemManager::emergencyNumbers();
            for(int ii = 0; type == None && ii < emergency.count(); ++ii) {
                if(emergency.at(ii).length() < newNumber.length()) {
                } else if(emergency.at(ii) == newNumber) {
                    type = Match;
                } else if(emergency.at(ii).startsWith(newNumber)) {
                    type = Partial;
                }
            }
        }

        if(None == type) {
            // If this was an emergency number, we need to reset the state
            if(PartialEmergencyNumber == state() ||
               EmergencyNumber == state()) {
                newState = stateFromCellState(d->m_cell->state());
            }

            if(SimPinRequired != newState && SimPukRequired != newState &&
               NewSimPinRequired != newState) {
                // We don't care unless this is a pin entry
                newNumber.clear();
            }
        } else if(Partial == type) {
            newState = PartialEmergencyNumber;
        } else if(Match == type) {
            newState = EmergencyNumber;
        }

        if(newNumber != d->m_number || newState != d->m_state) {
            d->m_number = newNumber;
            d->m_state = newState;
            emit stateChanged(d->m_state, d->m_number);
        }
    }
}

void BasicSimPinLock::reset()
{
    if(d->m_cell) {
        if(!d->m_number.isEmpty()) {
            if(PartialEmergencyNumber == state() || EmergencyNumber == state())
                // Force an invalid state so cellStateChanged() will always
                // change it
                d->m_state = (State)(EmergencyNumber + 1);
            d->m_number.clear();
            cellStateChanged(d->m_cell->state());
        }
    } else {
        Q_ASSERT(d->m_number.isEmpty());
        Q_ASSERT(Open == state());
    }
}


#endif // QTOPIA_CELL
