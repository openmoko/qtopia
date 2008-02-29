/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qphoneprofileprovider.h"
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QValueSpaceObject>
#include <QValueSpaceItem>
#include "qtopiaserverapplication.h"

// declare QPhoneProfileProviderPrivate
class QPhoneProfileProviderPrivate
{
public:
    QPhoneProfileProviderPrivate()
    : manager(0), profile(0), accessories(0), preAccessorySelected(-1) {}

    QPhoneProfileManager *manager;
    QValueSpaceObject *profile;
    QValueSpaceItem *accessories;

    int preAccessorySelected;
};

/*!
  \class QPhoneProfileProvider
  \ingroup QtopiaServer
  \brief The QPhoneProfileProvider class provides the backend functionality for
         phone profiles.

  The QPhoneProfileProvider class provides the auto activation portion of the
  profile system.  Profiles may be auto activated at a specific time, controlled
  through the QPhoneProfile::schedule() value, or whenever the
  QPhoneProfile::accessory() accessory is present.

  Currently the QPhoneProfileProvider class checks the
  \c {/Hardware/Accessories/<accessory name>/Present} value space key to
  determine whether a given accessory is available.

  Additionally, the QPhoneProfileProvider class sets the following value space
  keys to values dictated by the currently active profile:

  \table
  \header \o Key \o Description
  \row \o \c {/UI/Profile/Name} \o The name of the profile, as returned by QPhoneProfile::name().
  \row \o \c {/UI/Profile/Default} \o True if this is the default profile (profile with id 1) or false if not.
  \row \o \c {/UI/Profile/PlaneMode} \o True if plane mode is on, false if not, as returned by QPhoneProfileManager::planeMode().
  \row \o \c {/UI/Profile/RingVolume} \o Set to the ring volume of the current profile, as returned by QPhoneProfile::volume().
  \endtable

  The QPhoneProfileProvider provides the \c {PhoneProfiles} task.
 */

// define QPhoneProfileProvider
/*! \internal */
QPhoneProfileProvider::QPhoneProfileProvider(QObject *parent)
: QObject(parent), d(new QPhoneProfileProviderPrivate)
{
    d->manager = new QPhoneProfileManager(this);
    d->profile = new QValueSpaceObject("/UI/Profile", this);
    d->accessories = new QValueSpaceItem("/Hardware/Accessories", this);

    QObject::connect(d->manager, SIGNAL(profileUpdated(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(profileAdded(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(profileRemoved(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(activeProfileChanged(QPhoneProfile)),
                     this, SLOT(activeChanged()));
    QObject::connect(d->manager, SIGNAL(planeModeChanged(bool)),
                     this, SLOT(activeChanged()));
    QObject::connect(d->accessories, SIGNAL(contentsChanged()),
                     this, SLOT(accessoriesChanged()));
    QObject::connect(d->manager, SIGNAL(profileUpdated(QPhoneProfile)),
                     this, SLOT(accessoriesChanged()));
    QObject::connect(d->manager, SIGNAL(profileAdded(QPhoneProfile)),
                     this, SLOT(accessoriesChanged()));
    QObject::connect(QtopiaApplication::instance(),
                     SIGNAL(appMessage(QString,QByteArray)),
                     this,
                     SLOT(appMessage(QString,QByteArray)));

    activeChanged();
    accessoriesChanged();
}

/*! \internal */
QPhoneProfileProvider::~QPhoneProfileProvider()
{
    delete d;
}

void QPhoneProfileProvider::appMessage(const QString &msg,
                                       const QByteArray &data)
{
    if(msg == "activateProfile(QDateTime,int)") {
        QDataStream ds(data);
        QDateTime when;
        int id;
        ds >> when >> id;

        QPhoneProfile prof = d->manager->profile(id);
        if(prof.id() != -1)
            d->manager->activateProfile(id);

        scheduleActivation();
    }
}

void QPhoneProfileProvider::activeChanged()
{
    QPhoneProfile active = d->manager->activeProfile();
    d->profile->setAttribute("Name", active.name());
    d->profile->setAttribute("Default", active.id() == 1);
    d->profile->setAttribute("PlaneMode", d->manager->planeMode());
    d->profile->setAttribute("RingVolume", active.volume());
}

void QPhoneProfileProvider::scheduleActivation()
{
    QDateTime now = QDateTime::currentDateTime();
    int today = now.date().dayOfWeek();
    QDateTime when;
    int id = -1;

    QList<QPhoneProfile> profiles = d->manager->profiles();
    for(int ii = 0; ii < profiles.count(); ++ii) {
        const QPhoneProfile &prof = profiles.at(ii);
        QPhoneProfile::Schedule s = prof.schedule();

        if(s.isActive()) {
            QList<Qt::DayOfWeek> days = s.scheduledOnDays();
            for(int jj = 0; jj < days.count(); ++jj) {
                QDateTime current(now.date(), s.time());
                Qt::DayOfWeek day = days.at(jj);

                if(day < today)
                    current = current.addDays(7 - today + day);
                else if(day > today)
                    current = current.addDays(day - today);
                else if(s.time() < now.time())
                    current = current.addDays(7);

                if(!when.isValid() || current < when) {
                    when = current;
                    id = prof.id();
                }
            }

        }
    }

    if ( id != -1 )
        Qtopia::addAlarm(when, "QPE/Application/" + QtopiaApplication::applicationName(), "activateProfile(QDateTime,int)", id );
    else
        Qtopia::deleteAlarm(QDateTime(), "QPE/Application/" + QtopiaApplication::applicationName(), "activateProfile(QDateTime,int)", id);
}

void QPhoneProfileProvider::accessoriesChanged()
{
    // Check whether active profile matches accessories.  In the case of
    // accessory clash (two profiles depending on the same accessory) we should
    // avoid switching between the two.
    QPhoneProfile activeProfile = d->manager->activeProfile();
    if(!activeProfile.accessory().isEmpty() &&
       d->accessories->value(activeProfile.accessory() + "/Present", false).toBool())
        return;

    // See if any of the available profiles match
    QList<QPhoneProfile> profiles = d->manager->profiles();
    for(int ii = 0; ii < profiles.count(); ++ii) {
        if(!profiles.at(ii).accessory().isEmpty() &&
           d->accessories->value(profiles.at(ii).accessory() + "/Present", false).toBool()) {

            if(-1 == d->preAccessorySelected)
                d->preAccessorySelected = d->manager->activeProfile().id();

            d->manager->activateProfile(profiles.at(ii).id());
            return;

        }
    }

    // Nothing - revert to the preaccessory profile
    if(-1 != d->preAccessorySelected) {
        QPhoneProfile preAccessory = d->manager->profile(d->preAccessorySelected);
        d->preAccessorySelected = -1;
        if(preAccessory.id() != -1) {
            d->manager->activateProfile(preAccessory.id());
        } else {
            // Try default
            QPhoneProfile general = d->manager->profile(1);
            if(general.id() != -1)
                d->manager->activateProfile(general.id());
        }
    }
}


QTOPIA_TASK(PhoneProfiles, QPhoneProfileProvider);
