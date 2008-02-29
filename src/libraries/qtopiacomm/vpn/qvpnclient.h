/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef QVPNCLIENT_H
#define QVPNCLIENT_H

#include <qtopiaglobal.h>

#include <QObject>
#include <QDialog>

class QVPNClientPrivate;

class QTOPIACOMM_EXPORT QVPNClient : public QObject
{
    Q_OBJECT
public:
    enum Type {
        OpenVPN = 0,
        IPSec = 20
    };

    enum State {
        Disconnected = 0,
        Pending,
        Connected = 20
    };

    virtual ~QVPNClient();

    virtual Type type() const = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual QDialog* configure( QWidget* parent = 0 ) = 0;
    virtual State state() const = 0;
    virtual void cleanup() = 0;

    virtual QString name() const;

    QString errorString() const;
    uint id() const;

signals:
    void connectionStateChanged( bool error );

protected:
    QVPNClient( bool serverMode, QObject* parent = 0 );
    QVPNClient( bool serverMode, uint vpnID, QObject* parent = 0 );

protected:
    QVPNClientPrivate* d;
};


#endif // QVPNCLIENT_H
