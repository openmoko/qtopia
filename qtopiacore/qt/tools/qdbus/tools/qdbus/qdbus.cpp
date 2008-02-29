/****************************************************************************
**
** Copyright (C) 2006-2007 Trolltech ASA. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/qmetaobject.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtDBus>
#include <private/qdbusutil_p.h>

static QDBusConnection connection("");

static void listObjects(const QString &service, const QString &path)
{
    QDBusInterface iface(service, path.isEmpty() ? "/" : path,
                         "org.freedesktop.DBus.Introspectable", connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Cannot introspect object %s at %s:\n%s (%s)\n",
                qPrintable(path.isEmpty() ? "/" : path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    QDBusReply<QString> xml = iface.call("Introspect");

    if (!xml.isValid())
        return;                 // silently

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QLatin1String("node")) {
            QString sub = path + '/' + child.attribute("name");
            printf("%s\n", qPrintable(sub));
            listObjects(service, sub);
        }
        child = child.nextSiblingElement();
    }
}

static void listInterface(const QString &service, const QString &path, const QString &interface)
{
    QDBusInterface iface(service, path, interface, connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Interface '%s' not available in object %s at %s:\n%s (%s)\n",
                qPrintable(interface), qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    const QMetaObject *mo = iface.metaObject();

    // properties
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        QMetaProperty mp = mo->property(i);
        printf("property ");

        if (mp.isReadable() && mp.isWritable())
            printf("readwrite");
        else if (mp.isReadable())
            printf("read");
        else
            printf("write");

        printf(" %s %s.%s\n", mp.typeName(), qPrintable(interface), mp.name());
    }

    // methods (signals and slots)
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);

        QByteArray signature = mm.signature();
        signature.truncate(signature.indexOf('('));
        printf("%s %s%s%s %s.%s(",
               mm.methodType() == QMetaMethod::Signal ? "signal" : "method",
               mm.tag(), *mm.tag() ? " " : "",
               *mm.typeName() ? mm.typeName() : "void",
               qPrintable(interface), signature.constData());

        QList<QByteArray> types = mm.parameterTypes();
        QList<QByteArray> names = mm.parameterNames();
        bool first = true;
        for (int i = 0; i < types.count(); ++i) {
            printf("%s%s",
                   first ? "" : ", ",
                   types.at(i).constData());
            if (!names.at(i).isEmpty())
                printf(" %s", names.at(i).constData());
            first = false;
        }
        printf(")\n");
    }
}

static void listAllInterfaces(const QString &service, const QString &path)
{
    QDBusInterface iface(service, path, "org.freedesktop.DBus.Introspectable", connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Cannot introspect object %s at %s:\n%s (%s)\n",
                qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    QDBusReply<QString> xml = iface.call("Introspect");

    if (!xml.isValid())
        return;                 // silently

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QLatin1String("interface")) {
            QString ifaceName = child.attribute("name");
            if (QDBusUtil::isValidInterfaceName(ifaceName))
                listInterface(service, path, ifaceName);
            else {
                qWarning("Invalid D-BUS interface name '%s' found while parsing introspection",
                         qPrintable(ifaceName));
            }
        }
        child = child.nextSiblingElement();
    }
}

static QStringList readList(QStringList &args)
{
    args.takeFirst();

    QStringList retval;
    while (!args.isEmpty() && args.at(0) != QLatin1String(")"))
        retval += args.takeFirst();

    return retval;
}

static void placeCall(const QString &service, const QString &path, const QString &interface,
               const QString &member, QStringList args)
{
    QDBusInterface iface(service, path, interface, connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Interface '%s' not available in object %s at %s:\n%s (%s)\n",
                qPrintable(interface), qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }

    const QMetaObject *mo = iface.metaObject();
    QByteArray match = member.toLatin1();
    match += '(';

    int midx = -1;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        if (signature.startsWith(match)) {
            midx = i;
            break;
        }
    }

    if (midx == -1) {
        fprintf(stderr, "Cannot find '%s.%s' in object %s at %s\n",
                qPrintable(interface), qPrintable(member), qPrintable(path),
                qPrintable(service));
        exit(1);
    }

    QMetaMethod mm = mo->method(midx);
    QList<QByteArray> types = mm.parameterTypes();
    for (int i = 0; i < types.count(); ++i)
        if (types.at(i).endsWith('&')) {
            // reference (and not a reference to const): output argument
            // we're done with the inputs
            while (types.count() > i)
                types.removeLast();
            break;
        }

    QVariantList params;
    for (int i = 0; !args.isEmpty() && i < types.count(); ++i) {
        int id = QVariant::nameToType(types.at(i));
        if ((id == QVariant::UserType || id == QVariant::Map) && types.at(i) != "QDBusVariant") {
            fprintf(stderr, "Sorry, can't pass arg of type %s yet\n",
                    types.at(i).constData());
            exit(1);
        }
        if (id == QVariant::UserType)
            id = QMetaType::type(types.at(i));

        Q_ASSERT(id);

        QVariant p;
        QString argument;
        if ((id == QVariant::List || id == QVariant::StringList)
                && args.at(0) == QLatin1String("("))
            p = readList(args);
        else
            p = argument = args.takeFirst();

        if (id < int(QVariant::UserType)) {
            // avoid calling it for QVariant
            p.convert(QVariant::Type(id));
            if (p.type() == QVariant::Invalid) {
                fprintf(stderr, "Could not convert '%s' to type '%s'.\n",
                        qPrintable(argument), types.at(i).constData());
                exit(1);
            }
        } else if (types.at(i) == "QDBusVariant") {
            QDBusVariant tmp(p);
            p = qVariantFromValue(tmp);
        }
        params += p;
    }
    if (params.count() != types.count() || !args.isEmpty()) {
        fprintf(stderr, "Invalid number of parameters\n");
        exit(1);
    }

    QDBusMessage reply = iface.callWithArgumentList(QDBus::Block, member, params);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        QDBusError err = reply;
        printf("Error: %s\n%s\n", qPrintable(err.name()), qPrintable(err.message()));
        exit(2);
    } else if (reply.type() != QDBusMessage::ReplyMessage) {
        fprintf(stderr, "Invalid reply type %d\n", int(reply.type()));
        exit(1);
    }

    foreach (QVariant v, reply.arguments()) {
        if (v.userType() == QVariant::StringList) {
            foreach (QString s, v.toStringList())
                printf("%s\n", qPrintable(s));
        } else {
            if (v.userType() == qMetaTypeId<QDBusVariant>())
                v = qvariant_cast<QDBusVariant>(v).variant();
            printf("%s\n", qPrintable(v.toString()));
        }
    }

    exit(0);
}

static bool splitInterfaceAndName(const QString &interfaceAndName, const char *type,
                                  QString &interface, QString &member)
{
    interface = interfaceAndName;
    int pos = interface.lastIndexOf(QLatin1Char('.'));
    if (pos != -1) {
        member = interface.mid(pos + 1);
        interface.truncate(pos);
    }

    if (!QDBusUtil::isValidInterfaceName(interface)) {
        fprintf(stderr, "Interface '%s' is not a valid interface name.\n", qPrintable(interface));
        return false;
    } else if (!QDBusUtil::isValidMemberName(member)) {
        fprintf(stderr, "%s name '%s' is not a valid member name.\n", type, qPrintable(member));
        return false;
    }
    return true;
}

static void printAllServices(QDBusConnectionInterface *bus)
{
    const QStringList services = bus->registeredServiceNames();
    QMap<QString, QStringList> servicesWithAliases;

    foreach (QString serviceName, services) {
        QDBusReply<QString> reply = bus->serviceOwner(serviceName);
        QString owner = reply;
        if (owner.isEmpty())
            owner = serviceName;
        servicesWithAliases[owner].append(serviceName);
    }

    for (QMap<QString,QStringList>::const_iterator it = servicesWithAliases.constBegin();
         it != servicesWithAliases.constEnd(); ++it) {
        QStringList names = it.value();
        names.sort();
        printf("%s\n", qPrintable(names.join("\n ")));
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    args.takeFirst();

    if (args.value(0) == QLatin1String("--system")) {
        connection = QDBusConnection::systemBus();
        args.takeFirst();
    } else {
        connection = QDBusConnection::sessionBus();
    }

    if (!connection.isConnected()) {
        fprintf(stderr, "Could not connect to D-Bus server: %s: %s\n",
                qPrintable(connection.lastError().name()),
                qPrintable(connection.lastError().message()));
        return 1;
    }

    QDBusConnectionInterface *bus = connection.interface();
    if (args.isEmpty()) {
        printAllServices(bus);
        exit(0);
    }

    QString service = args.takeFirst();
    if (!QDBusUtil::isValidBusName(service)) {
        fprintf(stderr, "Service '%s' is not a valid name.\n", qPrintable(service));
        exit(1);
    }
    if (!bus->isServiceRegistered(service)) {
        fprintf(stderr, "Service '%s' does not exist.\n", qPrintable(service));
        exit(1);
    }

    if (args.isEmpty()) {
        printf("/\n");
        listObjects(service, QString());
        exit(0);
    }

    QString path = args.takeFirst();
    if (!QDBusUtil::isValidObjectPath(path)) {
        fprintf(stderr, "Path '%s' is not a valid path name.\n", qPrintable(path));
        exit(1);
    }
    if (args.isEmpty()) {
        listAllInterfaces(service, path);
        exit(0);
    }

    QString interface = args.takeFirst();
    QString member;
    int pos = interface.lastIndexOf(QLatin1Char('.'));
    if (pos == -1) {
        member = interface;
        interface.clear();
    } else {
        member = interface.mid(pos + 1);
        interface.truncate(pos);
    }
    if (!interface.isEmpty() && !QDBusUtil::isValidInterfaceName(interface)) {
        fprintf(stderr, "Interface '%s' is not a valid interface name.\n", qPrintable(interface));
        exit(1);
    }
    if (!QDBusUtil::isValidMemberName(member)) {
        fprintf(stderr, "Method name '%s' is not a valid member name.\n", qPrintable(member));
        exit(1);
    }

    placeCall(service, path, interface, member, args);
}

