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

#ifndef QABSTRACTIPCINTERFACE_H
#define QABSTRACTIPCINTERFACE_H

#include <qtopiaipcadaptor.h>

class QAbstractIpcInterfacePrivate;
class QAbstractIpcInterfaceGroup;

class QTOPIABASE_EXPORT QAbstractIpcInterface : public QObject
{
    Q_OBJECT
    friend class QAbstractIpcInterfaceGroup;
public:
    enum Mode
    {
        Client,
        Server,
        Invalid
    };

    QAbstractIpcInterface
        ( const QString& valueSpaceLocation,
          const QString& interfaceName,
          const QString& groupName = QString(),
          QObject *parent = 0, QAbstractIpcInterface::Mode mode = Client );
    ~QAbstractIpcInterface();

    QString groupName() const;
    QString interfaceName() const;
    QAbstractIpcInterface::Mode mode() const;
    bool available() const { return ( mode() != Invalid ); }

protected:
    enum SyncType
    {
        Immediate,
        Delayed
    };

    void setPriority( int value );

    void proxy( const QByteArray& member );
    void proxyAll( const QMetaObject& meta );
    void proxyAll( const QMetaObject& meta, const QString& subInterfaceName );

    QtopiaIpcSendEnvelope invoke( const QByteArray& name );
    void invoke( const QByteArray& name, QVariant arg1 );
    void invoke( const QByteArray& name, QVariant arg1, QVariant arg2 );
    void invoke( const QByteArray& name, QVariant arg1, QVariant arg2, QVariant arg3 );
    void invoke( const QByteArray& name, const QList<QVariant>& args );

    void setValue
        ( const QString& name, const QVariant& value,
          QAbstractIpcInterface::SyncType sync=Immediate );
    QVariant value
        ( const QString& name, const QVariant& def = QVariant() ) const;
    void removeValue
        ( const QString& name,
          QAbstractIpcInterface::SyncType sync=Immediate );
    QList<QString> valueNames( const QString& path = QString() ) const;

    virtual void groupInitialized( QAbstractIpcInterfaceGroup *group );

    void connectNotify( const char *signal );

signals:
    void disconnected();

private slots:
    void remoteDisconnected();

private:
    QAbstractIpcInterfacePrivate *d;
};

#endif // QABSTRACTIPCINTERFACE_H
