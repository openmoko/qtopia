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

#ifndef QMODEMCALLFORWARDING_H
#define QMODEMCALLFORWARDING_H

#include <qcallforwarding.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallForwarding : public QCallForwarding
{
    Q_OBJECT
public:
    explicit QModemCallForwarding( QModemService *service );
    ~QModemCallForwarding();

public slots:
    void requestForwardingStatus( QCallForwarding::Reason reason );
    void setForwarding( QCallForwarding::Reason reason,
                        const QCallForwarding::Status& status,
                        bool enable );

private slots:
    void requestDone( bool ok, const QAtResult& result );
    void setDone( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMCALLFORWARDING_H */
