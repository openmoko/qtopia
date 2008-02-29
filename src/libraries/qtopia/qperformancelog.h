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
#ifndef __QPERFORMANCELOG_H__
#define __QPERFORMANCELOG_H__

#include <qtopiabase/qtopiaglobal.h>

class QTime;
class QPerformanceLogData;

class QTOPIA_EXPORT QPerformanceLog
{
public:
    enum EventType {
        NoEvent = 0x00,

        Begin  = 0x01,
        End    = 0x02,

        LibraryLoading = 0x04,
        EventLoop      = 0x08,
        MainWindow     = 0x10
    };
    Q_DECLARE_FLAGS(Event, EventType)

    QPerformanceLog( QString const &applicationName = QString() );
    ~QPerformanceLog();
    QPerformanceLog &operator<<(QString const &string);
    QPerformanceLog &operator<<(Event const &event);

    static void adjustTimezone( QTime &preAdjustTime );
    static bool enabled();
    static QString stringFromEvent(Event const &event);

private:
    Q_DISABLE_COPY(QPerformanceLog)
    QPerformanceLogData *data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPerformanceLog::Event)

#endif // __QPERFORMANCELOG_H__

