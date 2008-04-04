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

#ifndef FICGTA01HARDWARE_H
#define FICGTA01HARDWARE_H

#ifdef QT_QWS_FICGTA01

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>

#include <linux/input.h>

class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class QSocketNotifier;
class QtopiaIpcAdaptor;
class QSpeakerPhoneAccessoryProvider;


/**
 * Start of a generic implementation to deal with the linux input event
 * handling. Open devices by physical address and later by name, product id
 * and vendor id
 */
class FicLinuxInputEventHandler : public QObject {
    Q_OBJECT

public:
    FicLinuxInputEventHandler(QObject* parent);
    bool openByPhysicalBus(const QByteArray&);
    bool openByName(const QByteArray&);

Q_SIGNALS:
    void inputEvent(struct input_event&);

private Q_SLOTS:
    void readData();

private:
    bool internalOpen(int request, int length, const QByteArray&);

private:
    int m_fd;
    QSocketNotifier* m_notifier;
};


class Ficgta01Hardware : public QObject
{
    Q_OBJECT

public:
    Ficgta01Hardware();
    ~Ficgta01Hardware();

private Q_SLOTS:
    void inputEvent(struct input_event&);

private:
    QValueSpaceObject m_vsoPortableHandsfree;
    FicLinuxInputEventHandler *m_handler;
};

#endif // QT_QWS_FICGTA01

#endif // FICGTA01HARDWARE_H
