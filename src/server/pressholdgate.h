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

#ifndef _PRESSHOLDGATE_H_
#define _PRESSHOLDGATE_H_

#include <QtCore/QObject>
class QDeviceButton;

class PressHoldGate : public QObject {
    Q_OBJECT
public:
    PressHoldGate(QObject* parent)
        : QObject( parent ), held_key(0), held_tid(0), hardfilter(false) { }
    PressHoldGate(const QString& contxt, QObject* parent)
        : QObject( parent ), held_key(0), held_tid(0), context(contxt), hardfilter(false) { }
    bool filterKey(int keycode, bool pressed, bool pressable, bool holdable);
    bool filterDeviceButton(int keycode, bool pressed, bool isautorepeat);
    void setHardFilter(bool y) { hardfilter=y; }
signals:
    void activate(int keycode, bool held);
private:
    void timerEvent(QTimerEvent*);
    int held_key;
    int held_tid;
    QString context;
    bool hardfilter;
};

#endif
