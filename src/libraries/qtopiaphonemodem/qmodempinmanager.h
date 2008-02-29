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

#ifndef QMODEMPINMANAGER_H
#define QMODEMPINMANAGER_H

#include <qpinmanager.h>

class QModemService;
class QAtResult;
class QModemPinManagerPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemPinManager : public QPinManager
{
    Q_OBJECT
public:
    explicit QModemPinManager( QModemService *service );
    ~QModemPinManager();

    void needPin( const QString& type, QObject *target,
                  const char *pinSlot, const char *cancelSlot );

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );
    void requestLockStatus( const QString& type );
    void setLockStatus
        ( const QString& type, const QString& password, bool enabled );

private slots:
    void sendQuery();
    void sendQueryAgain();
    void cpinQuery( bool ok, const QAtResult& result );
    void cpinResponse( bool ok, const QAtResult& result );
    void cpukResponse( bool ok );
    void cpwdResponse( bool ok );
    void clckQuery( bool ok, const QAtResult& result );
    void clckSet( bool ok, const QAtResult& result );
    void lastPinTimeout();

protected:
    virtual bool emptyPinIsReady() const;
    virtual QString pinTypeToCode( const QString& type ) const;
    virtual int pinMaximum() const;

private slots:
    void simMissing();

private:
    QModemPinManagerPrivate *d;
};

#endif /* QMODEMPINMANAGER_H */
