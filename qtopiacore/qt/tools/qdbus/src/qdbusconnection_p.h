/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSCONNECTION_P_H
#define QDBUSCONNECTION_P_H

#include <qdbuserror.h>
#include <qdbusconnection.h>

#include <QtCore/qatomic.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>

#include <dbus/dbus.h>

#include <qdbusmessage.h>

class QDBusMessage;
class QSocketNotifier;
class QTimerEvent;
class QDBusObjectPrivate;
class CallDeliveryEvent;
class QMetaMethod;
class QDBusInterfacePrivate;
struct QDBusMetaObject;
class QDBusAbstractInterface;
class QDBusConnectionInterface;

// QDBusConnectionPrivate holds the DBusConnection and
// can have many QDBusConnection objects referring to it

class QDBusConnectionPrivate: public QObject
{
    Q_OBJECT
public:
    // structs and enums
    enum ConnectionMode { InvalidMode, ServerMode, ClientMode }; // LocalMode

    struct Watcher
    {
        Watcher(): watch(0), read(0), write(0) {}
        DBusWatch *watch;
        QSocketNotifier *read;
        QSocketNotifier *write;
    };

    struct SignalHook
    {
        inline SignalHook() : obj(0), midx(-1) { }
        QString owner, service, path, signature;
        QObject* obj;
        int midx;
        QList<int> params;
    };

    struct ObjectTreeNode
    {
        struct Data
        {
            QString name;
            ObjectTreeNode *node;

            inline bool operator<(const QString &other) const
            { return name < other; }
        };
        typedef QVector<Data> DataList;

        inline ObjectTreeNode() : obj(0), flags(0) { }
        inline ~ObjectTreeNode() { clear(); }
        inline void clear()
        {
            DataList::ConstIterator it = children.constBegin();
            DataList::ConstIterator end = children.constEnd();
            for ( ; it != end; ++it) {
                it->node->clear();
                delete it->node;
            }
            children.clear();
        }

        QObject* obj;
        int flags;
        DataList children;
    };

public:
    // typedefs
    typedef QMultiHash<int, Watcher> WatcherHash;
    typedef QHash<int, DBusTimeout *> TimeoutHash;
    typedef QMultiHash<QString, SignalHook> SignalHookHash;
    typedef QHash<QString, QDBusMetaObject* > MetaObjectHash;

public:
    // public methods
    QDBusConnectionPrivate(QObject *parent = 0);
    ~QDBusConnectionPrivate();

    void bindToApplication();

    void setConnection(DBusConnection *connection);
    void setServer(DBusServer *server);
    void closeConnection();
    void timerEvent(QTimerEvent *e);

    QString getNameOwner(const QString &service);

    int send(const QDBusMessage &message) const;
    QDBusMessage sendWithReply(const QDBusMessage &message, int mode, int timeout = -1);
    int sendWithReplyAsync(const QDBusMessage &message, QObject *receiver,
                           const char *method, int timeout = -1);
    void connectSignal(const QString &key, const SignalHook &hook);
    void disconnectSignal(const QString &key, const SignalHook &hook);
    void registerObject(const ObjectTreeNode *node);
    void connectRelay(const QString &service, const QString &path, const QString &interface,
                      QDBusAbstractInterface *receiver, const char *signal);
    void disconnectRelay(const QString &service, const QString &path, const QString &interface,
                         QDBusAbstractInterface *receiver, const char *signal);

    bool handleSignal(const QString &key, const QDBusMessage &msg);
    bool handleSignal(const QDBusMessage &msg);
    bool handleObjectCall(const QDBusMessage &message);
    bool handleError();

    bool activateSignal(const SignalHook& hook, const QDBusMessage &msg);
    bool activateCall(QObject* object, int flags, const QDBusMessage &msg);
    bool activateObject(const ObjectTreeNode *node, const QDBusMessage &msg);
    bool activateInternalFilters(const ObjectTreeNode *node, const QDBusMessage &msg);

    void sendCallDeliveryEvent(CallDeliveryEvent *data);
    void postCallDeliveryEvent(CallDeliveryEvent *data);
    CallDeliveryEvent *postedCallDeliveryEvent();
    void deliverCall(const CallDeliveryEvent &data) const;

    QDBusMetaObject *findMetaObject(const QString &service, const QString &path,
                                    const QString &interface);

    void registerService(const QString &serviceName);
    void unregisterService(const QString &serviceName);
    bool isServiceRegisteredByThread(const QString &serviceName) const;
    QString baseService() const;

protected:
    virtual void customEvent(QEvent *event);

public slots:
    // public slots
    void doDispatch();
    void socketRead(int);
    void socketWrite(int);
    void objectDestroyed(QObject *o);
    void relaySignal(QObject *obj, const QMetaObject *, int signalId, const QVariantList &args);
    void _q_serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

signals:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

public:
    // public member variables
    QString name;               // this connection's name

    DBusError error;
    QDBusError lastError;

    QAtomic ref;
    QReadWriteLock lock;
    ConnectionMode mode;
    DBusConnection *connection;
    DBusServer *server;
    QDBusConnectionInterface *busService;

    WatcherHash watchers;
    TimeoutHash timeouts;
    SignalHookHash signalHooks;
    QList<DBusTimeout *> pendingTimeouts;

    ObjectTreeNode rootNode;
    MetaObjectHash cachedMetaObjects;

    QMutex callDeliveryMutex;
    CallDeliveryEvent *callDeliveryState; // protected by the callDeliveryMutex mutex

public:
    // static methods
    static int findSlot(QObject *obj, const QByteArray &normalizedName, QList<int>& params);
    static bool prepareHook(QDBusConnectionPrivate::SignalHook &hook, QString &key,
                            const QString &service, const QString &owner,
                            const QString &path, const QString &interface, const QString &name,
                            QObject *receiver, const char *signal, int minMIdx,
                            bool buildSignature);
    static DBusHandlerResult messageFilter(DBusConnection *, DBusMessage *, void *);
    static void messageResultReceived(DBusPendingCall *, void *);

    static QDBusConnectionPrivate *d(const QDBusConnection& q) { return q.d; }

    static void setSender(const QDBusConnectionPrivate *s);
};

class QDBusReplyWaiter: public QEventLoop
{
    Q_OBJECT
public:
    QDBusMessage replyMsg;

public slots:
    void reply(const QDBusMessage &msg);
};

// in qdbusmisc.cpp
extern int qDBusParametersForMethod(const QMetaMethod &mm, QList<int>& metaTypes);
extern int qDBusNameToTypeId(const char *name);
extern bool qDBusCheckAsyncTag(const char *tag);

// in qdbusinternalfilters.cpp
extern QString qDBusIntrospectObject(const QDBusConnectionPrivate::ObjectTreeNode *node);
extern QDBusMessage qDBusPropertyGet(const QDBusConnectionPrivate::ObjectTreeNode *node,
                                     const QDBusMessage &msg);
extern QDBusMessage qDBusPropertySet(const QDBusConnectionPrivate::ObjectTreeNode *node,
                                     const QDBusMessage &msg);

#endif
