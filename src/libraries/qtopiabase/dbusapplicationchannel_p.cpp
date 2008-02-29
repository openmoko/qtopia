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

#if defined(QTOPIA_DBUS_IPC)

#include "dbusapplicationchannel_p.h"
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbuserror.h>
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusconnectioninterface.h>
#include <qtdbus/qdbusabstractadaptor.h>
#include "dbusipccommon_p.h"

#include <qtopialog.h>
#include <QFile>

class DBUSQtopiaApplicationChannelAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.qtopia")

public:
    DBUSQtopiaApplicationChannelAdaptor(QObject *parent);
};

DBUSQtopiaApplicationChannelAdaptor::DBUSQtopiaApplicationChannelAdaptor(QObject *parent) :
         QDBusAbstractAdaptor(parent)
{

}

/*!
    \class DBUSQtopiaApplicationChannel
    \brief The DBUSQtopiaApplicationChannel class implements Qtopia application channels over DBUS.

    \internal

    This class is an implementation detail and should not be used.

    It is exported for use by libqtopia and the qpe server.
*/

DBUSQtopiaApplicationChannel::DBUSQtopiaApplicationChannel(const QString &appName,
                                                           QObject *parent) :
        QObject(parent),
        m_serviceName("com.trolltech.qtopia.QPE.Application."),
        m_appName(appName)
{
    m_serviceName.append(appName);
    DBUSQtopiaApplicationChannelAdaptor *adaptor =
            new DBUSQtopiaApplicationChannelAdaptor(this);

    QDBusConnection dbc = QDBus::sessionBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }

    QDBusConnectionInterface *iface = dbc.interface();

    if (iface->registerService(m_serviceName) ==
        QDBusConnectionInterface::ServiceNotRegistered) {
        qWarning() << "WARNING: Could not register QtopiaApplicationChannel service" << m_serviceName;
        return;
    }

    QString dbusPath("/com/trolltech/qtopia/QPE/Application/");
    dbusPath.append(appName);

    qLog(DBUS) << "Registering applicationChannel service:" << appName << "and Path" << dbusPath;

    bool succ = dbc.registerObject(dbusPath, this, QDBusConnection::ExportSlots);
    if (!succ) {
        qLog(DBUS) << dbc.lastError();
    }
}

/*!
    Destructor.
*/
DBUSQtopiaApplicationChannel::~DBUSQtopiaApplicationChannel()
{
    QDBusConnection dbc = QDBus::sessionBus();
    if (!dbc.isConnected())
        return;

    QDBusConnectionInterface *iface = dbc.interface();

    qLog(DBUS) << "Unregistering from:" << m_serviceName;
    iface->unregisterService(m_serviceName);
}

/*!
    Implement the appMessage.
*/
void DBUSQtopiaApplicationChannel::appMessage(const QString &msg, const QByteArray &data)
{
    QFile f("/home/dkenzior/foobar");
    f.open(QIODevice::WriteOnly);
    QDataStream stream(&f);
    stream << "DBUSQtopiaApplicationChannel Received message:" << msg;
    f.close();
    qLog(DBUS) << "DBUSQtopiaApplicationChannel Received message:" << msg;
    emit received(msg, data);
}

#include "dbusapplicationchannel_p.moc"

#endif
