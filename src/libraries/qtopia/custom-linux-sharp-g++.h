/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QPE_OWNAPM
#define QPE_HAVE_TOGGLELIGHT
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
#define	SHARP_BUZZER_IOCTL_START (SHARP_DEV_IOCTL_COMMAND_START)
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


#define CUSTOM_BUZZER( sound ) \
{ \
  static int fd = open( "/dev/sharp_buz", O_RDWR|O_NONBLOCK ); \
  ioctl( fd, SHARP_BUZZER_MAKESOUND, sound ); \
}

#define CUSTOM_SOUND_ALARM CUSTOM_BUZZER( SHARP_BUZ_SCHEDULE_ALARM )

#include <sys/ioctl.h>
#include <asm/sharp_char.h>

// a bit awkward, as this value is defined in emailclient.cpp aswell...
#define LED_MAIL 0
#define SHARP_LED_MAIL 9

#define CUSTOM_LEDS( led, status ) \
{ \
    if ( led == LED_MAIL ) \
        led = SHARP_LED_MAIL; \
    static int fd = open( "/dev/sharp_led", O_RDWR|O_NONBLOCK ); \
    sharp_led_status leds; \
    memset(&leds, 0, sizeof(leds)); \
    leds.which = led; \
    leds.status = status; \
    ioctl( fd, SHARP_LED_SETSTATUS, (char*)&leds ); \
}

#define QPE_HAVE_MEMALERTER

#define QPE_MEMALERTER_IMPL \
static void sig_handler(int sig) \
{ \
    switch (sig) { \
    case SIGHUP: \
	memstate = VeryLow; \
	break; \
    case SIGUSR1: \
	memstate = Normal; \
	break; \
    case SIGUSR2: \
	memstate = Low; \
	break; \
    } \
} \
static void initMemalerter() \
{ \
    struct sigaction sa; \
    memset(&sa, '\0', sizeof sa); \
    sa.sa_handler = sig_handler; \
    sa.sa_flags = SA_RESTART; \
    if (sigaction(SIGHUP, &sa, NULL) < 0) { \
	return; \
    } \
    if (sigaction(SIGUSR1, &sa, NULL) < 0) { \
	return; \
    } \
    if (sigaction(SIGUSR2, &sa, NULL) < 0) { \
	return; \
    } \
    FILE *fo = fopen("/proc/sys/vm/freepg_signal_proc", "w"); \
     \
    if (!fo) \
        return; \
    fprintf(fo, "qpe\n"); \
    fclose(fo); \
}

#define QPE_INITIAL_NUMLOCK_STATE \
{ \
    bool numLock = FALSE; \
    sharp_kbdctl_modifstat  st; \
    int dev = ::open("/dev/sharp_kbdctl", O_RDWR); \
    if( dev >= 0 ) { \
	memset(&st, 0, sizeof(st)); \
	st.which = 3; \
                int ret = ioctl(dev, SHARP_KBDCTL_GETMODIFSTAT, (char*)&st); \
	if( !ret ) \
	    numLock = (bool)st.stat; \
	::close(dev); \
    } \
    return numLock; \
}
