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

#ifndef PROFILEDBUSEXPORTER_H
#define PROFILEDBUSEXPORTER_H

#ifdef QT_ILLUME_LAUNCHER

#include <QPhoneProfile>
#include <QDBusConnection>

struct Profile {
    Profile()
        : name()
        , id(-1)
    {}

    Profile(const QPhoneProfile& profile)
        : name(profile.name())
        , id(profile.id())
    {}

    QString name;
    int id;
};

class ProfileDBusExporter : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openmoko.qtopia.Profiles");

public:
    ProfileDBusExporter(QObject* parent);

Q_SIGNALS:
    Q_SCRIPTABLE void activeProfileChanged(Profile);
    Q_SCRIPTABLE void profilesChanged();

public Q_SLOTS:
    Q_SCRIPTABLE QList<Profile> profiles();
    Q_SCRIPTABLE Profile currentProfile();
    Q_SCRIPTABLE bool setCurrentProfile(const Profile&);

private Q_SLOTS:
    void _q_activeProfileChanged(const QPhoneProfile&);

private:
    QPhoneProfileManager *m_manager;
};

#endif

#endif
