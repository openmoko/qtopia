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

#ifndef ACTIVITYMONITOR_H
#define ACTIVITYMONITOR_H

#include <private/observer_p.h>

#include <QtCore>
#include <qtopiaglobal.h>

class QTOPIAMEDIA_EXPORT ActivityMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ActivityMonitor( int interval, QObject* parent = 0 )
        : QObject( parent), m_interval( interval ), m_active( false ), m_updateCalled( false )
    { }

    bool isActive() { return m_active; }

signals:
    void active();
    void inactive();

public slots:
    void update();

protected:
    void timerEvent( QTimerEvent* );

private:
    int m_interval;
    int m_timer;

    bool m_active;
    bool m_updateCalled;
};

#endif // ACTIVITYMONITOR_H
