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

#ifndef QTELEPHONYSERVICE_H
#define QTELEPHONYSERVICE_H

#include <qabstractipcinterfacegroup.h>

class QPhoneCallProvider;
class QSMSMessage;

class QTOPIAPHONE_EXPORT QTelephonyService : public QAbstractIpcInterfaceGroup
{
    Q_OBJECT
public:
    explicit QTelephonyService( const QString& service, QObject *parent = 0 );
    ~QTelephonyService();

    QString service() const;

    QPhoneCallProvider *callProvider() const;
    void setCallProvider( QPhoneCallProvider *provider );

    void initialize();

#ifdef QTOPIA_CELL
    bool dispatchDatagram( const QSMSMessage& msg );
#endif

private:
    QPhoneCallProvider *provider;
};

#endif /* QTELEPHONYSERVICE_H */
