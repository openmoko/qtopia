/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSETSLIB_QWS_H
#define QMOUSETSLIB_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_MOUSE_TSLIB) || defined(QT_PLUGIN)

class QWSTslibMouseHandlerPrivate;

class QWSTslibMouseHandler : public QWSCalibratedMouseHandler
{
public:
    explicit QWSTslibMouseHandler(const QString &driver = QString(),
                                  const QString &device = QString());
    ~QWSTslibMouseHandler();

    void suspend();
    void resume();

    void calibrate(const QWSPointerCalibrationData *data);
    void clearCalibration();

protected:
    QWSTslibMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_TSLIB
QT_END_HEADER

#endif // QMOUSETSLIB_QWS_H
