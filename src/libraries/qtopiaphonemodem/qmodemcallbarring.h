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

#ifndef QMODEMCALLBARRING_H
#define QMODEMCALLBARRING_H

#include <qcallbarring.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallBarring : public QCallBarring
{
    Q_OBJECT
public:
    explicit QModemCallBarring( QModemService *service );
    ~QModemCallBarring();

public slots:
    void requestBarringStatus( QCallBarring::BarringType type );
    void setBarringStatus( QCallBarring::BarringType type,
                           const QString& password,
                           QTelephony::CallClass cls,
                           bool lock );
    void unlockAll( const QString& password );
    void unlockAllIncoming( const QString& password );
    void unlockAllOutgoing( const QString& password );
    void changeBarringPassword( QCallBarring::BarringType type,
                                const QString& oldPassword,
                                const QString& newPassword );

protected:
    virtual QString typeToString( QCallBarring::BarringType type ) const;

private slots:
    void setDone( bool ok, const QAtResult& result );
    void requestDone( bool ok, const QAtResult& result );
    void unlockDone( bool ok, const QAtResult& result );
    void cpwdDone( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMCALLBARRING_H */
