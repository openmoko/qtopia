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
  \mainclass
  \brief The QPowerStatus class represents a snapshot of the systems power status
  at a given time.

  The QPowerStatus class is a value class.  The latest power information can be
  obtained by calling QPowerStatusManager::readStatus().

  For example,

  \code
  QPowerStatus status = QPowerStatusManager::readStatus();
  if(status.acStatus() == QPowerStatus::Offline)
      qWarning() << "The device is not plugged into AC power!";
  \endcode

  \i {Note:} Use of the QPowerStatus class is discouraged.  Its API and functionality
  has been improved in future versions of Qtopia.

  \sa QPowerStatusManager
*/

/*!
  \fn QPowerStatus::QPowerStatus(const QPowerStatus& other)

  Constructs a copy of \a other.
*/

/*!
  \fn QPowerStatus::QPowerStatus()

  Constructs a QPowerStatus object with default values.  The default for
  each value is:

  \table
  \header \o Value \o Default
  \row \o QPowerStatus::acStatus() \o QPowerStatus::Offline
  \row \o QPowerStatus::batteryStatus() \o QPowerStatus::NotPresent
  \row \o QPowerStatus::backupBatteryStatus() \o QPowerStatus::NotPresent
  \row \o QPowerStatus::batteryPercentAccurate() \o false
  \row \o QPowerStatus::batteryPercentRemaining() \o -1
  \row \o QPowerStatus::batteryTimeRemaining() \o -1
  \endtable
*/

/*!
    \enum QPowerStatus::ACStatus

    The current state of AC power.  AC power is usually equivalent to an 
    "unlimited" wall provided source.

    \value Offline The device is running on battery
    \value Online The device is powered by an external power source
    \value Backup The device is running on backup power
    \value Unknown Status is unknown
*/

/*!
    \enum QPowerStatus::BatteryStatus

    The current battery state.

    \value High The battery is fully charged
    \value Low The battery level is low
    \value VeryLow The battery level is very low
    \value Critical The battery level is critical 
    \value Charging The battery is being charged
    \value NotPresent The status of the battery is unknown
*/

/*!
  \fn BatteryStatus QPowerStatus::batteryStatus() const

  Returns the status of the primary device battery, or QPowerStatus::NotPresent
  if the device has no primary battery.
*/

/*!
  \fn BatteryStatus QPowerStatus::backupBatteryStatus() const

  Returns the status of the backup device battery, or QPowerStatus::NotPresent
  if the device has no backup battery.
*/

/*!
  \fn bool QPowerStatus::batteryPercentAccurate() const

  Some battery sources cannot accuractely provide capacity information.  This
  method returns true if the value returned by batteryPercentRemaining() 
  reflects the true value of remaining battery. 
*/

/*!
  \fn int QPowerStatus::batteryPercentRemaining() const

  Returns the remaining battery life (percentage of charge), or -1 if no 
  information is available.
*/

/*!
  \fn int QPowerStatus::batteryTimeRemaining() const

  Returns the remaining battery life in seconds, or -1 if no information is 
  available.
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
  \mainclass
  \brief The QPowerStatusManager class provides easy access to the
  power status of the Qtopia device.
  \ingroup hardware

  QPowerStatusManager allows the current power status to be read on demand.

  \i {Note:} Use of the QPowerStatusManager class is discouraged.  Its functionality
  has been integrated into the QPowerStatus class in future versions of Qtopia.

  \sa QPowerStatus
*/

/*!

  \fn void QPowerStatusManager::getStatus()

  \internal

  Implements the platform specific part of the status reader and is called by
  \c{readStatus()}. This function should be implemented by system integrators in
  the custom-{arch}-{platform}-g++.cpp file to support their specific system.

  \sa readStatus(), getProcApmStatus()
*/

/*!
  \fn int qpe_sysBrightnessSteps()

  \relates QPowerStatus

  Returns the number of graduations supported by the device's LCD
  backlight/frontlight.

  This function must be implemented in the custom-<platform-spec>.cpp file by
  the system integrator, see \l {Hardware Configuration} for details.

  \sa qpe_setBrightness()
*/

/*!
  \fn int qpe_setBrightness(int bright)

  \relates QPowerStatus

  Sets the brightness of the device's LCD backlight/frontlight to \a bright.

  \a bright represents the LCD backlight/frontlight brightness as an integer
  between 0 and 255.  A value of 0 means that the LCD display should be turned
  off.  Values between 1 and 255 are interpreted as varying brightness levels
  with 1 representing the dimmest level and 255 the brightest.

  This function must be implemented in the custom-<platform-spec>.cpp file by
  the system integrator, see \l {Hardware Configuration} for details.  It may
  be necessary for the implementation of this function to convert \a bright to
  a device specific brightness level, as returned by qpe_sysBrightnessSteps().

  \sa qpe_sysBrightnessSteps()
*/

QPowerStatus *QPowerStatusManager::ps = 0;

/*!
  \internal

  Constructs a QPowerStatusManager object with the specified \a parent.
*/
QPowerStatusManager::QPowerStatusManager(QObject *parent)
: QObject(parent), timerId(0)
{
    if(!ps) ps = new QPowerStatus;
}

/*!
  Reads and returns the current power status of the device.

  \sa QPowerStatus
*/
QPowerStatus QPowerStatusManager::readStatus()
{
    if(!ps) ps = new QPowerStatus;
    getStatus();
    return *ps;
}

/*!
  \internal

  This function implements a standard /proc/apm reader and can be used in an
  APM based implementation of QPowerStatusManger::getStatus(). The results are 
  only valid if \c APMEnabled() returns true.
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
  \internal

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

  \internal

  Emitted whenever the power status of the device changes.  \a newStatus will
  be the new power status.
 */
