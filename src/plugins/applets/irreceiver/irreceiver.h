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
#ifndef __CLIPBOARD_APPLET_H__
#define __CLIPBOARD_APPLET_H__

#include <qpe/accessory/accessory.h>

#include <qwidget.h>
#include <qpixmap.h>


class IRReceiverApplet : public QWidget
{
    Q_OBJECT
public:
    IRReceiverApplet( QWidget *parent = 0, const char *name=0 );
    ~IRReceiverApplet();

    enum State { Off, On, On5Mins /*, On1Item*/ };

private slots:
    void activate(int choice);

private:
    void timerEvent(QTimerEvent*);
    void service(const QString& command);

    Accessory* acc;
    int it[4];
    int tid;
};


#endif // __CLIPBOARD_APPLET_H__

