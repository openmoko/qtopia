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
#ifndef Q_NETWORK_CONNECTION_H
#define Q_NETWORK_CONNECTION_H

#include <QObject>
#include <QList>
#include <QString>
#include <QUuid>

#include <qtopiaglobal.h>
#include <qtopianetwork.h>

class QNetworkConnectionManager;
class QNetworkConnectionPrivate;
class QTOPIACOMM_EXPORT QNetworkConnection : public QObject
{
    Q_OBJECT
public:
    class QTOPIACOMM_EXPORT Identity
    {
    public:
        Identity();
        explicit Identity( const QString& devHandle, const QUuid& vNetId );
        Identity( const Identity& other );
        ~Identity();
      
        //operators 
        Identity &operator=(const Identity& other);
        bool operator==(const Identity& other) const;
        bool operator!=(const Identity& other) const;

        QString deviceHandle() const; 
        QString name() const;
        QtopiaNetwork::Type type() const;
        bool isValid() const; 
    private:
        mutable QUuid vNetId;
        mutable QString devHandle;
        friend class QNetworkConnectionPrivate;
    };
    typedef QList<Identity> Identities;

    explicit QNetworkConnection( const Identity& ident, QObject* parent = 0 );
    virtual ~QNetworkConnection();
   
    Identity identity() const;
    bool isConnected() const;
    bool isValid() const;
signals:
    void connectivityChanged( bool isConnected ); 

private:
    Q_PRIVATE_SLOT( d, void _q_deviceStateChanged(QtopiaNetworkInterface::Status,bool) );
    QNetworkConnectionPrivate* d;
    friend class QNetworkConnectionPrivate;
};


class QNetworkConnectionManagerPrivate;
class QTOPIACOMM_EXPORT QNetworkConnectionManager : public QObject
{
    Q_OBJECT
public:
    QNetworkConnectionManager( QObject* parent = 0 );
    ~QNetworkConnectionManager();

    static QNetworkConnection::Identities connections();
signals:
    void connectionAdded();
    void connectionRemoved();
private:
    QNetworkConnectionManagerPrivate* d;
    Q_PRIVATE_SLOT( d, void _q_accountsAddedRemoved() );
    Q_PRIVATE_SLOT( d, void _q_accountChanged(const QString&) );
    friend class QNetworkConnectionManagerPrivate;
};
#endif //Q_NETWORK_CONNECTION_H
