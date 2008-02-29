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

#if defined(QTOPIA_DBUS_IPC)
#include <qtopia/private/dbusapplicationchannel_p.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbuserror.h>
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusconnectioninterface.h>
#include <qtdbus/qdbusabstractadaptor.h>
#include <qtopia/private/dbusipccommon_p.h>
#include "dbusrouter.h"

#include <QDebug>

class DBusLauncherAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.qtopia.DBusLauncher")
public:

    DBusLauncherAdaptor(QObject *parent);
};

DBusLauncherAdaptor::DBusLauncherAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent)
{

}

class DBusLauncherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.qtopia.DBusLauncher")

public:
    DBusLauncherService(QObject *parent = 0);
    ~DBusLauncherService();

public slots:
    Q_SCRIPTABLE void launch(const QString &app);

private slots:
    void terminated(const QString &app, ApplicationTypeLauncher::TerminationReason, bool filtered);
    void applicationStateChanged(const QString &app, ApplicationTypeLauncher::ApplicationState);

private:
    void sendLaunched(const QString &app);
    QSet<QString> m_pending;
};

DBusLauncherService::DBusLauncherService(QObject *parent) : QObject(parent)
{
}

DBusLauncherService::~DBusLauncherService()
{

}

void DBusLauncherService::sendLaunched(const QString &app)
{
    m_pending.remove(app);

    QDBusConnection dbc = QDBus::sessionBus();

    QDBusMessage message = QDBusMessage::signal("/DBusLauncher",
            "com.trolltech.qtopia.DBusLauncher", "launched", dbc);

    message << app;

    bool ret = dbc.send(message);
    if (!ret)
        qWarning("Unable to send message: %s", dbc.lastError().message().toAscii().constData());
}

void DBusLauncherService::applicationStateChanged(const QString &app,
                                 ApplicationTypeLauncher::ApplicationState state)
{
    if (!m_pending.contains(app)) {
        return;
    }

    if (state != ApplicationTypeLauncher::Running) {
        return;
    }

    sendLaunched(app);
}

void DBusLauncherService::terminated(const QString &app,
                    ApplicationTypeLauncher::TerminationReason reason, bool)
{
    if (!m_pending.contains(app)) {
        return;
    }

    sendLaunched(app);
}

void DBusLauncherService::launch(const QString &app)
{
    // Launch route
    ApplicationLauncher *l = qtopiaTask<ApplicationLauncher>();
    if (l) {
        m_pending.insert(app);
        connect(l, SIGNAL(applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)),
                this, SLOT(applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)));
        connect(l, SIGNAL(applicationTerminated(const QString &, ApplicationTypeLauncher::TerminationReason, bool)),
                this, SLOT(terminated(const QString &,                           ApplicationTypeLauncher::TerminationReason, bool)));

        l->launch(app);
    }
}

/*!
  \class DBusRouter
  \ingroup QtopiaServer::Task
  \brief The DBusRouter launches DBus applications and routes Application Messages

  For ease of replacability with QCopRouter, the DBusRouter task installs itself
  as IpcRouter.
 */

QTOPIA_TASK(IpcRouter, DBusRouter);
QTOPIA_TASK_PROVIDES(IpcRouter, ApplicationIpcRouter);

DBusRouter::DBusRouter()
{
    DBusLauncherService *service = new DBusLauncherService(this);
    DBusLauncherAdaptor *adaptor = new DBusLauncherAdaptor(service);

    QDBusConnection dbc = QDBus::sessionBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect do D-BUS:" << dbc.lastError();
        return;
    }

    QDBusConnectionInterface *iface = dbc.interface();

    if (iface->registerService("com.trolltech.qtopia.DBusLauncher") ==
        QDBusConnectionInterface::ServiceNotRegistered) {
        qWarning() << "WARNING: Could not register DBusLauncher service!!!";
        return;
    }

    bool succ = dbc.registerObject("/DBusLauncher", service, QDBusConnection::ExportSlots);
}

DBusRouter::~DBusRouter()
{
    foreach (DBUSQtopiaApplicationChannel *channel, m_channels) {
        delete channel;
    }
}

// Process all messages to QPE/Application/*.
void DBusRouter::applicationMessage
        ( const QString& msg, const QByteArray& data )
{
    QObject *s = sender();

    if (!s)
        return;

    DBUSQtopiaApplicationChannel *channel = qobject_cast<DBUSQtopiaApplicationChannel *>(s);
    if (!channel)
        return;

    routeMessage(channel->appName(), msg, data);
}

void DBusRouter::routeMessage(const QString &dest,
                              const QString &message,
                              const QByteArray &data)
{
    // Launch route
    ApplicationLauncher *l = qtopiaTask<ApplicationLauncher>();
    if(l)
        l->launch(dest);

    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(dest);
    while(iter != m_routes.end() && iter.key() == dest) {
        (*iter)->routeMessage(dest, message, data);
        ++iter;
    }
}

void DBusRouter::addRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    m_routes.insert(app, dest);

    if (!m_channels.contains(app)) {
        DBUSQtopiaApplicationChannel *channel = new DBUSQtopiaApplicationChannel(app, this);
        connect(channel, SIGNAL(received(const QString &, const QByteArray &)),
                this, SLOT(applicationMessage(const QString &, const QByteArray &)));
        m_channels.insert(app, channel);
    }
}

void DBusRouter::remRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(app);
    while(iter != m_routes.end() && iter.key() == app) {
        if(iter.value() == dest) {
            m_routes.erase(iter);
            return;
        }
        ++iter;
    }

    if (!m_routes.contains(app)) {
        QMap<QString, DBUSQtopiaApplicationChannel *>::Iterator iter = m_channels.find(app);
        delete iter.value();
        m_channels.erase(iter);
    }
}

#include "dbusrouter.moc"

#endif
