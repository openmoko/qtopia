/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qbasictimer.h"
#include "qcoreapplication.h"
#include "qabstracteventdispatcher.h"

QT_BEGIN_NAMESPACE

/*!
    \class QBasicTimer
    \brief The QBasicTimer class provides timer events for objects.

    \ingroup time
    \ingroup events

    This is a fast, lightweight, and low-level class used by Qt
    internally. We recommend using the higher-level QTimer class
    rather than this class if you want to use timers in your
    applications.

    To use this class, create a QBasicTimer, and call its start()
    function with a timeout interval and with a pointer to a QObject
    subclass. When the timer times out it will send a timer event to
    the QObject subclass. The timer can be stopped at any time using
    stop(). isActive() returns true for a timer that is running;
    i.e. it has been started, has not reached the timeout time, and
    has not been stopped. The timer's ID can be retrieved using
    timerId().

    The \l{widgets/wiggly}{Wiggly} example uses QBasicTimer to repaint
    a widget at regular intervals.

    \sa QTimer, QTimerEvent, QObject::timerEvent(), Timers, {Wiggly Example}
*/


/*!
    \fn QBasicTimer::QBasicTimer()

    Contructs a basic timer.

    \sa start()
*/
/*!
    \fn QBasicTimer::~QBasicTimer()

    Destroys the basic timer.
*/

/*!
    \fn bool QBasicTimer::isActive() const

    Returns true if the timer is running, has not yet timed
    out, and has not been stopped; otherwise returns false.

    \sa start() stop()
*/

/*!
    \fn int QBasicTimer::timerId() const

    Returns the timer's ID.

    \sa QTimerEvent::timerId()
*/

/*!
    \fn void QBasicTimer::start(int msec, QObject *object)

    Starts (or restarts) the timer with a \a msec milliseconds
    timeout.

    The given \a object will receive timer events.

    \sa stop() isActive() QObject::timerEvent()
 */
void QBasicTimer::start(int msec, QObject *obj)
{
   stop();
   if (obj)
       id = obj->startTimer(msec);
}

/*!
    Stops the timer.

    \sa start() isActive()
*/
void QBasicTimer::stop()
{
    if (id) {
        QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();
        if (eventDispatcher)
            eventDispatcher->unregisterTimer(id);
    }
    id = 0;
}

QT_END_NAMESPACE
