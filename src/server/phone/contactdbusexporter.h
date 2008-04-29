/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef CONTACTDBUS_EXPORTER_H
#define CONTACTDBUS_EXPORTER_H

#ifdef QT_ILLUME_LAUNCHER

#include <QDBusConnection>

class QContactModel;

class ContactDBusExporter : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openmoko.qtopia.Contact")

public:
    ContactDBusExporter(QObject* parent);

Q_SIGNALS:
    Q_SCRIPTABLE void modelChanged();

public Q_SLOTS:
    Q_SCRIPTABLE void open();
    Q_SCRIPTABLE void close();

    Q_SCRIPTABLE QList<QVariant> listContacts() const;
    Q_SCRIPTABLE QMap<QString, QVariant> contact(const QString&);

private:
    QContactModel* m_model;
};

#endif

#endif
