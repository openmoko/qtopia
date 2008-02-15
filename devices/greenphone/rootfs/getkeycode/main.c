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

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#include <stdio.h>

#define KBDDEVICE   "/dev/tty0"

int main(int argc, char *argv[])
{
    int kbdFd = open(KBDDEVICE, O_RDONLY, 0);
    if (kbdFd < 0) {
        return 1;
    }

    struct termios origTermData;
    struct termios termData;

    tcgetattr(kbdFd, &origTermData);
    tcgetattr(kbdFd, &termData);

    ioctl(kbdFd, KDSKBMODE, K_RAW);

    termData.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termData.c_oflag = 0;
    termData.c_cflag = CREAD | CS8;
    termData.c_lflag = 0;
    termData.c_cc[VTIME]=0;
    termData.c_cc[VMIN]=1;
    cfsetispeed(&termData, 9600);
    cfsetospeed(&termData, 9600);
    tcsetattr(kbdFd, TCSANOW, &termData);

    unsigned char code;
    int n = read(kbdFd, &code, 1);
    if (n == 1)
        printf("%02x\n", code);

    ioctl(kbdFd, KDSKBMODE, K_XLATE);
    tcsetattr(kbdFd, TCSANOW, &origTermData);
    close(kbdFd);

    return 0;
}

