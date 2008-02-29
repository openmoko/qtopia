/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __NETMON_APPLET_H__
#define __NETMON_APPLET_H__


#include <qwidget.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qguardedptr.h>

class NetMonitor;

class NetMonitorApplet : public QWidget
{
    Q_OBJECT

public:
    NetMonitorApplet( QWidget *parent = 0 );
    ~NetMonitorApplet();

private slots:
    void netMessage( const QCString &msg, const QByteArray & );

private:
    void mousePressEvent( QMouseEvent * );
    void paintEvent( QPaintEvent* );

    void chooseNet();
    void ensureMonitor();

    enum NetState { Unknown, Unavailable, Available, Running };
    void setNetState(NetState);
    void updateSize();

    NetMonitor* netmon;
    QPixmap up,down;
    NetState state;
    bool wasavailable;
    QStringList services;
    bool demand, demand_active;
};


#endif // __NETMON_APPLET_H__

