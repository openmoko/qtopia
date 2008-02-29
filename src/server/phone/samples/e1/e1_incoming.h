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

#ifndef _e1_INCOMING_H_
#define _e1_INCOMING_H_

#include "e1_dialog.h"
#include <qphonecallmanager.h>
class QLabel;

class E1Incoming : public E1Dialog
{
Q_OBJECT
public:
    E1Incoming();

signals:
    void showCallscreen();

private slots:

    void missedIncomingCall(const QPhoneCall&);
    void callIncoming( const QPhoneCall &call );
    void ignore();
    void busy();
    void answer();

private:
    void updateLabels();
    QPhoneCall currentCall;
    QLabel *image;
    QString m_lastId;
    QLabel *name;
};

#endif // _e1_INCOMING_H_
