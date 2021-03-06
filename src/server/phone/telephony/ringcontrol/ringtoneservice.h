/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef RINGTONESERVICE_H
#define RINGTONESERVICE_H

#include <qtopiaabstractservice.h>
class RingtoneService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    RingtoneService( QObject *parent )
        : QtopiaAbstractService( "Ringtone", parent )
        { publishAll(); }

public:
    ~RingtoneService();

public slots:
    virtual void startMessageRingtone() = 0;
    virtual void stopMessageRingtone() = 0;
    virtual void startRingtone(const QString&) = 0;
    virtual void stopRingtone(const QString&) = 0;
};

#endif
