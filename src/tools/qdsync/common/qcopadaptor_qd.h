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
#ifndef QCOPADAPTOR_QD_H
#define QCOPADAPTOR_QD_H

#include <qdglobal.h>

#include <qobject.h>
#include <qstring.h>
#include <qbytearray.h>
#include <qvariant.h>
#include <qmetatype.h>
#include <qdatastream.h>
#include <qatomic.h>
#include <qstringlist.h>

namespace qdsync {

class QCopAdaptorPrivate;
class QCopSignalIntercepter;

class QD_EXPORT QCopAdaptor : public QObject
{
    friend class QCopSignalIntercepter;
    Q_OBJECT
public:
    explicit QCopAdaptor( const QString &channel, QObject *parent = 0 );
    ~QCopAdaptor();

    enum ChannelSelector
    {
        AutoDetect,
        SenderIsChannel,
        ReceiverIsChannel
    };

    static bool connect( QObject *sender, const QByteArray &signal,
                         QObject *receiver, const QByteArray &member,
                         QCopAdaptor::ChannelSelector selector = AutoDetect );

    //QtopiaIpcSendEnvelope send( const QByteArray &member );
    void send( const QByteArray &member, const QList<QVariant> &args );

    bool isConnected( const QByteArray &signal );

protected:
    enum PublishType
    {
        Signals,
        Slots,
        SignalsAndSlots
    };
    bool publish( const QByteArray &member );
    void publishAll( QCopAdaptor::PublishType type );
    virtual QString memberToMessage( const QByteArray &member );
    virtual QStringList sendChannels( const QString &channel );
    virtual QString receiveChannel( const QString &channel );

private slots:
    void received( const QString &msg, const QByteArray &data );

private:
    bool connectLocalToRemote( QObject *sender, const QByteArray &signal,
                               const QByteArray &member );
    bool connectRemoteToLocal( const QByteArray &signal, QObject *receiver,
                               const QByteArray &member );
    void sendMessage( const QString &msg, const QList<QVariant> &args );
    static void send( const QStringList &channels,
                      const QString &msg, const QList<QVariant> &args );

    QCopAdaptorPrivate *d;
};

#if 0
class QD_EXPORT QtopiaIpcSendEnvelope
{
    friend class QtopiaIpcAdaptor;
private:
    QtopiaIpcSendEnvelope( const QStringList &channels, const QString &message );

public:
    QtopiaIpcSendEnvelope();
    QtopiaIpcSendEnvelope( const QtopiaIpcSendEnvelope &value );
    ~QtopiaIpcSendEnvelope();

    QtopiaIpcSendEnvelope &operator=( const QtopiaIpcSendEnvelope &value );
    template <class T>
    QtopiaIpcSendEnvelope &operator<<( const T &value );

    inline QtopiaIpcSendEnvelope &operator<<( const char *value )
    {
        addArgument(QVariant( QString( value ) ));
        return *this;
    }

private:
    QtopiaIpcSendEnvelopePrivate *d;

    void addArgument( const QVariant &value );
};

template<class T>
QtopiaIpcSendEnvelope &QtopiaIpcSendEnvelope::operator<<( const T &value )
{
    addArgument(qVariantFromValue(value));
    return *this;
}
#endif

};

using namespace qdsync;

#define MESSAGE(x) SIGNAL(x)

#endif
