/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "../../../include/ipmc.h"

#define IOCTL_WAIT_SLEEP    _IO('S', 1)
#define IOCTL_SLEEP         _IO('S', 2)

void omega_suspend()
{
    // SD card sleep
    pid_t child = fork();
    if (child < 0) {
        return;
    } else if (child == 0) {
        int sdFd = open("/dev/sdcard_pm", O_WRONLY);
        if (sdFd < 0) {
            perror("/dev/sdcard_pm");
            return;
        }

        ioctl(sdFd, IOCTL_WAIT_SLEEP);
        sync();
        ioctl(sdFd, IOCTL_SLEEP);
        close(sdFd);

        exit(0);
    } else {
        int ipmcFd = open("/dev/ipmc", O_RDWR);
        if (ipmcFd < 0) {
            perror("/dev/ipmc");
            return;
        }

        sleep(1);

        // Enter sleep mode
        struct ipm_config dpc;
        bzero(&dpc, sizeof(struct ipm_config));
        dpc.cpu_mode = 3;
        dpc.sleep_src_event = 13;

        if (ioctl(ipmcFd, IPMC_IOCTL_SET_IPM_CONFIG, &dpc) < 0)
            perror("IPMC_IOCTL_SET_IPM_CONFIG sleep");

        // Set processor frequency back to 312MHz
        bzero(&dpc, sizeof(struct ipm_config));
        dpc.core_freq = 312000;
        dpc.turbo_mode = 3;
        dpc.fast_bus_mode = 1;
        dpc.cpu_mode = 0;
        
        if (ioctl(ipmcFd, IPMC_IOCTL_SET_IPM_CONFIG, &dpc) < 0)
            perror("IPMC_IOCTL_SET_IPM_CONFIG sleep");

        // If the usb/charger cable is plugged in child process doesn't quit.
        // Kill it now.
        kill(child, SIGTERM);

        close(ipmcFd);

        return;
    }
}

int main(int argc, char *argv[])
{ 
    if (argc != 2) {
        printf("Usage: apm <option>\n"
               "\t--suspend         Suspend device\n"
               "\t--modem-poweron   Power on modem\n"
               "\t--modem-poweroff  Power off modem\n");
        return 1;
    }

    if(strcmp(argv[1], "--suspend") == 0) {
        omega_suspend();
    } else if(strcmp(argv[1], "--modem-poweron") == 0) {
        int modemFd = open("/dev/omega_bcm2121", O_RDWR);
        ioctl(modemFd, 0x5402, 0);   // power on modem
        ioctl(modemFd, 0x5401, 1);   // sleep
        close(modemFd);
    } else if(strcmp(argv[1], "--modem-poweroff") == 0) {
        int modemFd = open("/dev/omega_bcm2121", O_RDWR);
        ioctl(modemFd, 0x5403, 2);        // power off modem
        ioctl(modemFd, 0x5401, 1);        // sleep
        close(modemFd);
    }

    return 0;
}
