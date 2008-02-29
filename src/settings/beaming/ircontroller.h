/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef __IRCONTROLLER_H__
#define __IRCONTROLLER_H__

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QIcon>

#include <qcommdevicecontroller.h>

class IRController : public QObject {
    Q_OBJECT

public:
    enum State { Off, On, On5Mins, On1Item, LastState=On1Item };

    IRController(QObject* parent);
    ~IRController();

    void setState(State s);
    State state() const;
    static QString stateDescription(State);

    int protocolCount() const;
    int currentProtocol() const;
    QString protocolName(int i) const;
    QIcon protocolIcon(int i) const;
    bool setProtocol(int i);

public slots:
    void powerStateChanged(QCommDeviceController::PowerState state);

signals:
    void stateChanged(State);

private:
    QStringList names;
    QStringList icons;
    QStringList targets;
    int curproto;
    int protocount;

    QCommDeviceController *m_device;
    State m_st;
};

#endif // __IRCONTROLLER_H__

