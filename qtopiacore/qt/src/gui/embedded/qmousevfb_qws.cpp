/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_QWS_MOUSE_QVFB

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qvfbhdr.h>
#include <qmousevfb_qws.h>
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qtimer.h>

QVFbMouseHandler::QVFbMouseHandler(const QString &driver, const QString &device)
    : QObject(), QWSMouseHandler(driver, device)
{
    QString mouseDev = device;
    if (device.isEmpty())
        mouseDev = QLatin1String("/dev/vmouse");

    mouseFD = -1;
    if ((mouseFD = open(mouseDev.toLatin1().constData(), O_RDWR | O_NDELAY)) < 0) {
        qWarning("Cannot open %s (%s)", mouseDev.toLatin1().constData(),
                strerror(errno));
    } else {
        // Clear pending input
        char buf[2];
        while (read(mouseFD, buf, 1) > 0) { }

        mouseIdx = 0;

        mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this);
        connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    }
}

QVFbMouseHandler::~QVFbMouseHandler()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

void QVFbMouseHandler::resume()
{
    mouseNotifier->setEnabled(true);
}

void QVFbMouseHandler::suspend()
{
    mouseNotifier->setEnabled(false);
}

void QVFbMouseHandler::readMouseData()
{
    int n;
    do {
        n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx);
        if (n > 0)
            mouseIdx += n;
    } while (n > 0);

    int idx = 0;
    static const int packetsize = sizeof(QPoint) + 2*sizeof(int);
    while (mouseIdx-idx >= packetsize) {
        uchar *mb = mouseBuf+idx;
        QPoint mousePos = *reinterpret_cast<QPoint *>(mb);
        mb += sizeof(QPoint);
        int bstate = *reinterpret_cast<int *>(mb);
        mb += sizeof(int);
        int wheel = *reinterpret_cast<int *>(mb);
//        limitToScreen(mousePos);
        mouseChanged(mousePos, bstate, wheel);
        idx += packetsize;
    }

    int surplus = mouseIdx - idx;
    for (int i = 0; i < surplus; i++)
        mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;
}

#endif // QT_NO_QWS_MOUSE_QVFB
