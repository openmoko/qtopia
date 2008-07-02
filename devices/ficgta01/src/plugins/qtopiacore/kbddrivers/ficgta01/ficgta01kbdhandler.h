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

#ifndef FICGTA01KBDHANDLER_H
#define FICGTA01KBDHANDLER_H

#ifdef QT_QWS_FICGTA01

#include <QObject>
#include <QWSKeyboardHandler>
#include <QDebug>

#include <QValueSpaceItem>
//#include <qvibrateaccessory.h>
#include <QtopiaIpcAdaptor>
#include <QTimer>

class QSocketNotifier;


/**
 * Start of a generic implementation to deal with the linux input event
 * handling. Open devices by physical address and later by name, product id
 * and vendor id
 */
class FicLinuxInputEventHandler : public QObject
{
    Q_OBJECT

public:
    FicLinuxInputEventHandler(QObject* parent);
    bool openByPhysicalBus(const QByteArray&);
    bool openByName(const QByteArray&);
    bool openById(const struct input_id&);

Q_SIGNALS:
    void inputEvent(struct input_event&);

private slots:
    void readData();

private:
    bool internalOpen(unsigned request, int length, const QByteArray&, struct input_id const * = 0);

private:
    int m_fd;
    QSocketNotifier* m_notifier;
};



class Ficgta01KbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT

public:
    Ficgta01KbdHandler();
    ~Ficgta01KbdHandler();

private:
    QSocketNotifier *auxNotify;
    QSocketNotifier *powerNotify;
    bool shift;
    QTimer *keytimer;

    QtopiaIpcAdaptor *mgr;
    QValueSpaceItem *m_headset;
     
    FicLinuxInputEventHandler *auxHandler;
    FicLinuxInputEventHandler *powerHandler;
     
    private slots:
    void inputEvent(struct input_event&);
    void timerUpdate();
};

#endif // QT_QWS_FICGTA01

#endif
