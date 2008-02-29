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

#ifndef QPHONECALLPROVIDER_H
#define QPHONECALLPROVIDER_H

#include <qphonecall.h>
#include <QFlags>

class QPhoneCallImplPrivate;
class QPhoneCallProvider;
class QPhoneCallProviderPrivate;

class QTOPIAPHONE_EXPORT QPhoneCallImpl : public QObject
{
    Q_OBJECT
public:
    QPhoneCallImpl( QPhoneCallProvider *provider, const QString& identifier,
                    const QString& callType );
    virtual ~QPhoneCallImpl();

    enum Action
    {
        None            = 0,
        Accept          = 0x0001,
        Hold            = 0x0002,
        ActivateCall    = 0x0004,
        ActivateGroup   = 0x0008,
        Join            = 0x0010,
        JoinAndDetach   = 0x0020,
        Tone            = 0x0040,
        Transfer        = 0x0080
    };
    Q_DECLARE_FLAGS(Actions, Action)

    QPhoneCallProvider& provider() const;
    QString identifier() const;
    QString callType() const;
    QPhoneCall::State state() const;
    QString number() const;
    QPhoneCallImpl::Actions actions() const;

    virtual void dial( const QDialOptions& options );
    virtual void hangup( QPhoneCall::Scope scope );
    virtual void accept();
    virtual void hold();
    virtual void activate( QPhoneCall::Scope scope );
    virtual void join( bool detachSubscriber );
    virtual void tone( const QString& tones );
    virtual void transfer( const QString& number );
    virtual void requestFloor( int secs );
    virtual void releaseFloor();

    virtual void setState( QPhoneCall::State state );
    virtual void setNumber( const QString& number );
    virtual void setActions( QPhoneCallImpl::Actions actions );

    int dataPort() const;
    void setDataPort( int port );

    void emitNotification( QPhoneCall::Notification type, const QString& value );

signals:
    void stateChanged();
    void requestFailed( QPhoneCall::Request request );
    void notification( QPhoneCall::Notification type, const QString& value );
    void floorChanged( bool haveFloor, bool floorAvailable );

private:
    QPhoneCallImplPrivate *d;
};

class QTOPIAPHONE_EXPORT QPhoneCallProvider : public QObject
{
    Q_OBJECT
    friend class QPhoneCallImpl;
public:
    QPhoneCallProvider( const QString& service, QObject *parent );
    ~QPhoneCallProvider();

    QString service() const;
    QList<QPhoneCallImpl *> calls() const;
    QPhoneCallImpl *findCall( const QString& identifier ) const;

    void beginStateTransaction();
    void endStateTransaction();

signals:
    void callStatesChanged();

protected:
    virtual QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType ) = 0;
    void setCallTypes( const QStringList& types );
    virtual void registerCall( QPhoneCallImpl *call );
    virtual void deregisterCall( QPhoneCallImpl *call );

private slots:
    void listAllCalls();
    void dial( const QString& identifier, const QString& service,
               const QString& callType, const QDialOptions& options );
    void hangup( const QString& identifier, QPhoneCall::Scope scope );
    void accept( const QString& identifier );
    void hold( const QString& identifier );
    void activate( const QString& identifier, QPhoneCall::Scope scope );
    void join( const QString& identifier, bool detachSubscriber );
    void tone( const QString& identifier, const QString& tones );
    void transfer( const QString& identifier, const QString& number );
    void stateChanged();
    void requestFailed( QPhoneCall::Request request );
    void notification( QPhoneCall::Notification type, const QString& value );
    void floorChanged( bool haveFloor, bool floorAvailable );

private:
    QPhoneCallImpl *fromIdentifier( const QString& identifier );
    void sendState( QPhoneCallImpl *call );
    void sendStateTransaction();

private:
    QPhoneCallProviderPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPhoneCallImpl::Actions)

#endif // QPHONECALLPROVIDER_H
