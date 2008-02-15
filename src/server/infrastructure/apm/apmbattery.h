/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _APMBATTERY_H_
#define _APMBATTERY_H_

#include <QObject>
class QPowerSourceProvider;
class APMBattery : public QObject
{
Q_OBJECT
public:
    APMBattery(QObject *parent = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void updateStatus();
    bool APMEnabled() const;

    QPowerSourceProvider *ac;
    QPowerSourceProvider *battery;
    QPowerSourceProvider *backup;
};

#endif // _APMBATTERY_H_
