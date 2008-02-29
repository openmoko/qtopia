/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "nokiascreen.h"

#include <QRect>
#include <QRegion>
#include <QtGui/qscreen_qws.h>
#include <QWSServer>
#include <QDebug>

//#include <asm/arch/omapfb.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>

#define OMAP_IOW(num, dtype)    _IOW('O', num, dtype)
#define OMAPFB_UPDATE_WINDOW    OMAP_IOW(47, struct omapfb_update_window)
#define OMAPFB_UPDATE_WINDOW_OLD    OMAP_IOW(41, struct omapfb_update_window_old)

#define OMAPFB_SET_UPDATE_MODE OMAP_IOW(40, enum  omapfb_update_mode)

struct omapfb_update_window_old {
       __u32 x, y;
       __u32 width, height;
//       __u32 format;
};

struct omapfb_update_window {
  __u32 x, y;
  __u32 width, height;
  __u32 format;
};

enum omapfb_update_mode {
    OMAPFB_UPDATE_DISABLED = 0,
    OMAPFB_AUTO_UPDATE,
    OMAPFB_MANUAL_UPDATE
};


NokiaScreen::NokiaScreen(int displayId)
: QLinuxFbScreen(displayId)
{
    qWarning()<<"NokiaScreen";

    QString dev="/dev/fb0";
    int updatemode =  OMAPFB_AUTO_UPDATE;
    int infd = open( dev.toLatin1().constData(), O_RDWR);
    if (infd > 0) {

        if (ioctl( infd, OMAPFB_SET_UPDATE_MODE, &updatemode) < 0) {
            perror("ioctl( OMAPFB_SET_UPDATE_MODE)");
            qWarning()<<"ioctl( OMAPFB_SET_UPDATE_MODE)";
        }
        close(infd);
    } else {
        qWarning("NOPT!@");
    }
}

NokiaScreen::~NokiaScreen()
{
}

void NokiaScreen::exposeRegion(QRegion r, int changing)
{

    r &= region();
    if (r.isEmpty())
        return;

    QScreen::exposeRegion(r, changing);
    struct omapfb_update_window_old update;

    update.x = r.boundingRect().x();
    update.y = r.boundingRect().y();
    update.width = r.boundingRect().width();
    update.height = r.boundingRect().height();
//  update.format = 0/*OMAPFB_COLOR_RGB565*/;

    QString dev="/dev/fb0";

    int infd = open( dev.toLatin1().constData(), O_RDWR);
    if (infd > 0) {
        if (ioctl( infd, OMAPFB_UPDATE_WINDOW_OLD, &update) < 0) {
            perror("ioctl(OMAPFB_UPDATE_WINDOW)");
            qWarning()<<"ioctl(OMAPFB_UPDATE_WINDOW)";
        }
        close(infd);
    } else {
        qWarning("NOPT!@");
    }
}

