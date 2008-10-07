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

#ifndef QMODEMNETWORKREGISTRATION_H
#define QMODEMNETWORKREGISTRATION_H

#include <qnetworkregistration.h>
#include <qatresultparser.h>

class QModemService;
class QModemNetworkRegistrationPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemNetworkRegistration
        : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit QModemNetworkRegistration( QModemService *service );
    ~QModemNetworkRegistration();

    bool supportsOperatorTechnology() const;
    void setSupportsOperatorTechnology( bool value );

    static QString operatorNameForId( const QString& id );

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

protected slots:
    virtual void resetModem();
    void queryRegistration();

private slots:
    void cregNotify( const QString& msg );
    void cregTimeOut();
    void cregQuery( bool ok, const QAtResult& result );
    void copsDone( bool ok, const QAtResult& result );
    void copsNumericDone( bool ok, const QAtResult& result );
    void setDone( bool ok, const QAtResult& result );
    void availDone( bool ok, const QAtResult& result );
    void cfunDone();

protected:
    virtual QString setCurrentOperatorCommand
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology );

private:
    QModemNetworkRegistrationPrivate *d;

    void parseAvailableOperator
            ( QList<QNetworkRegistration::AvailableOperator>&list,
              const QList<QAtResultParser::Node>& values );
    void queryCurrentOperator();
};

#endif /* QMODEMNETWORKREGISTRATION_H */
