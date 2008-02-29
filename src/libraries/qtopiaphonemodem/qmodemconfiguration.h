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

#ifndef QMODEMCONFIGURATION_H
#define QMODEMCONFIGURATION_H

#include <qtelephonyconfiguration.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemConfiguration
            : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    explicit QModemConfiguration( QModemService *service );
    ~QModemConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private slots:
    void cgmi( bool ok, const QAtResult& result );
    void cgmm( bool ok, const QAtResult& result );
    void cgmr( bool ok, const QAtResult& result );
    void cgsn( bool ok, const QAtResult& result );

private:
    QModemService *service;

    static QString fixResponse( const QString& value, const QString& prefix );
};

#endif /* QMODEMCONFIGURATION_H */
