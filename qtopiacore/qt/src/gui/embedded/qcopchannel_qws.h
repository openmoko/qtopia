/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOPCHANNEL_QWS_H
#define QCOPCHANNEL_QWS_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_COP

class QWSClient;
class QCopChannelPrivate;

class Q_GUI_EXPORT QCopChannel : public QObject
{
    Q_OBJECT
public:
    explicit QCopChannel(const QString& channel, QObject *parent=0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QCopChannel(const QString& channel, QObject *parent, const char *name);
#endif
    virtual ~QCopChannel();

    QString channel() const;

    static bool isRegistered(const QString&  channel);
    static bool send(const QString& channel, const QString& msg);
    static bool send(const QString& channel, const QString& msg,
                      const QByteArray &data);

    static bool flush();

    static void sendLocally( const QString& ch, const QString& msg,
                               const QByteArray &data);
    static void reregisterAll();

    virtual void receive(const QString& msg, const QByteArray &data);

Q_SIGNALS:
    void received(const QString& msg, const QByteArray &data);

private:
    void init(const QString& channel);

    // server side
    static void registerChannel(const QString& ch, QWSClient *cl);
    static void detach(QWSClient *cl);
    static void answer(QWSClient *cl, const QString& ch,
                        const QString& msg, const QByteArray &data);
    // client side
    QCopChannelPrivate* d;

    friend class QWSServer;
    friend class QWSServerPrivate;
    friend class QApplication;
};

#endif // QT_NO_COP

QT_END_HEADER

#endif // QCOPCHANNEL_QWS_H
