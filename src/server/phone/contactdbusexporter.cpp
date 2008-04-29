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

#include "contactdbusexporter.h"

ContactDBusExporter::ContactDBusExporter(QObject* parent)
    : QObject(parent)
    , m_model(0)
{}

void ContactDBusExporter::open()
{
}

void ContactDBusExporter::close()
{
}

QList<QVariant> ContactDBusExporter::listContacts() const
{
    return QList<QVariant>();
}

QMap<QString, QVariant> ContactDBusExporter::contact(const QString&)
{
    return QMap<QString, QVariant>();
}
