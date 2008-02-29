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
#ifndef QDOCUMENTSERVERCHANNEL_P_H
#define QDOCUMENTSERVERCHANNEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qunixsocket_p.h>
#include <qtopiabase/qtopiaipcmarshal.h>
#include <QVariantList>

#ifndef QT_NO_SXE
#include <QTransportAuth>
#endif


class QDocumentServerChannel;
class QDocumentServerMessagePrivate;

class QDocumentServerMessage
{
public:
    enum MessageType
    {
        InvalidMessage,
        MethodCallMessage,
        SignalMessage,
        SlotMessage,
        ReplyMessage,
        ErrorMessage
    };

    QDocumentServerMessage();
    QDocumentServerMessage( const QDocumentServerMessage &other );
    QDocumentServerMessage &operator =( const QDocumentServerMessage &other );
    ~QDocumentServerMessage();

    static QDocumentServerMessage createMethodCall( const QByteArray &signature );
    static QDocumentServerMessage createSlot( const QByteArray &signature );
    static QDocumentServerMessage createSignal( const QByteArray &signature );

    QDocumentServerMessage createReply( const QVariantList &arguments, const QList< QUnixSocketRights > &rights = QList< QUnixSocketRights >()  ) const;
    inline QDocumentServerMessage createReply( const QVariant &argument ) const{ return createReply( QVariantList() << argument ); }
    inline QDocumentServerMessage createReply( const QUnixSocketRights &rights ) const{ return createReply( QVariantList(), QList< QUnixSocketRights >() << rights ); }

    QDocumentServerMessage createError( const QString &message ) const;

    MessageType type() const;

    QByteArray signature() const;

    int transactionId() const;

    QVariantList arguments() const;
    void setArguments( const QVariantList &arguments );

    QList< QUnixSocketRights > rights() const;
    void setRights( const QList< QUnixSocketRights > &rights );

    QDocumentServerMessage &operator <<( const QVariant &argument );
    QDocumentServerMessage &operator <<( const QUnixSocketRights &rights );

private:
    QSharedDataPointer< QDocumentServerMessagePrivate > d;

    friend class QDocumentServerChannel;
};

Q_DECLARE_METATYPE(QDocumentServerMessage);
Q_DECLARE_USER_METATYPE_ENUM(QDocumentServerMessage::MessageType);

class QDocumentServerChannel : public QObject
{
    Q_OBJECT
public:
    QDocumentServerChannel( const QByteArray &interface, QObject *parent = 0 );
    virtual ~QDocumentServerChannel();

    bool messagesAvailable() const;

    const QDocumentServerMessage &peek() const;
    QDocumentServerMessage read();

    bool write( const QDocumentServerMessage &message );
    void flush();

    bool connect();
    bool setSocketDescriptor( int socketDescriptor );

    bool isConnected() const;

    void disconnect();

    bool waitForReadyRead( int msecs = 3000 );

signals:
    void readyRead();
    void disconnected();

private slots:
    void socketReadyRead();
    void stateChanged();

private:
    QUnixSocketMessage marshalMessage( const QDocumentServerMessage &message );
    QDocumentServerMessage unmarshalMessage( const QByteArray &messageData, const QList< QUnixSocketRights > &rights );

    QByteArray m_interface;
    QUnixSocket m_socket;

    QByteArray m_messageBuffer;
    QList< QUnixSocketRights > m_rightsBuffer;
    QList< QDocumentServerMessage > m_receivedMessages;

#ifndef QT_NO_SXE
    const QString m_prefix;
    QTransportAuth::Data *m_authData;
#endif


    enum Role
    {
        Client,
        Server
    };

    Role m_role;

    bool m_messageReceived;
};

class QDocumentServerHost : public QObject
{
    Q_OBJECT
public:
    QDocumentServerHost( const QByteArray &interface, QObject *parent = 0 );

    bool setSocketDescriptor( int socketDescriptor );

    void emitSignalWithArgumentList( const char *signal, const QVariantList &arguments, const QList< QUnixSocketRights > &rights = QList< QUnixSocketRights >() );

signals:
    void disconnected();

protected:
    virtual QDocumentServerMessage invokeMethod( const QDocumentServerMessage &message ) = 0;
    virtual void invokeSlot( const QDocumentServerMessage &message );

private slots:
    virtual void handlePendingMessages();

private:
    QDocumentServerChannel m_channel;
};

class QDocumentServerClient : public QObject
{
    Q_OBJECT
public:
    QDocumentServerClient( const QByteArray &interface, QObject *parent = 0 );

    bool connect();

    QDocumentServerMessage callWithArgumentList( const char *method, const QVariantList &arguments, const QList< QUnixSocketRights > &rights = QList< QUnixSocketRights >() );

    void callSlotWithArgumentList( const char *method, const QVariantList &arguments, const QList< QUnixSocketRights > &rights = QList< QUnixSocketRights >() );

signals:
    void disconnected();

protected slots:
    virtual void invokeSignal( const QDocumentServerMessage &message );

private slots:
    virtual void handlePendingMessages();

private:
    QDocumentServerMessage blockingCall( const QDocumentServerMessage &message );

    QDocumentServerChannel m_channel;
};

#endif

