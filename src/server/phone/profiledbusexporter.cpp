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

#include "profiledbusexporter.h"

#ifdef QT_ILLUME_LAUNCHER

#include <QDBusArgument>
#include <QDBusMetaType>

Q_DECLARE_METATYPE(Profile);
Q_DECLARE_METATYPE(QList<Profile>);

QDBusArgument& operator<<(QDBusArgument &argument, const Profile& profile)
{
    argument.beginStructure();
    argument << profile.name;
    argument << profile.id;
    argument.endStructure();

    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, Profile &profile)
{
    argument.beginStructure();
    argument >> profile.name;
    argument >> profile.id;
    argument.endStructure();
    return argument;
}

ProfileDBusExporter::ProfileDBusExporter(QObject* parent)
    : QObject(parent)
    , m_manager(new QPhoneProfileManager(this))
{
    qRegisterMetaType<Profile>("Profile");
    qDBusRegisterMetaType<Profile>();
    qDBusRegisterMetaType<QList<Profile> >();


    connect(m_manager, SIGNAL(activeProfileChanged(const QPhoneProfile&)),
            SLOT(_q_activeProfileChanged(const QPhoneProfile&)));
    connect(m_manager, SIGNAL(profileUpdated(const QPhoneProfile&)),
            SIGNAL(profilesChanged()));
    connect(m_manager, SIGNAL(profileAdded(const QPhoneProfile&)),
            SIGNAL(profilesChanged()));
    connect(m_manager, SIGNAL(profileRemoved(const QPhoneProfile&)),
            SIGNAL(profilesChanged()));
}

QList<Profile> ProfileDBusExporter::profiles()
{
    QList<Profile> profiles;

    foreach(QPhoneProfile profile, m_manager->profiles())
        profiles.append(profile);

    return profiles;
}

Profile ProfileDBusExporter::currentProfile()
{
    return m_manager->activeProfile();
}

bool ProfileDBusExporter::setCurrentProfile(const Profile& profile)
{
    return m_manager->activateProfile(profile.id);
}

void ProfileDBusExporter::_q_activeProfileChanged(const QPhoneProfile& profile)
{
    emit activeProfileChanged(profile);
}

#endif
