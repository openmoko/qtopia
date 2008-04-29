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

#include <qcontactmodel.h>

ContactDBusExporter::ContactDBusExporter(QObject* parent)
    : QObject(parent)
    , m_model(0)
{}

void ContactDBusExporter::open()
{
    delete m_model;
    m_model = new QContactModel(this);
    connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            SIGNAL(modelChanged()));
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            SIGNAL(modelChanged()));
    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            SIGNAL(modelChanged()));
}

void ContactDBusExporter::close()
{
    delete m_model;
    m_model = 0;
}

QList<QVariant> ContactDBusExporter::listContacts() const
{
    if (!m_model)
        return QList<QVariant>();

    QList<QVariant> result;
    for (int i = 0; i < m_model->rowCount(); ++i)
        result << m_model->id(i).toString();

    return result;
}

QMap<QString, QVariant> ContactDBusExporter::contact(const QString& id)
{
    if (!m_model)
        return QMap<QString, QVariant>();

    QMap<QString, QVariant> result;
    QContact contact = m_model->contact(QUniqueId(id));

    result[QLatin1String("uid")] = contact.uid().toString();
    result[QLatin1String("notes")] = contact.notes();
    result[QLatin1String("nameTitle")] = contact.nameTitle();
    result[QLatin1String("firstName")] = contact.firstName();
    result[QLatin1String("middleName")] = contact.middleName();
    result[QLatin1String("lastName")] = contact.lastName();
    result[QLatin1String("suffix")] = contact.suffix();

    result[QLatin1String("homePhone")] = contact.homePhone();
    result[QLatin1String("homeMobile")] = contact.homeMobile();
    result[QLatin1String("homeVoip")] = contact.homeVOIP();

    result[QLatin1String("businessPhone")] = contact.businessPhone();
    result[QLatin1String("businessMobile")] = contact.businessMobile();
    result[QLatin1String("businessVoip")] = contact.businessVOIP();

    return result;
}
