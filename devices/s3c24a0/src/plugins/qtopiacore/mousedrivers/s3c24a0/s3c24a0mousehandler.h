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

#ifndef S3C24A0MOUSEHANDLER_H
#define S3C24A0MOUSEHANDLER_H

#include <QtGui/QWSCalibratedMouseHandler>

#ifdef QT_QWS_S3C24A0

#define QT_QWS_TP_SAMPLE_SIZE 5
#define QT_QWS_TP_MINIMUM_SAMPLES 5
#define QT_QWS_TP_PRESSURE_THRESHOLD 1
#define QT_QWS_TP_MOVE_LIMIT 100
#define QT_QWS_TP_JITTER_LIMIT 2


class QSocketNotifier;
class S3c24a0MouseHandler : public QObject, public QWSCalibratedMouseHandler
{
    Q_OBJECT
public:
    S3c24a0MouseHandler();
    ~S3c24a0MouseHandler();

    //void clearCalibration();
    void calibrate( QWSPointerCalibrationData * );
    void suspend();
    void resume();

private:
    void openTs();
    void closeTs();

private:
    bool m_raw : 1;
    int  mouseFD;
    int  mouseIdx;
    int  numSamples;
    int  currSample;
    QPoint samples[QT_QWS_TP_SAMPLE_SIZE+1];
    static const int mouseBufSize = 2048;
    uchar mouseBuf[mouseBufSize];
    QPoint oldmouse;
    S3c24a0MouseHandler *handler;
    QSocketNotifier *m_notify;

private Q_SLOTS:
    void readMouseData();
};

#endif // QT_QWS_S3C24A0

#endif // S3C24A0MOUSEHANDLER_H
