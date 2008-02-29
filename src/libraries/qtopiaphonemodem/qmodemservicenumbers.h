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

#ifndef QMODEMSERVICENUMBERS_H
#define QMODEMSERVICENUMBERS_H

#include <qservicenumbers.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemServiceNumbers : public QServiceNumbers
{
    Q_OBJECT
public:
    explicit QModemServiceNumbers( QModemService *service );
    ~QModemServiceNumbers();

public slots:
    void requestServiceNumber( QServiceNumbers::NumberId id );
    void setServiceNumber
            ( QServiceNumbers::NumberId id, const QString& number );

protected:
    void requestServiceNumberFromFile( QServiceNumbers::NumberId id );
    void setServiceNumberInFile
            ( QServiceNumbers::NumberId id, const QString& number );

private slots:
    void csvm( bool ok, const QAtResult& result );
    void csca( bool ok, const QAtResult& result );
    void cnum( bool ok, const QAtResult& result );
    void csvmSet( bool ok, const QAtResult& result );
    void cscaSet( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMSERVICENUMBERS_H */
