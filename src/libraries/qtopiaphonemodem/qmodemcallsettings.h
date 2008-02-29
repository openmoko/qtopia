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

#ifndef QMODEMCALLSETTINGS_H
#define QMODEMCALLSETTINGS_H

#include <qcallsettings.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallSettings : public QCallSettings
{
    Q_OBJECT
public:
    explicit QModemCallSettings( QModemService *service );
    ~QModemCallSettings();

public slots:
    void requestCallWaiting();
    void setCallWaiting( bool enable, QTelephony::CallClass cls );
    void requestCallerIdRestriction();
    void setCallerIdRestriction( QCallSettings::CallerIdRestriction clir );
    void requestSmsTransport();
    void setSmsTransport( QCallSettings::SmsTransport transport );
    void requestCallerIdPresentation();
    void requestConnectedIdPresentation();

private slots:
    void ccwaRequest( bool ok, const QAtResult& result );
    void ccwaSet( bool ok, const QAtResult& result );
    void clirRequest( bool ok, const QAtResult& result );
    void clirSet( bool ok, const QAtResult& result );
    void cgsmsRequest( bool ok, const QAtResult& result );
    void cgsmsSet( bool ok, const QAtResult& result );
    void clipRequest( bool ok, const QAtResult& result );
    void colpRequest( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMCALLSETTINGS_H */
