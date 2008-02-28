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

#ifndef __QTOPIAIPCADAPTOR_H__
#define __QTOPIAIPCADAPTOR_H__

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstring.h>
#include <qbytearray.h>
#include <qvariant.h>
#include <qmetatype.h>
#include <qdatastream.h>
#include <qatomic.h>
#include <qstringlist.h>

class QtopiaIpcAdaptorPrivate;
class QtopiaIpcSendEnvelopePrivate;

class QTOPIABASE_EXPORT QtopiaIpcSendEnvelope
{
    friend class QtopiaIpcAdaptor;
private:
    QtopiaIpcSendEnvelope( const QStringList& channels, const QString& message );

public:
    QtopiaIpcSendEnvelope();
    QtopiaIpcSendEnvelope( const QtopiaIpcSendEnvelope& value );
    ~QtopiaIpcSendEnvelope();

    QtopiaIpcSendEnvelope& operator=( const QtopiaIpcSendEnvelope& value );
    template <class T>
    QtopiaIpcSendEnvelope& operator<<( const T &value );

    inline QtopiaIpcSendEnvelope& operator<<( const char *value )
    {
        addArgument(QVariant( QString( value ) ));
        return *this;
    }

private:
    QtopiaIpcSendEnvelopePrivate *d;

    void addArgument( const QVariant& value );
};

class QTOPIABASE_EXPORT QtopiaIpcAdaptor : public QObject
{
    Q_OBJECT
    friend class QtopiaIpcSignalIntercepter;
    friend class QtopiaIpcSendEnvelope;
    friend class QtopiaIpcAdaptorChannel;
public:
    explicit QtopiaIpcAdaptor( const QString& channel, QObject *parent = 0 );
    ~QtopiaIpcAdaptor();

    enum ChannelSelector
    {
        AutoDetect,
        SenderIsChannel,
        ReceiverIsChannel
    };

    static bool connect( QObject *sender, const QByteArray& signal,
                         QObject *receiver, const QByteArray& member,
                         QtopiaIpcAdaptor::ChannelSelector selector = AutoDetect );

    QtopiaIpcSendEnvelope send( const QByteArray& member );
    void send( const QByteArray& member, const QVariant &arg1 );
    void send( const QByteArray& member, const QVariant &arg1, const QVariant &arg2 );
    void send( const QByteArray& member, const QVariant &arg1, const QVariant &arg2,
               const QVariant &arg3 );
    void send( const QByteArray& member, const QList<QVariant>& args );

    bool isConnected( const QByteArray& signal );

protected:
    enum PublishType
    {
        Signals,
        Slots,
        SignalsAndSlots
    };
    bool publish( const QByteArray& member );
    void publishAll( QtopiaIpcAdaptor::PublishType type );
    virtual QString memberToMessage( const QByteArray& member );
    virtual QStringList sendChannels( const QString& channel );
    virtual QString receiveChannel( const QString& channel );

private slots:
    void received( const QString& msg, const QByteArray& data );

private:
    QtopiaIpcAdaptorPrivate *d;

    bool connectLocalToRemote( QObject *sender, const QByteArray& signal,
                               const QByteArray& member );
    bool connectRemoteToLocal( const QByteArray& signal, QObject *receiver,
                               const QByteArray& member );
    void sendMessage( const QString& msg, const QList<QVariant>& args );
    static void send( const QStringList& channels,
                      const QString& msg, const QList<QVariant>& args );
};

template<class T>
QtopiaIpcSendEnvelope& QtopiaIpcSendEnvelope::operator<<( const T &value )
{
    addArgument(qVariantFromValue(value));
    return *this;
}

// Useful alias to make it clearer when connecting to messages on a channel.
#define MESSAGE(x)      "3"#x
#define QMESSAGE_CODE   3

#endif /* __QTOPIAIPCADAPTOR_H__ */
