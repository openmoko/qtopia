/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#define QPE_USE_MALLOC_FOR_NEW
#define QPE_NEED_CALIBRATION
#define QPE_NOCIBAUD
#define QPE_STARTMENU
#include <asm/sharp_apm.h>
#ifndef APM_IOC_BATTERY_BACK_CHK
#define APM_IOC_BATTERY_BACK_CHK       _IO(APM_IOC_MAGIC, 32)
#endif
#ifndef APM_IOC_BATTERY_MAIN_CHK
#define APM_IOC_BATTERY_MAIN_CHK       _IO(APM_IOC_MAGIC, 33)
#endif

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define SHARP_DEV_IOCTL_COMMAND_START 0x5680

/* --- for SHARP_BUZZER device --- */
#define SHARP_BUZZER_IOCTL_START (SHARP_DEV_IOCTL_COMMAND_START)
#define SHARP_BUZZER_MAKESOUND   (SHARP_BUZZER_IOCTL_START)
#define SHARP_BUZZER_SETVOLUME   (SHARP_BUZZER_IOCTL_START+1)
#define SHARP_BUZZER_GETVOLUME   (SHARP_BUZZER_IOCTL_START+2)
#define SHARP_BUZZER_ISSUPPORTED (SHARP_BUZZER_IOCTL_START+3)
#define SHARP_BUZZER_SETMUTE     (SHARP_BUZZER_IOCTL_START+4)
#define SHARP_BUZZER_STOPSOUND   (SHARP_BUZZER_IOCTL_START+5)

#define SHARP_BUZ_TOUCHSOUND       1  /* touch panel sound */
#define SHARP_BUZ_KEYSOUND         2  /* key sound */
#define SHARP_PDA_ILLCLICKSOUND    3  /* illegal click */
#define SHARP_PDA_WARNSOUND        4  /* warning occurred */
#define SHARP_PDA_ERRORSOUND       5  /* error occurred */
#define SHARP_PDA_CRITICALSOUND    6  /* critical error occurred */
#define SHARP_PDA_SYSSTARTSOUND    7  /* system start */
#define SHARP_PDA_SYSTEMENDSOUND   8  /* system shutdown */
#define SHARP_PDA_APPSTART         9  /* application start */
#define SHARP_PDA_APPQUIT         10  /* application ends */
#define SHARP_BUZ_SCHEDULE_ALARM  11  /* schedule alarm */
#define SHARP_BUZ_DAILY_ALARM     12  /* daily alarm */
#define SHARP_BUZ_GOT_PHONE_CALL  13  /* phone call sound */
#define SHARP_BUZ_GOT_MAIL        14  /* mail sound */


#include <sys/ioctl.h>
#include <asm/sharp_char.h>

// a bit awkward, as this value is defined in emailclient.cpp aswell...
#define LED_MAIL 0
#define SHARP_LED_MAIL 9

#define SoftKey 0x8000

#define QPE_ARCHITECTURE "SHARP/SL5500"
