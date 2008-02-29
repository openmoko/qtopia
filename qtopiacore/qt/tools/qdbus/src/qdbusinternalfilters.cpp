/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdbusconnection_p.h"

#include <dbus/dbus.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>

#include "qdbusabstractadaptor.h"
#include "qdbusabstractadaptor_p.h"
#include "qdbusconnection.h"
#include "qdbusextratypes.h"
#include "qdbusmessage.h"
#include "qdbusmessage_p.h"
#include "qdbusutil_p.h"

// ### duplicated code, see QDBusUtil::isValidPartOfObjectPath(const QString &part)
static bool isValidPartOfObjectPath(const QString &part)
{
    if (part.isEmpty())
        return false;       // can't be valid if it's empty

    const QChar *c = part.unicode();
    for (int i = 0; i < part.length(); ++i) {
        register ushort u = c[i].unicode();
        if (!((u >= 'a' && u <= 'z') ||
              (u >= 'A' && u <= 'Z') ||
              (u >= '0' && u <= '9') ||
              u == '_'))
            return false;
    }
    return true;
}

// defined in qdbusxmlgenerator.cpp
extern QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo,
                                          const QMetaObject *base, int flags);

static const char introspectableInterfaceXml[] =
    "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
    "    <method name=\"Introspect\">\n"
    "      <arg name=\"xml_data\" type=\"s\" direction=\"out\"/>\n"
    "    </method>\n"
    "  </interface>\n";

static const char propertiesInterfaceXml[] =
    "  <interface name=\"org.freedesktop.DBus.Properties\">\n"
    "    <method name=\"Get\">\n"
    "      <arg name=\"interface_name\" type=\"s\" direction=\"in\"/>\n"
    "      <arg name=\"property_name\" type=\"s\" direction=\"in\"/>\n"
    "      <arg name=\"value\" type=\"v\" direction=\"out\"/>\n"
    "    </method>\n"
    "    <method name=\"Set\">\n"
    "      <arg name=\"interface_name\" type=\"s\" direction=\"in\"/>\n"
    "      <arg name=\"property_name\" type=\"s\" direction=\"in\"/>\n"
    "      <arg name=\"value\" type=\"v\" direction=\"in\"/>\n"
    "    </method>\n"
    "  </interface>\n";

static QString generateSubObjectXml(QObject *object)
{
    QString retval;
    const QObjectList &objs = object->children();
    QObjectList::ConstIterator it = objs.constBegin();
    QObjectList::ConstIterator end = objs.constEnd();
    for ( ; it != end; ++it) {
        QString name = (*it)->objectName();
        if (!name.isEmpty() && isValidPartOfObjectPath(name))
            retval += QString(QLatin1String("  <node name=\"%1\"/>\n"))
                      .arg(name);
    }
    return retval;
}

// declared as extern in qdbusconnection_p.h

QString qDBusIntrospectObject(const QDBusConnectionPrivate::ObjectTreeNode *node)
{
    // object may be null

    QString xml_data(QLatin1String(DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE));
    xml_data += QLatin1String("<node>\n");

    if (node->obj) {
        if (node->flags & (QDBusConnection::ExportScriptableContents
                           | QDBusConnection::ExportNonScriptableContents)) {
            // create XML for the object itself
            const QMetaObject *mo = node->obj->metaObject();
            for ( ; mo != &QObject::staticMetaObject; mo = mo->superClass())
                xml_data += qDBusGenerateMetaObjectXml(QString(), mo, mo->superClass(),
                                                  node->flags);
        }

        // does this object have adaptors?
        QDBusAdaptorConnector *connector;
        if (node->flags & QDBusConnection::ExportAdaptors &&
            (connector = qDBusFindAdaptorConnector(node->obj))) {

            // trasverse every adaptor in this object
            QDBusAdaptorConnector::AdaptorMap::ConstIterator it = connector->adaptors.constBegin();
            QDBusAdaptorConnector::AdaptorMap::ConstIterator end = connector->adaptors.constEnd();
            for ( ; it != end; ++it) {
                // add the interface:
                QString ifaceXml = QDBusAbstractAdaptorPrivate::retrieveIntrospectionXml(it->adaptor);
                if (ifaceXml.isEmpty()) {
                    // add the interface's contents:
                    ifaceXml += qDBusGenerateMetaObjectXml(QString::fromLatin1(it->interface),
                                                           it->adaptor->metaObject(),
                                                           &QDBusAbstractAdaptor::staticMetaObject,
                                                           QDBusConnection::ExportScriptableContents
                                                           | QDBusConnection::ExportNonScriptableContents);

                    QDBusAbstractAdaptorPrivate::saveIntrospectionXml(it->adaptor, ifaceXml);
                }

                xml_data += ifaceXml;
            }
        }

        xml_data += QLatin1String( propertiesInterfaceXml );
    }

    xml_data += QLatin1String( introspectableInterfaceXml );

    if (node->flags & QDBusConnection::ExportChildObjects) {
        xml_data += generateSubObjectXml(node->obj);
    } else {
        // generate from the object tree
        QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator it =
            node->children.constBegin();
        QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator end =
            node->children.constEnd();
        for ( ; it != end; ++it) {
            const QDBusConnectionPrivate::ObjectTreeNode::Data &entry = *it;
            if (entry.node && (entry.node->obj || !entry.node->children.isEmpty()))
                xml_data += QString(QLatin1String("  <node name=\"%1\"/>\n"))
                            .arg(entry.name);
        }
    }

    xml_data += QLatin1String("</node>\n");
    return xml_data;
}

