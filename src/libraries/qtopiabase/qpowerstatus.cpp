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

#include "qpowerstatus.h"

#include "custom.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef Q_OS_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <QTimerEvent>

#ifdef QT_QWS_IPAQ_NO_APM
#include <linux/h3600_ts.h>
#endif

/*!
  \class QPowerStatus
  \brief The QPowerStatus class represents a snapshot of the systems power status
  at a given time.

  An updated status can be obtained by calling \c{QPowerStatusManager::readStatus()}.

  \ingroup hardware
*/

/*!
  \fn QPowerStatus::QPowerStatus(const QPowerStatus& other)

  Constructs a QPowerstatus object from a deep copy of \a other.
*/

/*!
  \fn QPowerStatus::QPowerStatus()

  Constructs a QPowerStatus object.
*/
/*!
    \enum QPowerStatus::ACStatus

    \value Offline the device is running on battery
    \value Online the device is powered by an external power source
    \value Backup the device is running on backup power
    \value Unknown status is unknown
*/


/*!
    \enum QPowerStatus::BatteryStatus

    \value High battery is fully charged
    \value Low the battery level is low
    \value VeryLow very low battery level
    \value Critical critical battery level
    \value Charging the battery is being charged
    \value NotPresent the status if the battery is unknown
*/

/*!
  \fn BatteryStatus QPowerStatus::batteryStatus() const

  Returns the battery status of the device.
*/

/*!
  \fn BatteryStatus QPowerStatus::backupBatteryStatus() const

  Returns the status of the backup battery.
*/

/*!
  \fn bool QPowerStatus::batteryPercentAccurate() const

  Returns true if the value returned by \c{batteryPercentRemaining()}
  reflects the true value of remaining battery. If for instance APM is
  not available the return value of remaining battery does not reflect
  the true value.
*/

/*!
  \fn int QPowerStatus::batteryPercentRemaining() const

  Returns the remaining battery life (percentage of charge).
*/

/*!
  \fn int QPowerStatus::batteryTimeRemaining() const

  Returns the remaining battery life in seconds.
*/

/*!
  \fn ACStatus QPowerStatus::acStatus() const

  Returns the AC status of the device.
*/

/*!
  \fn bool QPowerStatus::operator!=( const QPowerStatus &ps )

  Returns true if this power status is not equal to \a ps, otherwise returns
  false.
*/

/*!
  \class QPowerStatusManager
  \brief The QPowerStatusManager class provides easy access to the
  power status of the Qtopia device.
  \ingroup hardware
*/

/*!

  \fn void QPowerStatusManager::getStatus()

  Implements the platform specific part of the status reader and is called by
  \c{readStatus()}. This function is defined in custom-{arch}-{platform}-g++.cpp files.

  \sa readStatus(), getProcApmStatus()
*/

QPowerStatus *QPowerStatusManager::ps = 0;

/*!
  Constructs a QPowerStatusManager object with the specified \a parent.
*/
QPowerStatusManager::QPowerStatusManager(QObject *parent)
: QObject(parent), timerId(0)
{
    if(!ps) ps = new QPowerStatus;
}

/*!
  Reads and returns the current power status of the device.

  \sa getStatus(), QPowerStatus
*/
QPowerStatus QPowerStatusManager::readStatus()
{
    if(!ps) ps = new QPowerStatus;
    getStatus();
    return *ps;
}

/*!
  This function implements a standard /proc/apm reader. The results
  are only valid if \c APMEnabled() returns true.

  \list
  \o \a ac - AC line status
  \o \a bs - battery status
  \o \a bf - battery flag (not used by Qtopia)
  \o \a pc - remaining battery life (percentage of charge)
  \o \a sec - remaining battery life (time units)
  \endlist
*/
bool QPowerStatusManager::getProcApmStatus( int &ac, int &bs, int &bf, int &pc, int &sec )
{
    bool ok = false;

    ac = 0xff;
    bs = 0xff;
    bf = 0xff;
    pc = -1;
    sec = -1;

    FILE *f = fopen("/proc/apm", "r");
    if ( f  && APMEnabled()) {
        //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
        char u;
        fscanf(f, "%*[^ ] %*d.%*d 0x%*x 0x%x 0x%x 0x%x %d%% %i %c",
                &ac, &bs, &bf, &pc, &sec, &u);
        fclose(f);
        switch ( u ) {
            case 'm': sec *= 60;
            case 's': break; // ok
            default: sec = -1; // unknown
        }

        // extract data
        switch ( bs ) {
            case 0x00:
                ps->bs = QPowerStatus::High;
                break;
            case 0x01:
                ps->bs = QPowerStatus::Low;
                break;
            case 0x7f:
                ps->bs = QPowerStatus::VeryLow;
                break;
            case 0x02:
                ps->bs = QPowerStatus::Critical;
                break;
            case 0x03:
                ps->bs = QPowerStatus::Charging;
                break;
            case 0x04:
            case 0xff: // 0xff is Unknown but we map to NotPresent
            default:
                ps->bs = QPowerStatus::NotPresent;
                break;
        }

        switch ( ac ) {
            case 0x00:
                ps->ac = QPowerStatus::Offline;
                break;
            case 0x01:
                ps->ac = QPowerStatus::Online;
                break;
            case 0x02:
                ps->ac = QPowerStatus::Backup;
                break;
        }

        if ( pc > 100 )
            pc = -1;

        ps->percentRemain = pc;
        ps->secsRemain = sec;

        ok = true;
    } else {
       ps->bs = QPowerStatus::NotPresent;
       pc = sec = -1;
       ps->percentRemain = -1;
       ps->secsRemain = -1;
       ps->ac = QPowerStatus::Unknown;
    }


    return ok;
}

/*!
  Returns true if the device has enabled APM support.
*/
bool QPowerStatusManager::APMEnabled()
{
    int apm_install_flags;
    FILE *f = fopen("/proc/apm", "r");
    if ( f ) {
        //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
        fscanf(f, "%*[^ ] %*d.%*d 0x%x 0x%*x 0x%*x 0x%*x %*d%% %*i %*c",
                &apm_install_flags);
        fclose(f);

        if (!(apm_install_flags & 0x08)) //!APM_BIOS_DISABLED
        {
            return true;
        }
    }
    return false;
}

/*!  \internal */
void QPowerStatusManager::connectNotify(const char *signal)
{
    if(!timerId && 0 == qstrcmp(signal,
                                SIGNAL(powerStatusChanged(QPowerStatus)))) {
        oldStatus = *ps;
        timerId = startTimer(10000);
    }

    QObject::connectNotify(signal);
}

/*!  \internal */
void QPowerStatusManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() != timerId) {
        QObject::timerEvent(e);
    } else {
        readStatus();
        if(oldStatus != *ps) {
            oldStatus = *ps;
            emit powerStatusChanged(oldStatus);
        }
    }
}

/*!
  \fn void QPowerStatusManager::powerStatusChanged(const QPowerStatus &newStatus)

  Emitted whenever the power status of the device changes.  \a newStatus will
  be the new power status.
 */
