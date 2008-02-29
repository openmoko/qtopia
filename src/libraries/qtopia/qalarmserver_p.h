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

#ifndef __QALARMSERVER_P_H__
#define __QALARMSERVER_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QtopiaIpcAdaptor>
#include <QDateTime>

class AlarmServerService : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    AlarmServerService( QObject *parent = 0 )
        : QtopiaIpcAdaptor( "QPE/AlarmServer", parent ) { publishAll( Slots ); }
    ~AlarmServerService() {}

public slots:
    void addAlarm( QDateTime when, const QString& channel,
                   const QString& msg, int data );
    void deleteAlarm( QDateTime when, const QString& channel,
                      const QString& msg, int data );
    void dailyAlarmEnabled( bool flag );
};

#endif
