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
#ifndef QPHONECALLMANAGER_H
#define QPHONECALLMANAGER_H

#include <qphonecall.h>

class QPhoneCallManagerPrivate;

class QTOPIAPHONE_EXPORT QPhoneCallManager : public QObject
{
    Q_OBJECT
public:
    explicit QPhoneCallManager( QObject *parent = 0 );
    ~QPhoneCallManager();

    QList<QPhoneCall> calls() const;

    QPhoneCall create( const QString& type );
    QPhoneCall create( const QString& type, const QString& service );

    QStringList services() const;
    QStringList services( const QString& type ) const;

    QStringList callTypes() const;
    QStringList callTypes( const QString& service ) const;

    QPhoneCall fromModemIdentifier( int id ) const;

signals:
    void newCall( const QPhoneCall& call );
    void callTypesChanged();
    void statesChanged( const QList<QPhoneCall>& calls );

private:
    QPhoneCallManagerPrivate *d;
};

#endif // QPHONECALLMANAGER_H
