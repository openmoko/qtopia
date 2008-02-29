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

#ifndef _ALARMCONTROL_H_
#define _ALARMCONTROL_H_

#include <QObject>
#include <qvaluespace.h>
#include <qtopiaipcenvelope.h>

class AlarmControl : public QObject
{
Q_OBJECT
public:
    static AlarmControl *instance();

    bool alarmState() const;

signals:
    void alarmStateChanged(bool);

private slots:
    void alarmMessage(const QString& message, const QByteArray&);

private:
    void alarmEnabled(bool);
    bool alarmOn;
    QValueSpaceObject alarmValueSpace;
    QtopiaChannel alarmChannel;
    AlarmControl();
};

#endif // _ALARMCONTROL_H_