// implement the D-Bus interface org.freedesktop.DBus.Properties

static QDBusMessage qDBusPropertyError(const QDBusMessage &msg, const QString &interface_name)
{
    return msg.createErrorReply(QLatin1String(DBUS_ERROR_INVALID_ARGS),
                                QString::fromLatin1("Interface %1 was not found in object %2")
                                .arg(interface_name)
                                .arg(msg.path()));
}

QDBusMessage qDBusPropertyGet(const QDBusConnectionPrivate::ObjectTreeNode *node,
                              const QDBusMessage &msg)
{
    Q_ASSERT(msg.arguments().count() == 2);
    QString interface_name = msg.arguments().at(0).toString();
    QByteArray property_name = msg.arguments().at(1).toString().toUtf8();

    QDBusAdaptorConnector *connector;
    QVariant value;
    if (node->flags & QDBusConnection::ExportAdaptors &&
        (connector = qDBusFindAdaptorConnector(node->obj))) {

        // find the class that implements interface_name
        QDBusAdaptorConnector::AdaptorMap::ConstIterator it;
        it = qLowerBound(connector->adaptors.constBegin(), connector->adaptors.constEnd(),
                         interface_name);
        if (it != connector->adaptors.constEnd() && interface_name == QLatin1String(it->interface))
            value = it->adaptor->property(property_name);
    }

    if (!value.isValid() && node->flags & (QDBusConnection::ExportScriptableProperties |
                                           QDBusConnection::ExportNonScriptableProperties)) {
        // try the object itself
        int pidx = node->obj->metaObject()->indexOfProperty(property_name);
        if (pidx != -1) {
            QMetaProperty mp = node->obj->metaObject()->property(pidx);
            if ((mp.isScriptable() && (node->flags & QDBusConnection::ExportScriptableProperties)) ||
                (!mp.isScriptable() && (node->flags & QDBusConnection::ExportNonScriptableProperties)))
                value = mp.read(node->obj);
        }
    }

    if (!value.isValid()) {
        // the property was not found
        return qDBusPropertyError(msg, interface_name);
    }

    return msg.createReply(qVariantFromValue(QDBusVariant(value)));
}

QDBusMessage qDBusPropertySet(const QDBusConnectionPrivate::ObjectTreeNode *node,
                              const QDBusMessage &msg)
{
    Q_ASSERT(msg.arguments().count() == 3);
    QString interface_name = msg.arguments().at(0).toString();
    QByteArray property_name = msg.arguments().at(1).toString().toUtf8();
    QVariant value = qvariant_cast<QDBusVariant>(msg.arguments().at(2)).variant();

    QDBusAdaptorConnector *connector;
    if (node->flags & QDBusConnection::ExportAdaptors &&
        (connector = qDBusFindAdaptorConnector(node->obj))) {

        // find the class that implements interface_name
        QDBusAdaptorConnector::AdaptorMap::ConstIterator it;
        it = qLowerBound(connector->adaptors.constBegin(), connector->adaptors.constEnd(),
                         interface_name);
        if (it != connector->adaptors.end() && interface_name == QLatin1String(it->interface))
            if (it->adaptor->setProperty(property_name, value))
                return msg.createReply();
    }

    if (node->flags & (QDBusConnection::ExportScriptableProperties |
                       QDBusConnection::ExportNonScriptableProperties)) {
        // try the object itself
        int pidx = node->obj->metaObject()->indexOfProperty(property_name);
        if (pidx != -1) {
            QMetaProperty mp = node->obj->metaObject()->property(pidx);
            if ((mp.isScriptable() && (node->flags & QDBusConnection::ExportScriptableProperties)) ||
                (!mp.isScriptable() && (node->flags & QDBusConnection::ExportNonScriptableProperties)))
                if (mp.write(node->obj, value))
                    return msg.createReply();
        }
    }

    // the property was not found or not written to
    return qDBusPropertyError(msg, interface_name);
}
