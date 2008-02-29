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

#ifndef _CELLBROADCASTCONTROL_H_
#define _CELLBROADCASTCONTROL_H_

#include <QObject>
#include "cellmodemmanager.h"
class QString;
class QCellBroadcast;
class QCBSMessage;

class CellBroadcastControl : public QObject
{
Q_OBJECT
public:
    CellBroadcastControl(QObject *parent = 0);

    enum Type { Popup, Background };

    static CellBroadcastControl *instance();

signals:
    void broadcast(CellBroadcastControl::Type, const QString &channel, const QString &text);

private slots:
    void cellBroadcast(const QCBSMessage &);
    void registrationChanged(QTelephony::RegistrationState);

private:
    void subscribe();
    QCellBroadcast *cb;
    bool firstSubscribe;
};

#endif // _CELLBROADCASTCONTROL_H_

