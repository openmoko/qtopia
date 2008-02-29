/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "examplemousehandler.h"

#ifdef QT_QWS_EXAMPLE
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>

#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

struct ExampleInput {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

// sanity check values of the range of possible mouse positions
#define MOUSE_SAMPLE_MIN    0
#define MOUSE_SAMPLE_MAX    2000

ExampleMouseHandler::ExampleMouseHandler(const QString &device)
    : nX(0), nY(0), min_x(INT_MAX), min_y(INT_MAX), mouseIdx(0)
{
    qLog(Input) << "Loaded Example touchscreen plugin!";
    setObjectName("Example Mouse Handler");
    mouseFd = ::open(device.toLocal8Bit().constData(), O_RDONLY | O_NDELAY);
    if (mouseFd >= 0) {
        qLog(Input) << "Opened" << device << "as touchscreen input";
        m_notify = new QSocketNotifier(mouseFd, QSocketNotifier::Read, this);
        connect(m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
    } else {
        qWarning("Cannot open %s for touchscreen input (%s)",
                 device.toLocal8Bit().constData(), strerror(errno));
        return;
    }
}

ExampleMouseHandler::~ExampleMouseHandler()
{
    if (mouseFd >= 0)
        ::close(mouseFd);
}

void ExampleMouseHandler::suspend()
{
    m_notify->setEnabled( false );
}

void ExampleMouseHandler::resume()
{
    m_notify->setEnabled( true );
}

void ExampleMouseHandler::readMouseData()
{
    if (!qt_screen)
        return;

    int n;

    do {
        n = read(mouseFd, mouseBuf+mouseIdx, mouseBufSize-mouseIdx);
        if (n > 0)
            mouseIdx += n;

        ExampleInput *data;
        int idx = 0;

        while (mouseIdx-idx >= (int)sizeof(ExampleInput)) {
            uchar *mb = mouseBuf+idx;
            data = (ExampleInput*)mb;
            if (data->code == 0) {
                // x value
                sx[nX] = data->value;
                nX++;
            } else if (data->code == 1) {
                // y value
                sy[nY] = data->value;
                nY++;
            }
            if (nX >= TS_SAMPLES && nY >= TS_SAMPLES) {
                int ss = (nX < nY) ? nX : nY;

                for (int i = 0; i < ss - 1; i++) {
                    for (int j = i + 1; j < ss; j++) {
                        int dx = sx[i] - sx[j];
                        if (dx < 0)
                            dx = -dx;
                        int dy = sy[i] - sy[j];
                        if (dy < 0)
                            dy = -dy;
                        if (min_x > dx) {
                            min_x = dx;
                            index_x1 = i;
                            index_x2 = j;
                        }
                        if (min_y > dy) {
                            min_y = dy;
                            index_y1 = i;
                            index_y2 = j;
                        }
                    }
                }

                QPoint pos((sx[index_x1] + sx[index_x2])/2,
                           (sy[index_y1] + sy[index_y2])/2);

                nX = 0;
                nY = 0;
                min_x = INT_MAX;
                min_y = INT_MAX;

                oldmouse = transform( pos );
                if (oldmouse.x() < MOUSE_SAMPLE_MIN || oldmouse.x() > MOUSE_SAMPLE_MAX ||
                    oldmouse.y() < MOUSE_SAMPLE_MIN || oldmouse.y() > MOUSE_SAMPLE_MAX) {
                    qLog(Input) << "*BAD Mouse sample :x="
                                << oldmouse.x() << ",y=" << oldmouse.y();
                    oldmouse.setX(0);
                    oldmouse.setY(0);
                } else {
                    qLog(Input) << "Mouse Down:x="
                                << oldmouse.x() << ",y=" << oldmouse.y();
                    emit mouseChanged(oldmouse, Qt::LeftButton);
                }
            }
            if ((data->code == 24) && (data->value == 0)) {
                // Removed pen from screen
                qLog(Input) << "Mouse Up  :x=" << oldmouse.x() << ",y=" << oldmouse.y();
                emit mouseChanged(oldmouse, 0);
                nX = 0;
                nY = 0;
            }
            idx += sizeof(ExampleInput);
        }
        int surplus = mouseIdx - idx;
        for (int i = 0; i < surplus; i++)
            mouseBuf[i] = mouseBuf[idx+i];
        mouseIdx = surplus;
    } while (n > 0);
}

#endif // QT_QWS_EXAMPLE
