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

#ifndef QMODEMSUPPLEMENTARYSERVICES_H
#define QMODEMSUPPLEMENTARYSERVICES_H

#include <qsupplementaryservices.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemSupplementaryServices
            : public QSupplementaryServices
{
    Q_OBJECT
public:
    explicit QModemSupplementaryServices( QModemService *service );
    ~QModemSupplementaryServices();

public slots:
    void cancelUnstructuredSession();
    void sendUnstructuredData( const QString& data );
    void sendSupplementaryServiceData( const QString& data );

private slots:
    void resetModem();
    void cusdDone( bool ok, const QAtResult& result );
    void atdDone( bool ok, const QAtResult& result );
    void cssi( const QString& msg );
    void cssu( const QString& msg );
    void cusd( const QString& msg );

private:
    QModemService *service;
};

#endif /* QMODEMSUPPLEMENTARYSERVICES_H */
