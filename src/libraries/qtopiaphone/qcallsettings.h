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

#ifndef QCALLSETTINGS_H
#define QCALLSETTINGS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallSettings : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(CallerIdRestriction CallerIdRestrictionStatus SmsTransport)
    Q_ENUMS(PresentationStatus)
public:
    explicit QCallSettings( const QString& service = QString::null,
                            QObject *parent = 0,
                            QCommInterface::Mode mode = Client );
    ~QCallSettings();

    enum CallerIdRestriction
    {
        Subscription,
        Invoked,
        Suppressed
    };

    enum CallerIdRestrictionStatus
    {
        NotProvisioned,
        Permanent,
        Unknown,
        TemporaryRestricted,
        TemporaryAllowed
    };

    enum SmsTransport
    {
        SmsTransportPD,
        SmsTransportCS,
        SmsTransportPDPreferred,
        SmsTransportCSPreferred,
        SmsTransportUnavailable
    };

    enum PresentationStatus
    {
        PresentationNotProvisioned,
        PresentationProvisioned,
        PresentationUnknown
    };

public slots:
    virtual void requestCallWaiting();
    virtual void setCallWaiting( bool enable, QTelephony::CallClass cls );
    virtual void requestCallerIdRestriction();
    virtual void setCallerIdRestriction
        ( QCallSettings::CallerIdRestriction clir );
    virtual void requestSmsTransport();
    virtual void setSmsTransport( QCallSettings::SmsTransport transport );
    virtual void requestCallerIdPresentation();
    virtual void requestConnectedIdPresentation();

signals:
    void callWaiting( QTelephony::CallClass cls );
    void setCallWaitingResult( QTelephony::Result result );
    void callerIdRestriction( QCallSettings::CallerIdRestriction clir,
                              QCallSettings::CallerIdRestrictionStatus status );
    void setCallerIdRestrictionResult( QTelephony::Result result );
    void smsTransport( QCallSettings::SmsTransport transport );
    void setSmsTransportResult( QTelephony::Result result );
    void callerIdPresentation( QCallSettings::PresentationStatus status );
    void connectedIdPresentation( QCallSettings::PresentationStatus status );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::CallerIdRestriction)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::CallerIdRestrictionStatus)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::SmsTransport)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::PresentationStatus)

#endif /* QCALLSETTINGS_H */
