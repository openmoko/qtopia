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

#ifndef EXAMPLEMOUSEHANDLER_H
#define EXAMPLEMOUSEHANDLER_H

#define TS_SAMPLES   5

#include <QtGui/QWSCalibratedMouseHandler>

#ifdef QT_QWS_EXAMPLE

class QSocketNotifier;
class ExampleMouseHandler : public QObject, public QWSCalibratedMouseHandler {
    Q_OBJECT
public:
    ExampleMouseHandler();
    ~ExampleMouseHandler();

    void calibrate( QWSPointerCalibrationData * );
    void suspend();
    void resume();

private:
    void openTs();
    void closeTs();

private:
    bool m_raw : 1;
    int  totX,totY,nX,nY;
    int  sx[TS_SAMPLES+3], sy[TS_SAMPLES+3], ss;
    int  index_x1, index_x2, index_y1, index_y2, min_x, min_y;
    int  mouseFD;
    int  mouseIdx;
    static const int mouseBufSize = 2048;
    uchar mouseBuf[mouseBufSize];
    QPoint oldmouse;
    ExampleMouseHandler *handler;
    QSocketNotifier *m_notify;

private Q_SLOTS:
    void readMouseData();
};

#endif // QT_QWS_EXAMPLE

#endif // EXAMPLEMOUSEHANDLER_H
