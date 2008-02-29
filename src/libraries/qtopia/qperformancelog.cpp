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

#include "qperformancelog.h"

#include <QTime>
#include <QStringList>
#include <qtopialog.h>
#include <QApplication>

#ifdef QTOPIA_TEST
# include <QTestSlave>
#endif

class QPerformanceLogPrivate
{
public:
    QPerformanceLogPrivate();
    ~QPerformanceLogPrivate();
    void send( const QString &msg );
    void adjustTimezone( QTime &preAdjustTime );

    QTime start_time;
private:
    void sendUnsent();
    QStringList unsent;
};

static QPerformanceLogPrivate private_log;
static QPerformanceLogPrivate *d;

QPerformanceLogPrivate::QPerformanceLogPrivate()
{
    d = this;
    // set the start time. This time will be used to calculate the difference between events
    start_time = QTime::currentTime();
}

QPerformanceLogPrivate::~QPerformanceLogPrivate()
{
    unsent.clear();
    d = 0;
}

void QPerformanceLogPrivate::adjustTimezone( QTime &preAdjustTime )
{
    // TODO: Need to also compensate for timezone or time changes later in the life cycle
    int offset = preAdjustTime.msecsTo( QTime::currentTime() );
    offset = offset - (offset % 100);
    // adjust the start time so that the change of timezone doesn't effect measured values.
    start_time = start_time.addMSecs( offset );
}

void QPerformanceLogPrivate::sendUnsent()
{
#ifdef QTOPIA_TEST
    if (unsent.count() == 0) {
        return;
    }

    QTestSlave *i = QTestSlave::instance();
    if (!i || !i->isConnected()) return;

    while (!unsent.isEmpty()) {
        QTestMessage msg("Performance");
        msg["data"] = unsent.takeFirst();
        i->postMessage( msg );
    }
#endif
}

void QPerformanceLogPrivate::send( const QString &txt )
{
#ifndef QTOPIA_TEST
    Q_UNUSED(txt);
#else
    QTestSlave *i = QTestSlave::instance();
    if (!i || !i->isConnected()) {
        if (!txt.isEmpty())
            unsent << txt;
        return;
    }

    if (unsent.count() > 0)
        sendUnsent();

    if (!txt.isEmpty()) {
        QTestMessage msg("Performance");
        msg["data"] = txt;
        i->postMessage( msg );
    }
#endif
}

/*!
  \internal
  \class QPerformanceLog
  \brief The QPerformanceLog class implements a performance logging mechanism
  available to all Qtopia applications.

  The QPerformanceLog class has a similar behaviour to qDebug and qLog, but every
  message has a timestamp appended.  Also, messages can be handled in three different
  ways:
  \list
   \o QtopiaTest disabled, qLog(Performance) disabled - does nothing.
   \o QtopiaTest disabled, qLog(Performance) enabled - outputs messages via qLog.
   \o QtopiaTest enabled, qLog(performance) enabled
  \endlist

  Example:
  \code
    QPerformanceLog() << "Application started";
  \endcode

  All messages will automatically have a time identifier appended
  so that the receiving side can process the data accordingly.
*/
QPerformanceLog::QPerformanceLog( QString const &applicationName )
{
    if (qLogEnabled(Performance)) {
        if (applicationName.isEmpty() && qApp) buf += qApp->applicationName();
        else                                   buf += applicationName;
        buf += QString(" : %1 : ").arg(d->start_time.msecsTo(QTime::currentTime()));
    }
}

QPerformanceLog::~QPerformanceLog()
{
    if (!qLogEnabled(Performance) || buf.isEmpty()) return;

#ifdef QTOPIA_TEST
    if (d != 0)
        d->send( buf );
#endif

    qLog(Performance) << qPrintable(buf);
}

QPerformanceLog &QPerformanceLog::operator<<(const char*t) {
    if (qLogEnabled(Performance))
        buf+=t;
    return *this;
}

void QPerformanceLog::adjustTimezone( QTime &preAdjustTime )
{
#ifdef QTOPIA_TEST
    if (d)
        d->adjustTimezone(preAdjustTime);
#endif
}

