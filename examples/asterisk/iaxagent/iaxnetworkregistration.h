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

#ifndef IAXNETWORKREGISTRATION_H
#define IAXNETWORKREGISTRATION_H

#include <qnetworkregistration.h>

class IaxTelephonyService;

class IaxNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit IaxNetworkRegistration( IaxTelephonyService *service );
    ~IaxNetworkRegistration();

    QString callUri() const;

public slots:
    void setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology );
    void requestAvailableOperators();
    void registrationEvent( int eventType );
    void autoRegisterToServer();
    void registerToServer();
    void deregisterFromServer();
    void updateRegistrationConfig();

private:
    IaxTelephonyService *service;
    bool pendingSetCurrentOperator;
    int registrationId;
    QString callUriValue;
};

#endif /* IAXNETWORKREGISTRATION_H */
