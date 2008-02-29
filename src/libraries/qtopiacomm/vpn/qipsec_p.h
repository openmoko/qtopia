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

#ifndef QIPSEC_P_H
#define QIPSEC_P_H

#ifndef QTOPIA_NO_IPSEC
#include "qvpnclient.h"

#include <QWidget>

class QIPSec : public QVPNClient
{
    Q_OBJECT
public:
    explicit QIPSec( QObject* parent = 0 );
    explicit QIPSec( bool serverMode, uint vpnID, QObject* parent = 0 );
    ~QIPSec();

    QVPNClient::Type type() const;
    void connect();
    void disconnect();
    QDialog* configure( QWidget* parent = 0 );
    QVPNClient::State state() const;
    QString name() const;
    void cleanup();
};
#endif //QTOPIA_NO_IPSEC
#endif //QIPSEC_P_H
